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
 * slice.h
 *
 *
 *  @author Daniel Depaoli <daniel.depaoli (at) create-net.org>
 */

#ifndef XDPDCLASS_H_
#define XDPDCLASS_H_

#include "../openflow/openflow_switch.h"
using namespace std;


//namespace xdpd {

class slice{
public:

	std::string dp_name ;
	uint64_t dp_id;
	caddress address;
	std::string name;
	std::vector<std::string> ports_list;

	std::string ip_string;
	int port_int;

	cofctl* controller; //Instance of controller related to slice

	// Slice id set by virtual agent
	uint32_t slice_id;

	virtual ~slice();

	//slice(std::string dp_name, uint64_t dp_id, std::string name, caddress address, std::vector<std::string> ports_list, cofctl* controller=NULL);
	slice(std::string dp_name, uint64_t dp_id, std::string name, int port, std::string ip, std::vector<std::string> ports_list, cofctl* controller=NULL);

	slice(caddress address);

	uint64_t getDpId() const {
		return dp_id;
	}

	const std::string& getDpName() const {
		return dp_name;
	}

	bool has_port(std::string port_name);

};

//}
#endif /* XDPDCLASS_H_ */
