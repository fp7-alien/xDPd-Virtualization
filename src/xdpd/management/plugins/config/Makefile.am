MAINTAINERCLEANFILES = Makefile.in

SUBDIRS=interfaces openflow virtual-agent

noinst_LTLIBRARIES = libxdpd_mgmt_config.la

libxdpd_mgmt_config_la_SOURCES = \
	scope.cc \
	root_scope.cc \
	config.cc 

libxdpd_mgmt_config_la_LIBADD = \
	interfaces/libxdpd_mgmt_config_interfaces.la\
	openflow/libxdpd_mgmt_config_openflow.la\
	virtual-agent/libxdpd_mgmt_config_virtual_agent.la\
	-lconfig++
