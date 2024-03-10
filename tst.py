from pwn import *





for i in range(100):
    p = remote("localhost", 6670)
    p.sendline("PASS password")
    print(p.recvline())
    p.sendline("NICK nick{0}".format(i))
    print(p.recvline())
    p.sendline("USER user{0} 0 * abc".format(i))
    print(p.recvline())
    p.sendline("JOIN #channel{0}".format(i))
    print(p.recvline())
    p.sendline("PRIVMSG #channel{0} :hello world!".format(i))
    print(p.recvline())
    p.sendline("QUIT")
    print(p.recvline())
    p.close()
