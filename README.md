This is the next generation Linux driver for the FSCC line of cards from
Commtech, Inc. Version 2 is a complete rewrite from the original. The original
driver was based largely in code that was written for 2.4 era kernels and then
ported over so there were some spots where it was not exactly using best
practices for today's kernels. The version 2 driver is essentially complete.
There are some features and functions that have not been integrated yet, but
hopefully we will be able to add them soon. WARNING: THIS DRIVER IS NOT YET
CONSIDERED A STABLE RELEASE. THERE ARE STILL SOME MISSING FUNCTIONS AND
FEATURES AND ARE LIKELY SOME BUGS.  Use at your own risk.  If you find a bugfix
and would like to submit it to us please email the patch or modified code to
techsupport@commtech-fastcom.com.

I.     Getting the latest driver software
       1) Switching between driver versions

II.    Compiling the driver
       1) Enabling debug prints
       2) Specifying non-default header files directory

III.   Loading/installed the driver
       1) Installing the driver
       2) Enabling hot plugging support
       3) Disabling DMA support

V.     Changing a port's register values
       1) Using sysfs from the command line
       2) Using the ioctl call from within your application code
       3) Using the Python API from within your application code
       4) Using the config.h file for modifying the boot defaults

VI.    Reading a port's register values
       1) Using sysfs from the command line
       2) Using the ioctl call from within your application code
       3) Using the Python API from within your application code

VII.   Asynchronous communication

VIII.  Setting the clock frequency
       1) Using the ioctl call from within your application code

IX.    Operating the driver
       1) Writing data in C code
       2) Writing data from the command line
       3) Writing data using the Python API
       4) Reading data in C code
       5) Reading data from the command line
       6) Reading data using the Python API

X.     Viewing/setting frame status data
       1) Using sysfs from the command line
       2) Using the ioctl call from within yzour application code
       3) Using the config.h file for modifying the boot defaults

XI.    Viewing/setting memory constraints
       1) Using sysfs from the command line
       2) Using the ioctl call from within yzour application code
       3) Using the config.h file for modifying the boot defaults

XII.   Purging data out of driver
       1) Using sysfs from the command line
       2) Using the ioctl call from within your application code
       2) Using the Python API from within your application code

XIII.  Migrating from the 1.x driver to the 2.x driver
       1) Setting register values
       2) Getting register values
       3) Purging transmit and receive data
       4) Getting the frame status
       5) Changing the clock frequency

XIV.   FAQ
       1) Ports are not created even though the driver has loaded
       2) What does poll() and select() base their information on
       3) Executing a purge without a clock puts the card in a broken state
       4) Receiving the error message 'Couldn't register serial port'
       5) Not seeing card in sysfs


## Installation Instructions
### Downloading Source Code
The source code for the FSCC driver is hosted on Github code hosting. To check
out the latest code you will need Git and to run the following in a terminal.

```
git clone git://github.com/commtech/fscc-linux.git fscc
```

NOTE: We prefer you use the above method for downloading the driver (because it
      is the easiest way to stay up to date) but you can also get the driver
      from the
      [download page](https://github.com/commtech/fscc-linux/tags/).

Now that you have the latest code checked out you will more than likely want
to switch to a stable version within the code directory. To do this browse
the various tags for one you would like to switch to. Version v2.2.1 is only
listed here as an example.

```
git tag
git checkout v2.2.1
```


### Compiling Driver
Compiling the driver is relatively simple assuming you have all of the
required dependencies. Typically you will need gcc, make and your kernel's
header files. After assembling all of these things you can build the driver by
simply running the make command from within the source code directory.

```
cd fscc/
make
```

If you would like to enable debug prints within the driver you need to add
the DEBUG option while building the driver.

```
make DEBUG=1
```

Once debugging is enabled you will find extra kernel prints in the
/var/log/messages and /var/log/debug log files.

If the kernel header files you would like to build against are not in the
default location `/lib/modules/$(shell uname -r)/build` then you can specify
the location with the KDIR option while building the driver.

```
make KDIR="/location/to/kernel_headers/"
```


### Loading Driver
Assuming the driver has been successfully built in the previous step you are
now ready to load the driver so you can begin using it. To do this you insert
the driver's kernel object file (fscc.ko) into the kernel.

```
insmod fscc.ko
```

NOTE: You will more than likely need administrator privileges for this and
      the following commands.

By default if there are no cards present when the driver is loaded the
insmod will fail with a 'No such device' error. To allow the driver to load
even if no cards are present turn on the 'hot_plug' option.

```
insmod fscc.ko hot_plug=1
```

If no cards are present and the hot_plug option is not enabled (default) you
will see this.

```
insmod fscc.ko hot_plug=0
insmod: error inserting 'fscc.ko': -1 No such device
```

If you would like to disable DMA for cards (SuperFSCC* line) that have that
capability you can do so by using the 'force_fifo' option. This option will
disable DMA and allow you to operate in the FIFO based mode.

```
insmod fscc.ko force_fifo=1
```

NOTE: All driver load time options can be set in your modprobe.conf file for
      using upon system boot.


### Installing Driver
If you would like the driver to load automatically at boot use the included
installer.

```
make install
```

This will also install the header (.h) files, python library and command line
program for using the card.

To uninstall, use the included uninstaller.

```
make uninstall
```


### Changing Register Values
The FSCC driver is a swiss army knife of sorts with communication. It can
handle many different situations if configured correctly. Typically to
configure it to handle your specific situation you need to modify the card's
register values.

There are multiple ways of modifying the card's registers varying from simply
modifying a sysfs file to an ioctl call within code. Here are a few ways of
doing this.

NOTE: For a listing of all of the configuration options please see the manual.

Echo the register value in hexadecimal form to the sysfs file with the same
name of the register you would like to modify.

```
echo 030000ff > /sys/class/fscc/fscc0/registers/bgr
```

Use the FSCC_SET_REGISTERS ioctl to set the values of any registers you
need to modify from within C code. This ioctl can be found within
`<fscc/fscc.h>` after you install the header files (see section IV).

```c
struct fscc_registers regs;

FSCC_REGISTERS_INIT(regs);

regs.BGR = 0x030000ff;
regs.FCR = 0x00000000;

ioctl(port_fd, FSCC_SET_REGISTERS, &regs);
```

NOTE: A complete example of how to do this can be found in the file
      fscc-linux/examples/c/set-registers.c.

Use the Python API to easily set the values of any registers you need to
modify from within Python code.

```python
port.registers.BGR = 0x030000ff
port.registers.FCR = 0x00000000
```

NOTE: A complete example of how to do this can be found in the file
      fscc-linux/examples/python/set-registers.py.

Modify the '#define DEFAULT_[BGR|CCR0|...]_VALUE 0x030000ff' lines within
the config.h file to be whatever you would like the card to boot up as. You
will need to recompile the driver after doing this.

NOTE: This will set all ports to this at driver boot up. It is a driver wide
      setting.


### Reading Register Values
There are multiple ways of reading the card's registers varying from simply
modifying a sysfs file to an ioctl call within code. Here are a few ways of
doing this.

Cat the register value in hexadecimal form from the sysfs file with the same
name of the register you would like to view.

```
cat /sys/class/fscc/fscc0/registers/bgr
00000001
```

Use the FSCC_GET_REGISTERS ioctl to get the values of any registers you
need to read from within code. This ioctl can be found within
`<fscc/fscc.h>` after you install the header files (see section IV).

```c
struct fscc_registers regs;

FSCC_REGISTERS_INIT(regs);

regs.BGR = FSCC_UPDATE_VALUE;
regs.FCR = FSCC_UPDATE_VALUE;

ioctl(port_fd, FSCC_GET_REGISTERS, &regs);
```

At this point 'regs.BGR' and 'regs.FCR' would be set to their respective
values.

NOTE: A complete example of how to do this can be found in the file
      fscc-linux/examples/c/get-registers.c.

Use the Python API to easily get the values of any registers you need to
read from within Python code.

```python
"0x%08x" % port.registers.BGR
"0x00000000"

"0x%08x" % port.registers.FCR
"0x00000000"
```

NOTE: A complete example of how to do this can be found in the file
      fscc-linux/examples/python/get-registers.py.


### Asynchronous Communication
The FSCC driver lets the built in Linux serial driver handle asynchronous
communication for our UARTs. The Linux serial driver is highly tested and
likely more stable than anything we could produce in any reasonably amount of
time.

Prior to and after loading the FSCC driver (see section III) there are a few
steps needed to get the card ready for asynchronous communication.

Some Linux distributions have the default number of serial ports that are
available at boot set to a small number (usually 4). The first four serial
ports are reserved so you will need to change this value to something larger
to be able to configure more serial ports.

There are a couple ways of doing this. The easiest method is by appending
'8250.nr_uarts=x' to your grub boot line. Something like this:

kernel /boot/vmlinuz-2.6.20-15-generic ro quiet splash 8250.nr_uarts=16

This can be done temporarily by pressing 'e' at the grub menu during boot or
by permanently modifying this value which is grub version specific. To do
this please search google for one of the numerous guides on the subject.

Another method is by editing the .config file of you kernel before compiling
it to allow for more serial ports. This is not preferred because you will
need to recompile the kernel for it to take effect. The line you need to
change in the .config file is SERIAL_8250_RUNTIME_UARTS.

Load the FSCC driver. This will handle registering our UARTs with the serial
driver (see section III). Our UART's will now appear as ttyS nodes in the
/dev/ directory.

By default the FSCC driver boots up in synchronous communication mode. To
switch to the asynchronous mode you must modify the FSCC card's FCR register
to allow for asynchronous communication. There are multiple ways of doing
this (see section V). Possibly the simplest method is using sysfs and the
command line.

```
echo 03000000 > /sys/class/fscc/fscc0/registers/fcr
```


### Setting Clock Frequency
The FSCC device has a programmable clock that can be set anywhere from
20 kbit to 200 Mbit. However, this is not the fully operational range an
FSCC port.  This is the range that the onboard clock can be set to.

Using one of the synchronous modes you can only receive data consistently
up to 30 Mbit (when you are using a external clock). If you are transmitting
data using an internal clock you can safely go up 50 Mbit.

Use the FSCC_SET_CLOCK_BITS ioctl to set the frequency from within code.

```c
/* 10 MHz */
unsigned char clock_bits[20] = {0x01, 0xa0, 0x04, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x9a, 0x4a, 0x41,
                               0x01, 0x84, 0x01, 0xff, 0xff, 0xff};

ioctl(port_fd, FSCC_SET_CLOCK_BITS, &clock_bits);
```

NOTE: A complete example of how to do this along with how to calculate
      these clock bits can be found in the file
      fscc-linux/examples/c/set-clock-bits.c.


###  Operating Driver
The FSCC driver typically (but not always) works in "frames". This means that
data typically is packaged together in permanent chunks. If the card received
two frames of data prior to you retrieving the data you will only get one chunk
of data back at a time when you interface with the driver to get the data.

There are multiple ways of reading/writing data to/from the card. Listed below
are only the most common.

Writing data will typically be done within C code using the write()
function found within the `<unistd.h>` file. You can find a more in depth
example named sync-write.c within the examples/c directory. You will need
to look elsewhere for a more in depth explanation of the parameters and
return values.

```
bytes_written = write(port_fd, data, sizeof(data));
```

In in addition to the standard errors that the write() function returns
there are a couple errors specific to the FSCC you might run into.

-EOPNOTSUPP: If trying to use a FSCC port with the /dev/fscc nodes while
             in asynchronous communication mode (should be using /dev/ttyS
             nodes).

-ETIMEDOUT: If trying to use a FSCC port without a transmit clock present.
            This check can be turned off with the 'ignore_timeout' command
            line parameter.

-ENOBUFS: If the count parameter passed into the write() function is larger
          than the output cap. If the count parameter is less than the
          output cap but the amount out of output space isn't enough the
          driver will block instead of returning this error.

NOTE: A complete example of how to do this can be found in the file
      fscc-linux/examples/c/sync-write.c.

A simpler but not nearly as useful way of sending data out of a FSCC port is
by using the built-in linux program echo. This is mainly useful while
debugging.

```
echo "Hello world!" > /dev/fscc0
```

The Python API has a built-in method for writing data to the card.

```python
port.write('Hello world!'.encode())
```

The same errors returned from the C code above can be found while using the
Python API. The only difference is instead of returning an error they throw
an IOError exception with the appropriate errno value set.

NOTE: A complete example of how to do this can be found in the file
      fscc-linux/examples/python/write.py.

Reading data will typically be done within C code using the read() function
found within the `<unistd.h>` file. You can find a more in depth example named
sync-read.c within the examples/c directory. You will need to look elsewhere
for a more in depth explanation of the parameters and return values.

```c
bytes_read = read(port_fd, data, sizeof(data));
```

The length argument of the `read()` function means different things depending
on the mode you are using.

In a frame based mode the length argument specifies the maximum frame size
to return. If the next queued frame is larger than the size you specified
the error -ENOBUFS is returned and the data will remain waiting for a read()
of a larger value. If a `read()` length is specified that is larger than the
length of multiple frames in queue you will still only receive one frame per
read() call.

In streaming mode (no frame termination) the length argument specifies the
maximum amount of data to return. If there is 100 bytes of streaming data
in the card and you `read()` with a length of 50, you will receive 50 bytes.
If you were to do a `read()` of 200 bytes you will receive the 100 bytes
available.

Frame based data and streaming data are kept separate within the driver.
To understand what this means first imagine this scenario. You are in a
frame based mode and receive a couple of frames. You then switch to
streaming mode and receive a stream of data. When calling `read()` you will
receive the the streaming data until you switch back into a frame based
mode then do a `read()`.

In in addition to the standard errors that the read() function returns
there are a couple errors specific to the FSCC you might run into.

-ENOBUFS: If the size parameter passed into the read() function is smaller
          than the next frame (in a frame based mode).

-EOPNOTSUPP: If trying to using a FSCC port using the /dev/fscc nodes while
             in asynchronous communication mode (should be using /dev/ttyS
             nodes).


NOTE: A complete example of how to do this can be found in the file
      fscc-linux/examples/c/sync-read.c.

A simpler but not nearly as useful way of reading data out of a FSCC port is
by using the built-in linux program cat. This is mainly useful while
debugging.

```
cat /dev/fscc0
```

The Python API has a built-in method for read data from the card.

```python
port.read(4096)
```

The same errors returned from the C code above can be found while using the
Python API. The only difference is instead of returning an error they throw
an IOError exception with the appropriate errno value set.

NOTE: A complete example of how to do this can be found in the file
      fscc-linux/examples/python/read.py.

NOTE: fread/fwrite are not allowable ways of interfacing with the card. They
      make assumptions about the data that doesn't hold up with our "frame"
      perspective. This means Python functions like file.write() and
      file.read() will not work because they are wrappers around fread/fwrite.
      You will instead need to use file streams which act more like read/write.


### Viewing/Setting Frame Status
It is a good idea to pay attention to the status of each frame. For example if
you want to see if the frame's CRC check succeeded or failed.

The way the FSCC reports this data to you is by appending two additional bytes
to each frame you read from the card if you opt in to see this data. There are
a couple of ways of enabling this additional data.

The simplest method of doing this is by writing a value of 1 to the
append_status file in sysfs from the command line.

```
echo 1 > /sys/class/fscc/fscc0/settings/append_status
```

You can also check if append status is enabled from the command line.

```
cat /sys/class/fscc/fscc0/settings/append_status
```

Another way of doing this is by using the FSCC_ENABLE_APPEND_STATUS ioctl
from within your C code.

```c
ioctl(port_fd, FSCC_ENABLE_APPEND_STATUS);
```

NOTE: A complete example of how to do this can be found in the file
      fscc-linux/examples/c/append-status.c.

Modify the `#define DEFAULT_APPEND_STATUS_VALUE 1` lines within the config.h
file to be whatever you would like the card to boot up as. You will need to
recompile the driver after doing this.

NOTE: This will set all ports to this at driver boot up. It is a driver wide
      setting.


XI. Viewing/Setting Memory Constraints
-------------------------------------------------------------------------------
For systems with limited memory available to them there is safety checks in
place to prevent spurious incoming data from overrunning your system. Each port
has an option for setting it's input and output memory cap.

There are multiple ways of setting this value.

The simplest method of doing this is by writing a value to the
{input, output}_memory_cap file in sysfs from the command line.

```
echo 1000000 > /sys/class/fscc/fscc0/settings/input_memory_cap
```

You can also check the current memory cap value from the command line.

```
cat /sys/class/fscc/fscc0/settings/input_memory_cap
```

Use the `FSCC_SET_MEMORY_CAP` ioctl to set the memory cap values from within C
code. This ioctl can be found within `<fscc/fscc.h>` after you install the
header files (see section IV).

```c
struct fscc_memory_cap memory_cap;

FSCC_MEMORY_CAP_INIT(memory_cap);

memory_cap.input = 5000000;
memory_cap.output = 10000;

ioctl(port_fd, FSCC_SET_MEMORY_CAP, &memory_cap);
```

NOTE: You can set only 1 of the 2 values by running the `FSCC_MEMORY_CAP_INIT`
      macro on the `fscc_memory_cap` struct then setting only 1 of the values
      in the structure. The `FSCC_MEMORY_CAP_INIT` structure initializes both
      values to -1 which will be ignored in the driver.

NOTE: A complete example of how to do this can be found in the file
      fscc-linux/examples/c/memory-cap.c.

Modify the `#define DEFAULT_{INPUT, OUTPUT}_MEMORY_CAP 100000` lines within
the config.h file to be whatever you would like the card to boot up as. You
will need to recompile the driver after doing this.

NOTE: This will set all ports to this at driver boot up. It is a driver wide
      setting.

### Purging Data
Between the hardware FIFO and the driver's software buffers there are multiple
places data could be at excluding your application code. If you ever need to
clear this data out and start out fresh there are a couple ways of doing this.

The simplest method of doing this is by writing a value of 1 to the
purge_tx and purge_rx file in sysfs from the command line.

```
echo 1 > /sys/class/fscc/fscc0/commands/purge_tx
echo 1 > /sys/class/fscc/fscc0/commands/purge_rx
```

Another way of doing this is by using the `FSCC_PURGE_TX` and `FSCC_PURGE_TX`
ioctl's from within your C code.

```c
ioctl(port_fd, FSCC_PURGE_TX);
ioctl(port_fd, FSCC_PURGE_RX);
```

NOTE: A complete example of how to do this can be found in the files
      fscc-linux/examples/c/purge_tx.c and purge_rx.c.

In in addition to the standard errors that the ioctl() function returns
there is an error specific to the FSCC you might run into.

-ETIMEDOUT: If trying to use a FSCC port without a transmit clock present.
            This check can be turned off with the 'ignore_timeout' command
            line parameter.

The Python API makes this easy by calling a couple built-in methods.

```
port.purge_rx()
port.purge_tx()
```


### Migrating From 1.x to 2.x
There are multiple benefits of using the 2.x driver: accurate posix error
codes, intuitive ioctl calls, backend support for multiple languages (Python,
C#) and dynamic memory management are some.

The 1.x driver and the 2.x driver are very similar so porting from one to the
other should be rather painless.

NOTE: All ioctl values have changed even if their new names match their old
      names. This means even if you use an ioctl with an identical name, it
      will not work correctly.

Setting register values was split into two different ioctl's in the 1.x
driver, setting all the registers at once and one at a time. In the 2.x
driver these two scenarios have been combined into one ioctl.

Change the following ioctl's to the current ioctl `FSCC_SET_REGISTERS` (see
section V).

`FSCC_WRITE_REGISTER` (setting a single register at a time)
`FSCC_INIT` (setting all registers at a time)

Getting register values was limited to one at a time in the 1.x driver. In
the 2.x driver it has been made more convenient to read multiple register
values.

Change the following ioctl to the current ioctl `FSCC_GET_REGISTERS` (see
section VI).

`FSCC_READ_REGISTER` (reading a single register at a time)

Purging transmit and receive data has not changed. Continue using
`FSCC_PURGE_TX` and `FSCC_PURGE_RX`.

For more information on these ioctl's see section XII.

Getting the frame status has now been designed to be configurable. In the
1.x driver you would always have the frame status appended to your data on a
read. In the 2.x driver this can be toggled, and defaults to not appending
the status to the data.

For more information on the ioctl's to toggle this feature see section X.

Changing the clock frequency is basically the same but the data structure
and ioctl name are different.

Change the following ioctl to the current ioctl `FSCC_SET_CLOCK_BITS` (see
section VIII).

`FSCC_SET_FREQ` (setting the clock frequency)

In the 1.x driver you passed in a structure composed of both the desired
frequency and the clock bits that represent the frequency. In the 2.x driver
this has been simplified down to just the clock bits.

### FAQ
Q: Why are the /dev/fscc* ports not created even though the driver has
   loaded?

A: There are a couple of possibilities but you should first check
   /var/log/messages for any helpful information. If that doesn't help you
   out then continue reading.

   One possibility is that there is another driver loaded that has claimed
   our cards. For example if your kernel is patched to use our card for
   asynchronous transmission the linux serial driver has more than likely
   claimed the card and in turn we won't receive the appropriate 'probe'
   notifications to load our card.

   Another possibility is that you have accidentally tried insmod'ing with
   the 'hot_plug' option enabled and your cards are not actually present.
   Double check that your card shows up in the output of 'lspci' and make
   sure to use hot_plug=0.

Q: What does poll() and select() base their information on?

A: Whether or not you can read data will be based on if there is at least 1
   byte of data available to be read in your current mode of operation. For
   example, if there is streaming data it will not be considered when in
   a frame based mode.

   Whether or not you can write data will be based on if you have hit your
   output memory cap. (see section XI).

Q: Why does executing a purge without a clock put the card in a broken
   state?

A: When executing a purge on either the transmitter or receiver there is
   a TRES or RRES (command from the CMDR register) happening behind the
   scenes. If there is no clock available the command will stall until
   a clock is available. This should work in theory but doesn't in
   practice. So whenever you need to execute a purge without a clock, first
   put it into clock mode 7, execute your purge then return to your other
   clock mode.

Q: Why am I receiving the error message 'Couldn't register serial port'
   when loading the driver.

A: When loading, the driver will attempt to register the board's UARTs with
   the built in serial driver. The serial driver statically defines how
   many UARTs can be registered and will report this error if there isn't
   enough room.

   There are multiple ways of allowing more available UART room which can
   be found in section VII.

Q: Why am I not seeing my card in sysfs?

A: There are a couple possibilities but you should first check what kernel
   version you are using. Due to the way we register our card with sysfs it
   won't appear in kernel versions prior to 2.6.25.

   Another possibility is that it is located in a different directory than
   you are checking. Typically fscc/ appears in /sys/class/ but it might
   appear elsewhere. If it isn't in /sys/class/ do a search in /sys/ for
   our fscc/ directory.