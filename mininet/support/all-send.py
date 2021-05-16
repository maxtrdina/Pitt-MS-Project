import socket
import sys

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.connect((sys.argv[1], int(sys.argv[2])))

data = bytearray(sys.argv[3], encoding="utf-8")

print("Sending \"%s\" to (%s %s)" % (sys.argv[3], sys.argv[1], sys.argv[2]))

s.send(data)
#buf = s.recv(4096)
#data = bytearray()
#data.extend(buf)

#print data.decode()

#f_out = open("h1_in.txt", "w")
#f_out.write(data.decode())
#f_out.close()

s.close()
