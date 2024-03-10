import os
import sys
TMP='''PASS password \r\nNICK nick{0}\r\nUSER user{0} 0 * abc\r\nJOIN #channel{0}\r\nPRIVMSG #channel{0} :hello world!\r\n'''


for clinet in range(int(sys.argv[1])):
    cmd = TMP.format(clinet)
    filename = "files/infile{0}".format(clinet)
    with open(filename, "w") as fp:
        fp.write(cmd)
    os.system("nc 10.12.9.5 6670 < '" + filename + "'")