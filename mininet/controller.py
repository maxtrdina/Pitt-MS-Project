from os import walk
from os import remove
from os import popen
from os import path
import sys
import grpc
import os
from time import sleep
import string

sys.path.append(
    os.path.join(os.path.dirname(os.path.abspath(__file__)),
                 'utils/'))
import p4runtime_lib.bmv2
from p4runtime_lib.switch import ShutdownAllSwitchConnections
import p4runtime_lib.helper


WORKSPACE = "ext_controller"
REQUESTS = "reqs"

REQ_DIR = "%s/%s" % (WORKSPACE, REQUESTS)


def p4_mac(mac_str):
    parts = mac_str.split(":");
    return "8w0x%s++8w0x%s++8w0x%s++8w0x%s++8w0x%s++8w0x%s" % (parts[0], parts[1], parts[2], parts[3], parts[4], parts[5])
    

def p4_ip(ip_str):
    parts = ip_str.split(".");
    return "8w%s++8w%s++8w%s++8w%s" % (parts[0], parts[1], parts[2], parts[3])


def generate_p4(client_ip, client_mac, rules):
    template_file = open('ext_controller/templates/edge_template.p4', mode='r')
    template_string = template_file.read()
    template_file.close()

    next_hop_mac = "8w8++8w0++8w0++8w0++8w3++8w0"
    
    incoming_rule_template = string.Template("( $agentAddr, $agentPort, my_host_ip, _): fake_source(my_host_mac, $dstAddr, $dstPort, 1);")
    incoming_rules = []
    
    outgoing_rule_template = string.Template("( my_host_ip, _, $dstAddr, $dstPort): reroute(next_hop_mac, $agentAddr, $agentPort, 2);")
    outgoing_rules = []
    
    for rule in rules:
        parts = rule.split("-")
        dst = parts[0].split(":")
        agent = parts[1].split(":")
        
        incoming_rules.append(
            incoming_rule_template.substitute(
                agentAddr=p4_ip(agent[0]),
                agentPort=int(agent[1]),
                dstAddr=p4_ip(dst[0]),
                dstPort=int(dst[1])
            )
        )
        
        outgoing_rules.append(
            outgoing_rule_template.substitute(
                agentAddr=p4_ip(agent[0]),
                agentPort=int(agent[1]),
                dstAddr=p4_ip(dst[0]),
                dstPort=int(dst[1])
            )
        )

    template = string.Template(template_string)
    prog = template.substitute(
        my_host_ip=client_ip,
        my_host_mac=client_mac,
        next_hop_mac=next_hop_mac,
        addresses="",
        ports="",
        incoming_rules="\n".join(incoming_rules),
        outgoing_rules="\n".join(outgoing_rules)
    )

    out = open("ext_controller/replacement.p4", "w")
    out.write(prog)
    out.close()
    return 0
    

def install_switch_program(switch_addr):
    print "Compiling program"
    stream = popen('p4c-bm2-ss --p4v 16 --p4runtime-files build/replacement.p4.p4info.txt -o build/replacement.json ext_controller/replacement.p4')
    out = stream.read()
    print out
    
    p4info_helper = p4runtime_lib.helper.P4InfoHelper("build/replacement.p4.p4info.txt")
    
    print "Creating switch connection"
    s1 = p4runtime_lib.bmv2.Bmv2SwitchConnection(
        name='s1',
        address=switch_addr,
        device_id=0,
        proto_dump_file='logs/s1-p4runtime-requests.txt'
    )
    
    print "Installing program"
    s1.MasterArbitrationUpdate()
    s1.SetForwardingPipelineConfig(
        p4info=p4info_helper.p4info,
        bmv2_json_file_path="build/replacement.json"
    )
    print "Closing switch connection"
    ShutdownAllSwitchConnections()


def process(filename):
    handle = open("%s/%s" % (REQ_DIR, filename))
    data = handle.readlines()
    handle.close()
    
    switch_addr = data[0]
    client_ip = data[1]
    client_mac = data[2]
    new_rule = data[3]
    
    p4_client_ip = p4_ip(client_ip.strip())
    p4_client_mac = p4_mac(client_mac.strip())
    
    print "Got request!"
    print switch_addr
    print client_ip
    print p4_client_ip
    print client_mac
    print p4_client_mac
    print new_rule
    
    generate_p4(client_ip=p4_client_ip, client_mac=p4_client_mac, rules=[new_rule.strip()])
    install_switch_program(switch_addr.strip())
    
    requestId = filename[1:]
    
    f = open("%s/ack%s" % (REQ_DIR, requestId), "w")
    f.close()


def run():
    exit = False

    while not exit:
        files = []
        for (dirpath, dirnames, filenames) in walk(REQ_DIR):
            files.extend(filenames)
        
        # print "read %d files" % (len(files))
        for f in files:
            if f == "exit":
                exit = True
                break
            
            if f.startswith("r"):
                process(f)
                remove("%s/%s" % (REQ_DIR, f))
            
        # 50 millis
        # print "Sleeping"
        sleep(0.05)

    remove("%s/%s" % (REQ_DIR, "exit"))
    print "Exiting"


if __name__ == '__main__':
    run()

