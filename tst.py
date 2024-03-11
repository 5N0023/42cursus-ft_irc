from pwn import *





for i in range(100):
    p = remote("localhost", 6670)
    p.sendline("PASS password")
    p.sendline("NICK nick{0}".format(i))
    p.sendline("USER user{0} 0 * abc".format(i))
    p.sendline("JOIN #channel{0}".format(i))
    p.sendline("PRIVMSG #channel{0} :hello world!".format(i))
    p.sendline("QUIT")
    p.close()
