import socket
from time import sleep

def main():
    sock = socket.socket()

    sock.connect(("127.0.0.1", 23669))

    # sleep(5)


    print(sock.send(b"A" * 1023 + b"\x00"))

    # print(sock.recv(100))

if __name__ == "__main__":
    main()
