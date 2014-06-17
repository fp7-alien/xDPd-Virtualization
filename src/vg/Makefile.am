MAINTAINERCLEANFILES = Makefile.in

#Virtual Gateway client
sbin_PROGRAMS = vg
vg_SOURCES = vg.cc
vg_LDADD = \
    -ljsonrpc \
    -lpthread

AM_LDFLAGS = -static