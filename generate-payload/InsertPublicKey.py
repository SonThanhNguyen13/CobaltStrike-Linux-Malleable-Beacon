import os
import subprocess

search_string = "unsigned char BEACON_PUBLIC_KEY[256] ="

def hex_to_c_string(hex_string):
    hex_string = hex_string.strip().replace(" ", "")
    return ''.join(f'\\x{hex_string[i:i+2]}' for i in range(0, len(hex_string), 2))


def InsertPublicKey():
    # Read hex from file
    with open("publickey.txt", "r") as f:
        hex_data = f.read()

    c_string = hex_to_c_string(hex_data)
    replacement_line = f'unsigned char BEACON_PUBLIC_KEY[256] = "{c_string}";\n'

    # Read original file
    with open("../implant/src/beacon.c", "r") as f:
        lines = f.readlines()

    # Write modified file
    with open("../implant/src/beacon.tmp", "w") as f:
        for line in lines:
            if search_string in line:
                f.write(replacement_line)
                print("Inserted public key into beacon.c!")
            else:
                f.write(line)


    os.replace("../implant/src/beacon.tmp", "../implant/src/beacon.c")
    

if __name__ == "__main__":
    InsertPublicKey()
