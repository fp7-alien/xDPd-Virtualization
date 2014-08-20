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
 * slice.cpp
 *
 *
 *  @author Daniel Depaoli <daniel.depaoli (at) create-net.org>
 */

#include "slice.h"
#include <iomanip>
#include "virtualagent.h"
#include "flowspace.h"


//slice::slice(std::string dp_name, uint64_t dp_id, std::string name, caddress address, std::vector<std::string> ports_list, cofctl* controller) {
//
//	this->dp_id = dp_id;
//	this->dp_name = dp_name;
//	this->address = address;
//	this->name = name;
//	this->ports_list = ports_list;
//	this->controller = controller;
//
//}

//slice::slice(std::string dp_name, uint64_t dp_id, std::string name, int port, std::string ip, std::vector<std::string> ports_list, cofctl* controller=NULL)
//{
//	this->dp_id = dp_id;
//	this->dp_name = dp_name;
//	this->address = caddress(AF_INET, ip.c_str(), port);
//	this->port_int = port;
//	this->ip_string = ip;
//	this->name = name;
//	this->ports_list = ports_list;
//	this->controller = controller;
//}

slice::~slice() {

}

bool slice::has_port(std::string port_name) {

	// If port list is empty it means that the slice has all port avaible
	if (this->ports_list.empty())
	{
		return true;
	}
	else
	{
		std::string port_temp;
		for (std::vector<std::string>::iterator iter = this->ports_list.begin();
				iter != this->ports_list.end();
				iter++)
		{
			port_temp = *iter;
			if (strcmp(port_temp.c_str(), port_name.c_str()) == 0)
			{
				return true;
			}
		}
		return false;
	}
}

slice::slice(std::string dp_name, uint64_t dp_id, std::string name, int port,
		std::string ip, std::vector<std::string> ports_list,
		cofctl* controller) {
		this->dp_id = dp_id;
		this->dp_name = dp_name;
		this->address = caddress(AF_INET, ip.c_str(), port);
		this->port_int = port;
		this->ip_string = ip;
		this->name = name;
		this->ports_list = ports_list;
		this->controller = controller;
}

slice::slice(caddress address) {
}
