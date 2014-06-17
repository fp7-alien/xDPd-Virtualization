MAINTAINERCLEANFILES = Makefile.in


noinst_LTLIBRARIES = libvirtual_agent.la

libvirtual_agent_la_SOURCES = \
    virtualizationgateway.cc \
    virtualizationgateway.h \
	virtualagent.cc \
	virtualagent.h \
	slice.h \
	slice.cc \
	flowspace.h \
	va_switch.cc \
	va_switch.h

libvirtual_agent_la_LIBADD = \
	../management/libxdpd_mgmt.la \
	../openflow/libxdpd_openflow.la \
	../openflow/openflow12/libxdpd_openflow12.la \
	-ljsonrpc \
	-lpthread
