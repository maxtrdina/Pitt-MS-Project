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
kept in the switch to map sequence numbers back and forward. Switches are not built for thi, so, had it been
possible to do, this kind of computation would have certainly hurt line speed.

In light of this, we decided to add this component in between the switch and the spines node. Agents are compiled
against Spines to take advantage of its library, which provides a set of functions to deliver traffic to Spines
the way Spines understands traffic. An agent is capable of opening inbound (traffic that goes into Spines) and
outbound (traffic coming out of Spines) connections. The former acts as an interface between the sender and spines:
it takes traffic and sends it to spines. The latter extracts traffic from spines and directs it to its original
destination. These always come in pairs.

### Manager

A manager coordinates flow creation. This is the only component a client talks to and, ideally, supports two
operations: register flow and delete flow. When a client requests a flow to be created, the manager needs to
make sure a number of things happen:

- Appropriate in and out spines nodes are selected (currently there's only one).
- An inbound and outbound connections are started in their agents.
- Switch forwarding and rerouting rules are installed in the client's switch.

Once those things have happened, the client can start sending traffic.

In the original specification, the agent would keep track of a topology of clusters with machines with Spines nodes,
creating and selecting overlays given the client's latency and bandwidth requirements. We wound up abandoning this
part of the project in favor of rerouting and solving all the challenges it presented.

### Switch Controller

One of the limitations of mininet is that I couldn't find a way to talk to the switches within the virtual
environment. Because of this, I needed give the manager the ability to communicate with an external entity
running in the host OS (or yet another VM in my case). The way this was done was by having the manager write
requests to the filesystem. This requests will be picked up by the controller, which will then be in charge of
reprogramming the switch. This is just some duct tape I needed to put on to get the system to work in my
environment, but this is not intended to be the case in production grade software.

## Testbed

To develop and test the system, I used a three host, three switch mininet topology:

![Topology](assets/Topology.png "Topology")

In this topology, hosts 1 and 2 can create flows and send traffic through the system. These nodes need to have a
switch of their own to redirect traffic and fake traffic sources. S3 and H3 are reductions of the internet and
the Spines backbone, respectively.

The mininet topology is created by an adaptation of the supporting utility of the
[P4 tutorials](https://github.com/p4lang/tutorials), feel free to check them out.

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

I've only managed to bring up a single mininet console up to this point, which turned out to be a big annoyance.
Because of this, testing was performed in several phases: locally with independent agents; locally with single,
threaded agent; and within Mininet (yet to be done).

### Locally with independent agents

Two agents are set up using the `in_test` and `out_test` binaries. These create a spines connection on port 8108.
Inbound receives traffic on port 11678 and outbound delivers traffic to port 11999. Get those running along with
a Spines node on localhost and you'll see a client send packets and a "server" receive them.

### Locally with a single, threaded agent

The same set up, except a single agent on localhost controlled by the client. To run the agent, execute the
`agent` binary. Options 4 and 5 in `client` set up the agent's threads properly.

### Within mininet

The only part that's left within the manager is setting up the agent's threads, after that and ironing out a
couple other kinks, I'll test the system as a whole.
