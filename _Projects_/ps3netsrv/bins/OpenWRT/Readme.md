OpenWRT binary build webMAN-MOD 1.47.03

Here are a RELEASE and a DEBUG build of the most current ps3netsrv source code (1.47.03) for ARMv7 rev 0 (v71).

They have been dinamically compiled, natively on the router (static compilation is only possible when cross-compiling, from what I can tell).

These builds are specifically for routers running ARMv7 architecture (any modern, dual-core CPU router will likely use one of those). 
Here is the output of /proc/cpuinfo from mine:

root@NetGear:~# cat /proc/cpuinfo  
model name      : ARMv7 Processor rev 0 (v7l)  
processor       : 0  
BogoMIPS        : 1594.16  
Features        : half fastmult edsp tls  
CPU implementer : 0x41  
CPU architecture: 7  
CPU variant     : 0x3  
CPU part        : 0xc09  
CPU revision    : 0  

model name      : ARMv7 Processor rev 0 (v7l)  
processor       : 1  
BogoMIPS        : 1594.16  
Features        : half fastmult edsp tls  
CPU implementer : 0x41  
CPU architecture: 7  
CPU variant     : 0x3  
CPU part        : 0xc09  
CPU revision    : 0  

Hardware        : Northstar Prototype  
Revision        : 0000  
Serial          : 0000000000000000  

It was compiled on a NETGEAR R6400 running DD-WRT v.3.0 Linux version 4.4.134, but also tested on an ASUS RT-68U running Asuswrt-Merlin Linux version 2.6.36.4brcmarm (so a much older kernel version)

Since they are dinamically compiled, they rely on the following libraries in order to execute properly, here is the output of ldd on one of them:

root@NetGear:/opt/bin# ldd ps3netsrv_arm  
        linux-vdso.so.1 (0x7e9e2000)  
        libstdc++.so.6 => /opt/lib/libstdc++.so.6 (0x76e18000)  
        libpthread.so.0 => /opt/lib/libpthread.so.0 (0x76def000)  
        libgcc_s.so.1 => /opt/lib/libgcc_s.so.1 (0x76dd2000)  
        libc.so.6 => /opt/lib/libc.so.6 (0x76c90000)  
        libm.so.6 => /opt/lib/libm.so.6 (0x76bcc000)  
        /opt/lib/ld-linux.so.3 (0x76f67000)  

Luckily, to keep things nice and easy, all the required libraries are automatically installed when executing the scripts that install Entware on the router. You can find instructions on how to run these Entware installation scripts here: [DD-WRT](https://wiki.dd-wrt.com/wiki/index.php/Installing_Entware) or [Asuswrt-Merlin](https://github.com/RMerl/asuswrt-merlin/wiki/Entware). It's reasonable to believe that these will work well also on Tomato, AdvancedTomato, OpenWRT, etc. Both of my routers are running the required entware installation and ps3netsrv_arm from the jffs2 partition, so no external drive is involved, everything is on-board (minus the content served, of course).

In order to use it, copy the binary of your choice in the folder "/opt/bin" (make sure once the file is in the destination folder that the permission on it are set to 0755: on WinSCP right click on the file, chose "properties" and set it like that in there; if you prefer using telnet commands, use "chmod +x ps3netsrv_arm" (or "chmod +x ps3netsrv_arm_debug") when in "/opt/bin" folder.

The debug version will display a lot more information when running than the release one. Use the release version if you have no use for said information.
To go together with the binaries there is also a helper file (S99ps3netsrv) that you can place in "/opt/etc/init.d/" to automate the start of the service. Remember to open it first and edit to your specific share path the 5th line (ARGS=). If you are using the debug version ALSO change the 4th line to reflect that (PROCS=).

The usage for this build are:
(null) [rootdirectory] [port] [whitelist]
Default port: 38008
Whitelist: x.x.x.x, where x is 0-255 or *
(e.g 192.168.1.* to allow only connections from 192.168.1.0-192.168.1.255)

When firing it up from a telnet session through command line, it won't be able to start as a daemon service so it will run in the foreground, taking over the session. This is why it is adviced to use the helper file, properly configured with your unique shared path. Once you have that setup you can type "/opt/etc/init.d/S99ps3netsrv start" or "/opt/etc/init.d/S99ps3netsrv stop" or "/opt/etc/init.d/S99ps3netsrv restart" or "/opt/etc/init.d/S99ps3netsrv check" to manage the service. Once the service is started, you can type "top" to have the list of all the running processes, to verify it loaded properly (it will also show you the path you hardcoded in the helper file).

PLEASE NOTE: if you do chose to use the helper file, DO NOT RENAME IT! Files in the "init.d" folder are automatically executed at startup when their name starts with a capital "S", while the  two digits that follow indicate the sequence that should be used when starting them. So you can change the "99" to something less if you need so, but keep the capital S, that's the bottom line.
