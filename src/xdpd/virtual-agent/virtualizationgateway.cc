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

	//Validate switches and ports
	if ( !check_datapaths(datapaths, name) )
		return -1;

	Json::Value::Members dpMemebers = datapaths.getMemberNames();
	for (unsigned int i=0; i< dpMemebers.size(); i++)
	{
		//Get datapath
		std::string dpName = dpMemebers[i];
		openflow_switch* sw = switch_manager::find_by_name(dpName);

		//Get ports list
		Json::Value json_port_list = datapaths[dpName];

		caddress address = caddress(AF_INET, ip.c_str(), port);
		std::vector<std::string> ports_list;
		for (unsigned int i = 0; i<json_port_list.size(); i++)
		{
			ports_list.push_back(json_port_list[i].asString());

		}

		try {
			slice* slice_to_add = new slice(dpName, switch_manager::dpid_from_name(sw->dpname),name, address,ports_list);
			if ( virtual_agent::add_slice(slice_to_add, true) )
				switch_manager::rpc_connect_to_ctl(switch_manager::dpid_from_name(sw->dpname),address);
			printf("Slice %s added\n", slice_to_add->name.c_str());
		} catch (eSliceExist) {
			printf("Slice already exist\n");
		}
		catch(eSliceConfigError)
		{
			printf("Slice error\n");
		}

	}

	return 0;
}

int virtualization_gateway::deleteSlice(const std::string& name) {

	std::list<std::string> datapath_list = switch_manager::list_sw_names();
	for (std::list<std::string>::iterator it = datapath_list.begin();
			it != datapath_list.end();
			it++)
	{
		openflow_switch* sw = switch_manager::find_by_name(*it);
		if (virtual_agent::check_slice_existance(name,sw->dpname))
		{
			slice* slice_to_remove = virtual_agent::list_switch_by_name[sw->dpname]->get_slice(name);
			sw->rpc_disconnect_from_ctl(slice_to_remove->address);
			std::string slice_name = slice_to_remove->name;
			virtual_agent::list_switch_by_name[sw->dpname]->slice_list.remove(slice_to_remove);
			printf("Slice %s removed from %s\n",slice_name.c_str(), sw->dpname.c_str());
		}
		else
			printf("Slice %s not present\n", name.c_str());
	}

	return 0;
}

Json::Value virtualization_gateway::listDatapaths() {

	std::list<std::string> datapath_list = switch_manager::list_sw_names();
	Json::Value return_value;
	for (std::list<std::string>::iterator it = datapath_list.begin();
			it != datapath_list.end();
			it++)
	{
		return_value.append(*it);
	}
	return return_value;
}

Json::Value virtualization_gateway::listFlowspaceInfo(const std::string& name) {

	Json::Value return_value;
	return return_value;
}

Json::Value virtualization_gateway::listFlowspaces() {

	Json::Value flowspace_list;
	std::list<std::string> datapath_list = switch_manager::list_sw_names();
	for (std::list<std::string>::iterator it = datapath_list.begin();
			it != datapath_list.end();
			it++)
	{
		//va_switch* vaswitch = virtual_agent::list_switch_by_name[*it];
		for (std::list<flowspace_struct_t*>::iterator fs_it = virtual_agent::list_switch_by_name[*it]->flowspace_struct_list.begin();
				fs_it != virtual_agent::list_switch_by_name[*it]->flowspace_struct_list.end();
				fs_it++)
		 {
			flowspace_struct_t* temp = *fs_it;
			 std::string fs_name = temp->name;
			 flowspace_list[*it].append(fs_name);
		 }
	}
return flowspace_list;
}

Json::Value virtualization_gateway::listSlice() {

	Json::Value slice_list;
	std::list<std::string> datapath_list = switch_manager::list_sw_names();
	for (std::list<std::string>::iterator it = datapath_list.begin();
			it != datapath_list.end();
			it++)
	{
		std::cout << "switch " << *it->c_str() << " ha " << virtual_agent::list_switch_by_name[*it]->slice_list.size() << " slices\n";
		for (std::list<slice*>::iterator sl_it = virtual_agent::list_switch_by_name[*it]->slice_list.begin();
				sl_it != virtual_agent::list_switch_by_name[*it]->slice_list.end();
				sl_it++)
		 {
			slice* temp = *sl_it;
			std::string sl_name = temp->name;
			slice_list[*it].append(sl_name);
		 }
	}
return slice_list;
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


// Return true only if all datapaths and ports exist
bool virtualization_gateway::check_datapaths(Json::Value datapaths, std::string name) {

	Json::Value::Members dpMemebers = datapaths.getMemberNames();
	for (unsigned int i=0; i< dpMemebers.size(); i++)
	{
		std::string dpName = dpMemebers[i];
		openflow_switch* sw = switch_manager::find_by_name(dpName);

		//Validate slice name in the datapath
		if (virtual_agent::check_slice_existance(name, sw->dpname))
		{
			printf("Slice \"%s\" already present\n",name.c_str());
			return false;
		}

		if (!sw)
		{
			printf("Switch %s not found\n", dpName.c_str());
			return false;
		}

		//validate port
		Json::Value json_port_list = datapaths[dpName];
		for (unsigned int i = 0; i<json_port_list.size(); i++)
		{
			if (!sw->port_is_present(json_port_list[i].asString()))
			{
				printf("Port %s not present\n", json_port_list[i].asString().c_str());
				return false;
			}
			else
			{
				printf("Port %s present\n", json_port_list[i].asString().c_str());
			}
		}
	}
	return true;
}
