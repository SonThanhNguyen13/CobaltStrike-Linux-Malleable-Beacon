import os
import sys
import subprocess

C2_SERVER_string    = "#define C2_SERVER"
C2_PORT_string      = "#define C2_PORT"
C2_USE_HTTPS_string = "#define C2_USE_HTTPS"


def InsertListenerInfo(Target_Server, Target_Port, HTTPS_value):

    # Read original file
    with open("../implant/headers/config.h", "r") as f:
        lines = f.readlines()

    # Write modified file
    with open("../implant/headers/config.tmp", "w") as f:
        for line in lines:               
            if C2_SERVER_string in line:
                f.write(f'{C2_SERVER_string} "{Target_Server}"\n')
            elif C2_PORT_string in line:
                f.write(f'{C2_PORT_string} {Target_Port}\n')
            elif C2_USE_HTTPS_string in line:
                f.write(f'{C2_USE_HTTPS_string} {HTTPS_value}\n')
            else:
                f.write(line)

        print("Inserted listener info into config.h!")


    os.replace("../implant/headers/config.tmp", "../implant/headers/config.h")
    

if __name__ == "__main__":

    TargetServer = sys.argv[1]
    TargetPort = sys.argv[2]
    HTTPSvalue = sys.argv[3]

    InsertListenerInfo(TargetServer, TargetPort, HTTPSvalue)
