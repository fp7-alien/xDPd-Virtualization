xDPd-Virtualization
===================
xDPd-Virtualization is a modified version of xDPd (Extended datapath daemon) with a virtualization module (Virtualization Agent).
Like other virtualization approaches (FlowVisor and VeRTIGO), the VA’s main objective is to allow 
multiple parallel experiments to be executed on the same physical substrate without interfering each other. The VA has 
been designed with the following goals: 
* avoid Single Point of Failures (SPoF) through a distributed slicing architecture, 
* provide an OpenFlow version agnostic slicing mechanism and 
* minimize the latency overhead caused by the slicing operations. 

Main characteristics of this virtualization technique are:
* __Distributed slicing__: virtualization operations are performed directly on the nodes
* __Protocol agnostic__: it does not inspect the control protocol
* __Latency overhead__: neither inspects the OpenFlow protocol nor needs to establish additional TLS connections

IMPORTANT NOTES
===============
This software is based on rofl-core (<https://github.com/bisdn/rofl-core>) and xdpd (<https://github.com/bisdn/xdpd>).

The version of both is 0.3

Building
========
This revision is based on rofl-core and xdpd master-0.3, so install rofl-core 0.3.

This software uses libjson-rpc-cpp (<https://github.com/cinemast/libjson-rpc-cpp>). Compilie and install it before compiling xDPd-Virtualization

````
git clone https://github.com/fp7-alien/xDPd-Virtualization.git
cd xDPd-Virtualization
checkout v0.3.1
sh autogen.sh
cd build
../configure
make
````

Using
=====
With virtualization-agent active
````
sudo ./xdpd -c configFileVirtualization.cfg
````

Without virtualization-agent. 
Note that in this case all configuration regarding virtualization-agent will be ignored.
````
sudo ./xdpd -n -c configFileVirtualization.cfg
````

Config File
===========
The section _config_ is the same and unchanged respect to xdpd. The only differences is that in this section if virtualization agent is active all
controllers settings will be ignored.

In the section _virtual-agent_ there are the general parameters for Virtualization Agent, as the virtual agent ip and port.

_Slice_ section includes all slices with theri name, ip:port and a list of datapath where the slice will be installed. Each datapath is an array of ports. If no ports are specified, the Virtual Agent includes all ports of that switch.

_Flowspace_ section contains the list of flowspace rules (_at the moment only vlan\_vid is valid_). The number of priority means: bigger the number, less rule importance.

Two examples are avaible in folder *ConfigFile*.
In the examples there are 2 tap port used to connect virtual-machine with xdpd

- *2switch.cfg*: two datapaths connected each other. 3 slices and 3 flowspaces with no topology change
- *4switch-topology.cfg*: 4 datapaths. 3 slices with different topology. 3 flowspaces

In the config file there are the comments that explain the different parameters.
