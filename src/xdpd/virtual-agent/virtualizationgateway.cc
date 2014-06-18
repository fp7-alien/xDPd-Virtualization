/*
 * virtualizationgateway.cpp
 *
 *  Created on: Jun 16, 2014
 *      Author: daniel
 */

#include "virtualizationgateway.h"
#include "jsonRpc/abstractstubserver.h"
#include <pthread.h>
#include <iostream>
#include <stdio.h>
#include <jsonrpc/rpc.h>
#include <sstream>
#include "virtualagent.h"
#include "../management/switch_manager.h"

#include <rofl/common/crofbase.h>

virtualization_gateway::virtualization_gateway(int port):
	AbstractstubServer(new jsonrpc::HttpServer(port)){
	this->StartListening();
}


virtualization_gateway::~virtualization_gateway() {
	// TODO Auto-generated destructor stub
	this->StopListening();
}

int virtualization_gateway::addFlowspace(const Json::Value& listDatapaths,
		const Json::Value& matches, const std::string& name,
		const int& priority, const std::string& slice) {

	flowspace_struct_t* flowspaceStruct = new flowspace_struct_t;
	flowspaceStruct->name = "fsDaniel";
	flowspaceStruct->priority = 10;
	flowspaceStruct->slice = "daniel";

	flowspace_match_t* match_vlanvid = new flowspace_match_t;
	utern_t* uternvalue_vlanvid = new utern_t;
	//Create the match
	match_vlanvid->type = FLOWSPACE_MATCH_VLAN_VID;
	//int vlan = 10;
	uternvalue_vlanvid->value.u16 = 0;
	uternvalue_vlanvid->type = UTERN16_T;
	uternvalue_vlanvid->mask.u16 = ~0;
	match_vlanvid->value = uternvalue_vlanvid;

	//Update the match in the list
	flowspaceStruct->match_list.push_front(match_vlanvid);

	virtual_agent::list_switch_by_name["dp1"]->flowspace_struct_list.push_front(flowspaceStruct);
	return 0;
}

int virtualization_gateway::addSlice(const Json::Value& datapaths,
		const std::string& ip, const std::string& name, const int& port) {


	caddress address = caddress(AF_INET, "127.0.0.1", 6611);
	std::vector<std::string>::iterator port_iter;
	std::vector<std::string> ports_list;
	for (port_iter = switch_manager::find_by_name("dp1")->port_list.begin();
			port_iter != switch_manager::find_by_name("dp1")->port_list.end();
			port_iter++)
	{
		std::string port = *port_iter;
		ports_list.push_back(port);
	}

	slice* slice_to_add = new slice("dp1", switch_manager::dpid_from_name("dp1"),"daniel", address,ports_list);
	//switch_manager::find_by_name("dp1")->rpc_connect_to_ctl(address);
	switch_manager::rpc_connect_to_ctl(switch_manager::dpid_from_name("dp1"),address);
	virtual_agent::add_slice(slice_to_add, true);

	return 0;
}

int virtualization_gateway::deleteSlice(const std::string& name) {

	return 0;
}

Json::Value virtualization_gateway::listDatapaths() {

	Json::Value return_value;
	return return_value;
}

Json::Value virtualization_gateway::listFlowspaceInfo(const std::string& name) {

	Json::Value return_value;
	return return_value;
}

Json::Value virtualization_gateway::listFlowspaces() {

	Json::Value return_value;
	return return_value;
}

Json::Value virtualization_gateway::listSlice() {

	Json::Value return_value;
	return return_value;
}

Json::Value virtualization_gateway::listSliceInfo(const std::string& name) {

	Json::Value return_value;
	return return_value;
}

int virtualization_gateway::removeFlowspace(const std::string& name) {

	return 0;
}

int virtualization_gateway::nothing() {

	return 1;
}
