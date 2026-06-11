// Original tool: https://gist.github.com/olliencc/af056560e943bafa145120103a0947a3#file-dump-java

import java.io.File;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Base64;
import common.CommonUtils;
import java.security.KeyPair;

class DumpKeys
{
    private static String bytesToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder(bytes.length * 2);
        for (byte b : bytes) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }

    public static void main(String[] args)
    {
        try {
            File file = new File(".cobaltstrike.beacon_keys");
            if (file.exists()) {
                KeyPair keyPair = (KeyPair) CommonUtils.readObject(file, null);

                // Print private key (Base64)
                /*
                System.out.printf(
                    "Private Key: %s%n%n",
                    Base64.getEncoder().encodeToString(
                        keyPair.getPrivate().getEncoded()
                    )
                );
                */

                // Convert public key to hex
                String publicKeyHex = bytesToHex(
                    keyPair.getPublic().getEncoded()
                );

                // Write public key hex to file "publickey"
                Files.write(Path.of("publickey.txt"), publicKeyHex.getBytes());

                System.out.println("Public key written to file: publickey.txt");
            }
            else {
                System.out.println("Could not find .cobaltstrike.beacon_keys file");
            }
        }
        catch (Exception exception) {
            System.out.println("Could not read asymmetric keys");
        }
    }
}
