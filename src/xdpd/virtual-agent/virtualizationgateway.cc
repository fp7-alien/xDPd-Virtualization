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
;
	//Check valid format value
	if (!listDatapaths || !listDatapaths.isArray())
	{
		return -1;
	}

	if (!matches)
	{
		return -1;
	}

	if ( !(priority < 1000 && priority > 0) )
	{
		return -2;
	}

	std::cout << listDatapaths << "\n";
	std::cout << matches << "\n";

	//check if all datapaths exist
	for (unsigned i=0; i<listDatapaths.size(); i++)
	{
		if (!switch_manager::find_by_name(listDatapaths[i].asString()))
		{
			printf("il datapath %s non esiste\n",listDatapaths[i].asString().c_str());
			return -2;
		}
	}

	//All datapaths exist, so it is possible to continue
	for (unsigned i=0; i<listDatapaths.size(); i++)
	{
		std::string switch_name = listDatapaths[i].asString();
		openflow_switch* sw = switch_manager::find_by_name(switch_name);
		if (!sw)
			return -2;

		//validate flowspace and slice name.
		if (!virtual_agent::check_flowspace_existance(name,NULL,&switch_name) )
		{
			printf("%s valido ",switch_name.c_str());
		}
		else
		{
			return -4;
		}

		if (virtual_agent::check_slice_existance(slice, switch_name))
		{
			printf("%s valido ",slice.c_str());
		}
		else
		{
			return -4;
		}

		printf("Posso aggiungere al datapath %s il flowspace %s con destinazione slice %s\n",
					sw->dpname.c_str(), name.c_str(), slice.c_str());

		//validate flowspace_match
		if (!matches["vlan_vid"] || !matches["vlan_vid"].isInt())
		{
			printf("vlan vid non presente o sbagliato\n");
			return -5;
		}
		flowspace_struct_t* flowspaceStruct = new flowspace_struct_t;
		flowspaceStruct->name = name;
		flowspaceStruct->priority = priority;
		flowspaceStruct->slice = slice;

		flowspace_match_t* match_vlanvid = new flowspace_match_t;
		utern_t* uternvalue_vlanvid = new utern_t;
		//Create the match
		match_vlanvid->type = FLOWSPACE_MATCH_VLAN_VID;
		uternvalue_vlanvid->value.u16 = matches["vlan_vid"].asInt();
		uternvalue_vlanvid->type = UTERN16_T;
		uternvalue_vlanvid->mask.u16 = ~0;
		match_vlanvid->value = uternvalue_vlanvid;

		//Update the match in the list
		flowspaceStruct->match_list.push_front(match_vlanvid);

		virtual_agent::list_switch_by_name["dp1"]->flowspace_struct_list.push_front(flowspaceStruct);
	}


	return 0;
}

int virtualization_gateway::addSlice(const Json::Value& datapaths,
		const std::string& ip, const std::string& name, const int& port) {

	//std::string datapath_name="dp1";
	//std::string slice_name="daniel";
	//std::string slice_ip = "10.0.0.4";
	//int slice_port = 6611;

	//Validate switch name
	Json::Value::Members dpMemebers = datapaths.getMemberNames();
	for (unsigned int i=0; i< dpMemebers.size(); i++)
	{
		std::string dpName = dpMemebers[i];
		openflow_switch* sw = switch_manager::find_by_name(dpName);
		if (!sw)
		{
			printf("Switch %s not found\n", dpName.c_str());
			return -1;
		}

		//validate port
		Json::Value json_port_list = datapaths[dpName];
		for (unsigned int i = 0; i<json_port_list.size(); i++)
		{
			if (!sw->port_is_present(json_port_list[i].asString()))
			{
				printf("Port %s not present\n", json_port_list[i].asString().c_str());
			}
			else
			{
				printf("Port %s present\n", json_port_list[i].asString().c_str());
			}
		}


		//Slice name
		if (virtual_agent::check_slice_existance(name, sw->dpname))
			return -2;

		caddress address = caddress(AF_INET, ip.c_str(), port);
		std::vector<std::string> ports_list;
		for (unsigned int i = 0; i<json_port_list.size(); i++)
		{
			ports_list.push_back(json_port_list[i].asString());

		}

		slice* slice_to_add = new slice(sw->dpname, switch_manager::dpid_from_name(sw->dpname),name, address,ports_list);
		//switch_manager::find_by_name("dp1")->rpc_connect_to_ctl(address);
		printf("controller 1\n");
		switch_manager::rpc_connect_to_ctl(switch_manager::dpid_from_name(sw->dpname),address);
		printf("controller 2\n");
		switch_manager::rpc_connect_to_ctl(switch_manager::dpid_from_name(sw->dpname),caddress(AF_INET, "10.0.0.4", 6655));
		virtual_agent::add_slice(slice_to_add, true);
	}

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

int virtualization_gateway::parse_priority(int priority) {
	if (priority < 0 && priority > 1000)
		return -1;

	return 0;
}
