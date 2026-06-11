## Initial Setup

The Cobalt Strike client compiles the implant through an Aggressor Script. Therefore your machine running the CS client must be on Linux to have the necessary dependencies to build this project. In this documentation I show how to setup this project, as well as building manually if you would like.

### Setting up project on Linux:

##### 1. Place project in appropriate location
Clone this project onto the Linux machine where you use the Cobalt Strike client. Then open the `CobaltStrike-Linux-Beacon/CustomBeacon.cna` file and set the absolute path to the project folder on the first line.

##### 2. Retrieve Teamserver Publickey
These steps are to retrieve the teamserver's publickey from `.cobaltstrike.beacon_keys` so that it can be used by the Linux Beacon for initial communication.

First, download the `CobaltStrike-Linux-Beacon/generate-payload/Dump.java` file onto your teamserver.
Next, navigate to the teamserver folder containing `.cobaltstrike.beacon_keys`. 
Then, run this command:

```
java -cp "/path/to/cobaltstrike-client.jar" /path/to/Dump.java
```
This will output "publickey.txt". Move this file into the `CobaltStrike-Linux-Beacon/generate-payload/` folder back on the Linux machine where you are running the CS client.

##### 3. Install Build Dependencies
Run these commands on your Cobalt Strike client Linux machine to install dependencies.
```bash
sudo apt-get update
sudo apt-get install -y build-essential libssl-dev libcurl4-openssl-dev
```
* Also make sure you also have Python3 installed

##### 4. Import CustomBeacon.cna Cobalt Strike
Load the `CustomBeacon.cna` Aggressor Script into your client's Script Manager.

Then you can begin generating your payloads with a click of a button!


### Manually build payload (no Aggressor Script)
This is particularly useful if you use the CS client on Windows and can't import/use the Aggressor Script to build the implant. You would manually build the payload on Linux using the steps below and you can interact with the implant like normal after it executes and connects to the teamserver.

After completing steps 1, 2, and 3 (installing dependencies & preparing publickey), do this:

4. Go into the `generate-payload/` folder

5. Run `python3 InsertPublicKey.py` to insert the public key into the implant.

6. Run `python3 InsertListenerInfo.py [HOST_IP] [PORT] [0/1]`
    * 0 = use HTTP, 1 = use HTTPS

7. Then run `make`.

### Running Linux BOFs
There are 2 sample BOFs that are provided with this project (Thank you TrustedSec and Outflank). Their commands are `linux_id` and `linux_cat <file>`

To use them, first run `./build.sh` in the `sample_linux_bofs/` directory to build them.

Then load the `execute_linux_bof.cna` Aggressor Script.

Now you can use test them out!

When developing your own BOFs,
If your custom BOF use a separate Aggressor Script and the functionality is not in `execute_linux_bof.cna`, you must add this include into it:
```
include("/path/to/CobaltStrike-Linux-Beacon/CustomBeacon.cna");
```

This is so that you will get access to the `beacon_inline_execute_linux` function to actually execute it.