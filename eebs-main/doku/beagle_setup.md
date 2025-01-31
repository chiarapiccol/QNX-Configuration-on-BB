# Connect to BeagleBone via USB+putty and boot from SD-Card

## Physical connection
Connect USB to BB' J1 (UART)
- connect USB's GND to J1's GND (1st pin from dot)
- connect USB's TX to J1's 4th pin from dot (RX)
- connect USB's RX to J1's 5th bin from dot (TX)

## tty connection
- Start putty with root permissions
- Load serial configuration
    - a) first setup:
        - serial: set
            - port (/dev/ttyUSB0)
            - baud (115200)
            - data (8)
            - parity (none)
            - stop (1)
            - fc (none)
        - session:
            - select serial
            - save config
    - b) following runs:
        - load config
        - connect

## setup bootable SD-card
### obtain necessary files
- "MLO" from moodle
- "u-boot.img" from moodle (this is the standard image - if new properties are needed, this shall be updated (see next section))
- "ifs-ti-am335x-beaglebone.bin" by compiling the project (BSP_ti-am335x-beaglebone.zip.zip) using Momentics (in VM)
    --> ifs file in images dir
### formatting
- if SD-card corrupted (should not be necessary):
    create new partitiontable: "msdos"
- format (using for example gparted) the whole SD-card to the fat32 Filesystem
### mounting
- if SD-card is not detected by OS, manually mount it:
    - `sudo mkdir /mnt/<dir name>` to create dir in which to mount SD-Card
    - `lsblk` &rarr; search for 1.8G volume (should be "/dev/mmcblk0p1" (partition 1 on /dev/mmcblk0)
    - `sudo mount /dev/<device name> /mnt/<dir name>`
### copy files to SD-Card
- `sudo cp <path to MLO> /dev/<dir name>`
- `sudo cp <path to u-boot.img> /dev/<dir name>`
- `sudo cp <path to ifs> /dev/<dir name>`

## boot from SD-Card
- connect to BeagleBone via USB as mentioned above
- start putty: 'sudo putty'
- setup / load serial profile as mentioned above
- start tty
- plug in BeagleBone's powersupply
- interupt autostart by pressing any key
- &rarr; u-boot prompt ("=>") should be displayed
- enter
    - `mmcinfo`
    - `fatload mmc 0 81000000 ifs-ti-am335x-beaglebone.bin`
    - `go 81000000`
- &rarr; ksh prompt ("#") should be displayed


# Configure QNX image in Momentics (using the build-file)
Kernel configuration (what drivers are written into the image, what is executed on startup, ...) is done in a build-file (`images/beaglebone.build`).
Configuration is done in `[+script] .script = {...}` (this is executed on startup).

Everything specified in the buildfile can also be done in the QNX-terminal

## Setting up a Build Target, so that the build-file is not overwritten
When running build the regular way, the image is created from the bottom up, possibly overwriting the build-file.
To avoid this, create a build target:
- Right-click on images
- Build Targets &rarr; create
- name "all" (as specified in `images/Makefile`)
&rarr; From now on: Double-click on all to build the image (without a complete re-build).

## Manual changes to the build file
1. In Momentics:
    - Perform needed changes in buildfile
    - Run (Double-click) "all" build target (so that theere is no overwriting)
2. Move files manually to SD
3. Boot from SD
Recommended sanity-check: Append a message ("hello world" / version number / ...) to `display_msgr in the build-file.

# Setting up the NCM connection

## On BB
### Buildfile changes
uncomment lines
```
ulink_ctrl
... TODO ...
```

### Verify, the ncm-device exists
Run
```
ifconfig
```
The output should show
```
...

enx020000040506: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet6 fe80::458:6b25:ace3:e0e7  prefixlen 64  scopeid 0x20<link>
        ether 02:00:00:04:05:06  txqueuelen 1000  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 12  bytes 1936 (1.9 KB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

...
```

## On Host
The best way is to configure QNX using a dhcp server to dynamically assign an IP-adress a newly connected host.
For simplicity, we manually assign a static IP-adress.

### Test, if NCM-Setup was successful
Ensure, NCM connection was created
```
sudo dmesg -T | grep -i ncm
```
Should print something like
```
[Mon Nov 25 19:48:02 2024] cdc_ncm 2-2:1.0 enx020000040506: renamed from eth0
[Mon Nov 25 19:49:28 2024] cdc_ncm 2-2:1.0 enx020000040506: unregister 'cdc_ncm' usb-0000:06:00.3-2, CDC NCM (NO ZLP)
[Mon Nov 25 19:49:40 2024] usb 2-2: Product: QNX NCM Network Device
[Mon Nov 25 19:49:40 2024] cdc_ncm 2-2:1.0: MAC-Address: 02:00:00:04:05:06
[Mon Nov 25 19:49:40 2024] cdc_ncm 2-2:1.0: setting rx_max = 16384
[Mon Nov 25 19:49:40 2024] cdc_ncm 2-2:1.0: setting tx_max = 16384
[Mon Nov 25 19:49:40 2024] cdc_ncm 2-2:1.0 eth0: register 'cdc_ncm' at usb-0000:06:00.3-2, CDC NCM (NO ZLP), 02:00:00:04:05:06
[Mon Nov 25 19:49:40 2024] cdc_ncm 2-2:1.0 enx020000040506: renamed from eth0
```

### Assign IP-address to QNX (for testing the connection)
- list IP-devices using `ip a`. One of the outputs should be something like
    ```
    3: enx020000040506: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc fq_codel state UP group default qlen 1000
    link/ether 02:00:00:04:05:06 brd ff:ff:ff:ff:ff:ff
    inet 192.168.10.101/24 brd 192.168.10.255 scope global enx020000040506
       valid_lft forever preferred_lft forever
    ```
- assign an IP-address to QNX
    ```
    sudo ifconfig enx020000040506 192.168.10.101
    ```
    Where `enx020000040506` is the device name and `192.168.10.101` is the IP-address assigned to the BB in the buildfile plus 1
    
- Host to Target: `ping 192.168.10.100` should be successful
    &rarr; if not, it is possible (but not necessary) that host's network manager has interfered, which would not be a problem (see below for fix)

- Target to Host: `ping 192.168.10.100` should be successful

### Fix NetworkManager interference
Disable NetworkManager's handling of the QNX interface.
1. Edit
    ```
    /etc/NetworkManager/NetworkManager.conf
    ```
    (using `sudo`)
    to include
    ```
    [keyfile]
    unmanaged-devices=mac:<mac-adress of QNX interface>
    ```
    The <mac-adress of QNX interface> can be found in the output of `ip a` (it has the format `xx:xx:xx:xx:xx:xx`).

Restart NetworkManager service using `sudo systemctl restart NetworkManager`.
`ifconfig` should now yield
```
enx020000040506: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet6 fe80::458:6b25:ace3:e0e7  prefixlen 64  scopeid 0x20<link>
        ether 02:00:00:04:05:06  txqueuelen 1000  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 82  bytes 13556 (13.5 KB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
```
Now, give yourself an IP-adress using `ifconfig enx020000040506 192.168.10.101`


### Pass the connection through to the VM
- make sure, the host user is in the group `vboxusers`
    ```
    sudo usermod -a -G vboxusers $USER
    ```
- In Virtualbox, activate "QNX Software Systems QNX NCM Network Device"
    - On running VM: right-click on the bottom right of the running VM on the USB-Symbol and check the appropriate box
    - General: Add appropriate USB-Device-Filter to the VM (usb click button on the VM link unten und the +button)



# Run Projects on BB
## Create new QNX Project
In the Momentics IDE create a new project.
Make sure to set "lanch cofiguration" as test and the "on: target" as the right IP-Adresse (in our case 192.168.10.100). 
Click the green arrow to run the programm

- Create new Project
    - Besides the ti-am335x-beaglebone, right click
    - &rarr; "New" 
    - &rarr; "QNX Project"
    - &rarr; "C/C++"
    - &rarr; "QNX Executable"
- Edit sourcefiles in `<project name>/src/`
- Compile by clicking "Build" (in the top left)

## Make program executable on BeagleBone
### Manually
- Open Target file system navigator
    - "Window"
    - &rarr; "Show View"
    - &rarr; "Target File System Navigator"
- Copy binary to the BeagleBone's filesystem (for example into `/tmp/`)

### Via the build-file
TODO

# Calibration
Let `waste_time` run in highest priority **during calibration**, so that it is not interrupted for maximal accuracy.

# Stacksize calibration: "Hochwassermarke"
Threads accept pre-allocated memory to use as stack.
&rarr;   - pre-allocate memory
        - fill memory with recognizable pattern
        - execute thread (for different amounts of time?)
        - check, until where pattern is changed (using binary-search?)

## Make program executable on BeagleBone
### Manually
- Open Target file system navigator
    - "Window"
    - &rarr "Show View"
    - &rarr "Target File System Navigator"
- Copy binary to the BeagleBone's filesystem (for example into `/tmp/`)

### Via the build-file
TODO

# Calibration
Let `waste_time` run in highest priority **during calibration**, so that it is not interrupted for maximal accuracy.

# Stacksize calibration: "Hochwassermarke"
Threads accept pre-allocated memory to use as stack.
&rarr   - pre-allocate memory
        - fill memory with recognizable pattern
        - execute thread (for different amounts of time?)
        - check, until where pattern is changed (using binary-search?)
