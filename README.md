# Niflheim
**CS 254 DIGITAL LOGIC DESIGN LAB\
Railway Signal Controller**

### VHDL COMPILATION
Setting Constants in top_level.vhdl file
1. Change the coordinates to whatever you want in line 141 in the signal.
2. Change the value of 'i' for the channel in line 140 where '2i' and '2i + 1' are the channels.


We have included in the "VHDL_Related_Files" folder, a Xilinx ISE 14.7 Project as is. To open this project in Xilinx ISE, click on "Open Project" and select the file called "DLD_Proj.xise" from this folder. The project will now get imported into Xilinx ISE and it can be compiled completely by running the following in order:
- Synthesize
- Implement Design
- Generate Programming File

The .bit file is now ready and can be used to program the board. 

On linux systems, the following command can be used to program the board with the .bit file:\
`djtgcfg -d Atlys -i 0 prog -f top_level.bit`\
*NOTE: "top_level.bit" should be replaced with actual path to the .bit file. On windows systems, use Digilent Atlys utility to program the board.*


### C COMPILATION 
In main.c change the path of "network.txt" with the appropriate path in line 1263.

We have included a C file called "main.c" in the "C_Related_Files" folder. Copy this file into the "20140524/makestuff/apps/flcli" directory and from the same directory, run the "script.sh"

This will compile the modified flcli executable, which can now be executed by running:
`sudo ~/20140524/makestuff/apps/flcli/lin.x64/rel/flcli -v 1d50:602b:0002 -i 1443:0007`

`sudo ~/20140524/makestuff/apps/flcli/lin.x64/rel/flcli -v 1d50:602b:0002 -y`

*NOTE: "~/20140524/makestuff/apps/flcli/lin.x64/rel/flcli" should be replaced with actual path to flcli executable*


### UART COMMUNICATION
We have completed the optional part using a laptop as a relay. 

To establish UART communication via a relay, run:

`sudo insmod xr_usb_serial_common.ko`

`sudo socat -xvd /dev/ttyXRUSB0,raw,echo=0,b2400 /dev/ttyXRUSB1,raw,echo=0,b2400`
*NOTE: "xr_usb_serial_common.ko" might need to be replaced with the actual path to this file*

On the neighbouring controller, data sent over UART will be displayed on the terminal and will be relayed to the other controller. 