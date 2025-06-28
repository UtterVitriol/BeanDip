import socket
from time import sleep

def main():
    sock = socket.socket()
    sock2 = socket.socket()

    sock.connect(("127.0.0.1", 23669))
    sock2.connect(("127.0.0.1", 23669))
    for x in range(0x31, 0x3a):
        sleep(1)
        print(x.to_bytes())
        sock.send(x.to_bytes())
        sock2.send(x.to_bytes())

if __name__ == "__main__":
    main()
