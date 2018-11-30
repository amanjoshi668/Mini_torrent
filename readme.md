Mini Torrent
The code is divided into two parts one is Tracker and other one is client
For running seperately compile both the client and tracker file by using make
for starting tracker cd to tracker folder
    ./tracker <my_tracker_ip>:<my_tracker_port> <other_tracker_ip>:<other_tracker_port>
        <seederlist_file> <log_file>

for starting client cd to client folder
    ./client <CLIENT_IP>:<UPLOAD_PORT> <TRACKER_IP_1>:<TRACKER_PORT_1>
        <TRACKER_IP_2>:<TRACKER_PORT_2> <log_file>

There are multiple commands in the client
    For SHARING the file : share <local file path> <filename>.mtorrent
    For Downloading the remote file :  get <path to .mtorrent file> <destination path>
    View Downloads :  get <path to .mtorrent file> <destination path>
    Removing File : remove <filename.mtorrent>
    For exiting : close

Mtorrent files are created in the current working directory
Logfiles are automatically generated.

