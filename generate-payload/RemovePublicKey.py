import os
import subprocess

search_string = "unsigned char BEACON_PUBLIC_KEY[256] ="


def RemovePublicKey():
    replacement_line = f'unsigned char BEACON_PUBLIC_KEY[256] = "\\x30";\n'

    # Read original file
    with open("../implant/src/beacon.c", "r") as f:
        lines = f.readlines()

    # Write modified file
    with open("../implant/src/beacon.tmp", "w") as f:
        for line in lines:
            if search_string in line:
                f.write(replacement_line)
                print("Replaced public key line with placeholder!")
            else:
                f.write(line)


    os.replace("../implant/src/beacon.tmp", "../implant/src/beacon.c")
    

if __name__ == "__main__":
    RemovePublicKey()
