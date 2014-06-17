//The ALIEN project (work funded by European Commission).
//
//Copyright (C) 2014 Create-Net (www.create-net.org)
//
//Authors:
//Daniel Depaoli <daniel.depaoli (at) create-net.org>
//
//This Source Code Form is subject to the terms of the Mozilla Public
//License, v. 2.0. If a copy of the MPL was not distributed with this
//file, You can obtain one at http://mozilla.org/MPL/2.0/.


/*
 * virtualgateway.cpp
 *
 *  @author Daniel Depaoli <daniel.depaoli (at) create-net.org>
 */

#include "virtualizationgateway.h"
#include <rofl/common/csocket.h>
#include "../openflow/openflow_switch.h"

/**
 *
 * UNUSED
 * TO IMPLEMENT
 */

namespace rofl {
virtualization_gateway::virtualization_gateway(caddress addr):
				socket(new csocket(this, PF_INET, SOCK_STREAM, IPPROTO_TCP, 10)),
				cli_fd(-1),
				baddr(addr)
{
	printf("Listen to virtual gateway\n");
	socket->clisten(baddr, PF_INET, SOCK_STREAM, IPPROTO_TCP);
}


void virtualization_gateway::handle_accepted(csocket* socket, int newsd,
		const caddress& ra) {
	//int fd = socket->getfd();
	char buffer[256];
	bzero(buffer, 256);
	int n = read(newsd, buffer, 255);
	printf("Message: %s %i\n", buffer, n);
	//socket->cclose();
	cmemory* msg =new  cmemory(&n,sizeof(buffer));
	//uint8_t
	//memccpy(msg,buffer,)
	//csocket* new_sock = new csocket(this, PF_INET, SOCK_STREAM, IPPROTO_TCP, 10);
	//caddress new_address(AF_INET, "127.0.0.1", 6601);
	//new_sock->cconnect(new_address,)
	//new_sock->send_packet(msg,new_address);
	//socket->cconnect(socket->raddr,PF_INET,SOCK_STREAM, IPPROTO_TCP);
	socket->send_packet(msg, socket->raddr);
	printf("messaggio: inviato\n");
}

void virtualization_gateway::handle_connected(csocket* socket, int sd) {
	printf("Connected\n");
}

void virtualization_gateway::handle_connect_refused(csocket* socket, int sd) {
	printf("Reject\n");
}

void virtualization_gateway::handle_read(csocket* socket, int sd) {
	printf("read\n");
}

void virtualization_gateway::handle_closed(csocket* socket, int sd) {
	printf("close\n");
}

void virtualization_gateway::handle_revent(int fd) {
	printf("handle_revent");
}


}


