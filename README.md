xDPd-Virtualization
===================
Module that adds virtualization capabilities to xDPd.

This is a reference for xdpd_virtualization. Before this you must read the *README* of rofl-core and xdpd.

IMPORTANT NOTES
===============
This revision is based on rofl-core and xdpd master-0.3
````
git clone https://github.com/bisdn/rofl-core.git
cd rofl-core
git checkout master-0.3
````

Building
========
Build and install rofl-core

For xDPd-Virtualization just follow xdpd guide.

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
Two examples are avaible in folder *ConfigFile*.
In the examples there are 2 tap port used to connect virtual-machine with xdpd

- *2switch.cfg*: two datapaths connected each other. 3 slices and 3 flowspaces with no topology change
- *4switch-topology.cfg*: 4 datapaths. 3 slices with different topology. 3 flowspaces

In the config file there are the comments that explain the different parameters.