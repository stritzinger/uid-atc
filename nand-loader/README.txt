BR UID
======

== Git Repositories

-------------------------------------------------------
git clone zbfxigop@ftp.rtems.eu:rtems.git git-rtems
git clone zbfxigop@ftp.rtems.eu:yaffs2.git git-yaffs2
git clone zbfxigop@ftp.rtems.eu:bed.git git-bed
git clone zbfxigop@ftp.rtems.eu:rtems-apps.git git-apps
-------------------------------------------------------

== RTEMS Installation

---------------------------------------------------------
cd ${ROOT}/git-rtems
git pull
./bootstrap
rm -rf ${ROOT}/b-rtems
cd ${ROOT}/b-rtems
${ROOT}/git-rtems/configure \
	--target=powerpc-rtems4.11 \
	--prefix=/opt/rtems-4.11 \
	--enable-rtemsbsp=br_uid \
	--enable-posix \
	--enable-maintainer-mode
make
make install
---------------------------------------------------------

== Library Installation

---------------------------------------------------------
cd ${ROOT}/git-yaffs2
git pull
make clean install

cd ${ROOT}/git-bed
git pull
make clean install

cd ${ROOT}/git-apps
git pull

cd ${ROOT}/git-apps/libdemo
make clean install

cd ${ROOT}/git-apps/quicc
make clean
make
# The make terminates with an error while building the tests. This error can be
# ignored for the installation.
make install

cd ${ROOT}/git-apps/libini
make clean install

cd ${ROOT}/git-apps/libuid
make clean install
---------------------------------------------------------

After the libraries are installed, the nand-loader can be build with a simple
+make+.

== Flash Partitions

.Flash Partitions
[cols=">,>,>,3<",frame="topbot",options="header"]
|======================================================================================
| Start Block | Block Count | Size   | Usage
|           0 |          32 |   4MiB | Stage-1 program (sequential with bad block skip)
|          32 |        1984 | 248MiB | Flash file system (YAFFS2)
|        1984 |          32 |   4MiB | Reserved
|======================================================================================

== Stage-1 (NAND Boot Loader)

The stage-1 boot loader is started by the MPC8309 boot sequencer provided a
valid RCW is stored in the first 4KiB of a NAND flash block.  The initial start
code must fit into a 4KiB area.  This initial start code initializes the DDR
RAM, relocates itself to the RAM and loads the remaining content of its
program.  After the basic initialization RTEMS is started.

The high-level stage-1 part will try to mount the flash file system.  In case
this is successful it tries to read the +/ffs/br_uid.ini+ configuration file.

--------------------------------------------------
[boot]
timeout_in_seconds = 3

[file]
image_path = /ffs/br_uid.bin

[network]
mac_address = 0e:b0:ba:5e:ba:11
ip_self = 192.168.100.50
ip_gateway = 192.168.100.254
ip_netmask = 255.255.255.0

[nfs]
server_path = 1000.100@192.168.96.64:/scratch/nfs
image_path = /nfs/br_uid.bin
--------------------------------------------------

All values are optional and in case something is missing default values will be
used (presented in the listing above).  Once the timeout expired without user
input the automatic application load sequence starts.  In case a valid _file_
section is present in the configuration file this image will be started.  In
case a valid _nfs_ section is present in the configuration file this image will
be started.  If both sections are present, then the image of the _file_ section
has priority.

=== Recovering from bad settings

If there are any problems with the settings (eg. timeout is set to 0) so that
there is no possibility to get into the bootloader, it is possible to reset to
the default settings by loading and executing the stage-1-loader application
into the RAM via a debugger. It will restore defaults and exit. After a reboot,
the bootloader should show up again.

=== Update stage-1 via shell command

It is possible to update the stage-1 via a shell command, for example:

--------------------------------------------------
[/] # stage_1_update /ffs/stage-1.bin
--------------------------------------------------

Using invalid binary images will place the board into a bad state.  There are
no checks to prevent user errors.
