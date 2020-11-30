import socket
import sys


f_out = open("h3_in.txt", "w")
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('localhost', 11999))

try:
    print "Accepting socket..."
    
    while True:
        data, addr = sock.recvfrom(4096)
        print "Read some data..."
        
        print "Got some data: %s" % data.decode()
        f_out.write("Received: ")
        f_out.write(data.decode())
    
except Exception as e:
    print e

sock.close()
f_out.close()
