import os
import sys
TMP='''PASS password\r\nNICK nick{0}\r\nUSER user{0} 0 * abc\r\nJOIN #test\r\nPRIVMSG #test :hello world!\r\n'''


for clinet in range(int(sys.argv[1])):
    cmd = TMP.format(clinet)
    filename = "files/infile{0}".format(clinet)
    with open(filename, "w") as fp:
        fp.write(cmd)
    os.system("nc 127.0.0.1 6697 < '" + filename + "'")