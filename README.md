## Before we Begin

You may notice that this readme will be very similar to Izzy's readme. It differs in two primary ways:
 - I did not use mininet for the testing/developing of my version of the project. Any sections pertaining to mininet have been eliminated
 - As a result of the above difference, there is no programmable switch functionality present

# Masters Project

A system capable of registering traffic flows and intercepting traffic at the switch level to reroute it to a
[Spines](http://spines.org/) backbone. When traffic comes out of the backbone, it's delivered to its intended
destination. The goal is to accomplish this with minimal user intervention.

## Components

The project has three parts: the controller, a set of agents and Spines nodes, and programmed switches. All these
parts work together to accomplish the goal described above.

### Programmable Switches

A switch needs to be capable of detecting traffic going from node A to node B at specific ports. Additionally, a
switch needs to re-route traffic to a specific port in node C, which hosts an instance of an agent. Finally,
a switch needs to be able to detect traffic coming from the agent in node C with A as destination A and have node
A believe the traffic came from node B. To accomplish this, [P4](https://p4.org/) was used to program switches to
deconstruct packets into ethernet headers, IPv4 headers, TCP/UDP, TCP options if necessary, and the payload.

Fundamentally, the job of a switch is to decide where to send the traffic it receives next. That is, given an
IP address, what is this packet's next hop. In P4, this is done by setting the next device's mac address in
the ethernet packet, setting the egress port of the switch the packet should come out of, and decreasing
the TTL in the IP header. Because we're modifying a field in the IP header, the checksum needs to be recalculated
as well.

Per the requirements, the switch's next job is to re-route traffic when necessary. A switch knows it needs to
re-route traffic when it comes from node A and it's going to a specific port of node B, as defined by a flow.
Every time a new flow is created a new flow we install a set of rules into a routing table the switch can
access. When a packet arrives, the switch inspects the deconstructed packet, matching address and port fields
in the IP and TCP/UDP headers, respectively. If an incoming packet matches one of the existing rules, the
destination address and port are replaced to match the agent we want to forward the packet to. Additionally
the next hop mac and egress port values are set to send the packet towards the agent instance. Because we've
modified a value in the TCP/UDP header, we need to recompute the checksum.

Finally, a switch needs to be able to modify the source address and port of a packet to trick node A to believe
the packet came from node B instead of the agent in node C. This process is the same as the one described above,
but in reverse: a packet with node A as the destination and node C as the source will still be forwarded to A,
but if the source port happens to be the agent's, the source address and port will be replaced with B's.

### Agent

Originally, the idea was to have a router to not only re-route incoming traffic to the spines backbone, but also
to adapt the traffic to what a Spines node expects. A spines node receiving traffic requires packets to have
specific application headers. Having the requesting application include those application headers, though, would
violate the minimal intervention clause, so the first attempt at adding them involved having the switch adding
them. This turned out to be a problem, since making a header larger meant that a whole lot of state needed to be
kept in the switch to map sequence numbers back and forward. Switches are not built for this, so, had it been
possible to do, this kind of computation would have certainly hurt line speed.

In light of this, we decided to add this component in between the switch and the spines node. Agents are compiled
against Spines to take advantage of its library, which provides a set of functions to deliver traffic to Spines
the way Spines understands traffic. An agent is capable of opening inbound (traffic that goes into Spines) and
outbound (traffic coming out of Spines) connections. The former acts as an interface between the sender and spines:
it takes traffic and sends it to spines. The latter extracts traffic from spines and directs it to its original
destination. These always come in pairs.

### Manager

A manager coordinates flow creation and manages resources associated with flow creation. This is the only 
component a client talks to and, ideally, supports two operations: register flow and delete flow. When a 
client requests a flow to be created, the manager needs to ensure several things:

- Appropriate in and out spines nodes are used.
- Selected spines nodes have available capacity
- An inbound and outbound connections are started in their agents.
- Switch forwarding and rerouting rules are installed in the client's switch.

Once those things have happened, the client can start sending traffic.

In the original specification, the agent would keep track of a topology of clusters with machines with Spines nodes,
creating and selecting overlays given the client's latency and bandwidth requirements. We wound up abandoning this
part of the project in favor of rerouting and solving all the challenges it presented.

## Testbed

To develop and test the system, I used the following topology:

![topology (1)](https://user-images.githubusercontent.com/77497547/118414091-affb3380-b670-11eb-85ac-83fec9768ba2.png)

## Requirements

- Python 2
- GCC

## Setup

P4 requires a few python packages. `make` in the [mininet](mininet) folder will bring those to light.

Secure a version of Spines and place it in the root directory of the project; name the directory `spines`.

Agent requires a few objects from Spines, so compile spines before compiling the project.

To compile the project, run the build script (build.sh). You may need to make it executable.

## Artifacts

Spines: the Spines executable will be in spines/daemon

All the project binaries will be placed in mininet. These include:

- manager
- agent
- client
- in_test
- out_test

## Testing

My version of the project only tests the functionality of the Manager and Agent code. This can be done as follows

### Traffic delivery through spines

Tests that the agent is capable of routing traffic through spines. Run the following commands from the
[mininet](mininet) directory on the machine corresponding to the name of the device on the topology diagram 
unless instructed otherwise:

- Run the Spines 1 instance: `./spines -l 10.0.3.103` (run from the spines dir)
- Run the Spines 2 instance: `./spines -l 10.0.4.104` (run from the spines dir)
- Run the `h2-rcv-ext.py` script on the Receiver: `python support/h2-rcv-ext.py`
  - Input `10.0.6.106`
  - Input `12345` 
- Run the Agent 1 instance: `./agent 10.0.2.102`
- Run the Agent 2 instance: `./agent 10.0.5.105`
- Run the Manager: `./manager`
- Run the Client: `./client 1`
  - Select option 2
  - Type `10.0.6.106`
  - Type  `12345`
  - Specify a flow value (30 to 50 is a good place to start)
  - Type 0 for Inbound Agent 
  - Type 1 for Outbound Agent
  - Type 0 for Entry Overlay
  - Type 1 for Exit Overlay
- On the Client, use the `all-send.py` script to send messgaes to the receive script: `python support/all-send.py 10.0.2.102 8108 HelloWorld`

Observe messages come through to the receive script. They will be displayed in the terminal and written to a file
named h2-in.txt.

Additional flows can be created by repeating the last two steps with a different port number, and running 
the third step again with the same port number. To send data across these flows, repeat the last step but
increment the port number by one to send on the correct flow. For example, if you are creating a second
flow, you would use the command `python support/all-send.py 10.0.2.102 8109 HelloWorld`

NOTE: Because we don't have any overlay switches intercepting and modifying the client sends, the client
must instead send data to the inbound agent using the port the agent uses to communicate with spines in
order for the data to be sent through spines to the destination we specified. We are effectively doing
the formatting that the overlay switch would normally be doing.

## Additional Functionality over Izzy's Version

There are several differences between the functionality of Izzy's project and this one. Primarily, these are:
 - This version works with multiple spines instances using a physical topology with multiple machines
 - Support of multiple simultaneous flows
 - The Manager performs resource management in two forms
   - Spines port management on a per flow basis
   - Spines overlay flow capacity management

## Acknowledgements

I thank my advisor, [Dr. Amy Babay](https://www.pitt.edu/~babay/), for her support in getting this project off
the ground. Thanks!
