import socket
import sys


def deal_with_connection(in_sock):
    print "Incoming..."
    data = bytearray()
    while True:
        buf, addr = in_sock.recvfrom(4096)
        data.extend(buf)
        
        if len(buf) < 4096:
            break
    
    
    print "Got: %s" % data.decode()
    
    f_out = open("h2_in.txt", "w")
    f_out.write(data.decode())
    f_out.close()
    
    print "Relaying..."
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('10.0.3.3', 7341))
    s.send(data)
    buf = s.recv(4096)
    data = bytearray()
    data.extend(buf)
    print "Got back: %s" % data.decode()
    s.close()
    
    in_sock.send(data)
    


sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind(('10.0.2.2', 7342))
sock.listen(1)

print "Listening to socket"

while True:
    try:
        connection, clientAddress = sock.accept()
        deal_with_connection(connection)
        connection.close()
    except Exception:
        break

sock.close()


