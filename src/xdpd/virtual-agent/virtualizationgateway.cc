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

void *try_thread( void *ptr )
{
	slice* slice_to_add = (slice*) ptr;

	cofctl* controller = NULL;
	//controller = sw->rpc_connect_to_ctl_return(address, controller);

	controller = switch_manager::find_by_dpid(slice_to_add->dp_id)->rpc_connect_to_ctl_return(slice_to_add->address, NULL);//switch_manager::rpc_connect_to_ctl(slice_to_add->dp_id,slice_to_add->address);
	virtual_agent::add_slice(slice_to_add,true);
	printf("%s\n", controller->c_str());
	ciosrv::run();
	return NULL;
}

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
			ROFL_ERR("datapath %s doesn't exist\n",listDatapaths[i].asString().c_str());
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
		if (virtual_agent::check_flowspace_existance(name,NULL,&switch_name) )
			return -4;

		if (!virtual_agent::check_slice_existance(slice, switch_name))
			return -4;


		ROFL_DEBUG("adding to datapath %s flowspace %s with destination slice %s\n",
					sw->dpname.c_str(), name.c_str(), slice.c_str());

		//validate flowspace_match
		if (!matches["vlan_vid"] || !matches["vlan_vid"].isInt())
		{
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

		virtual_agent::list_switch_by_name[switch_name]->flowspace_struct_list.push_front(flowspaceStruct);
	}


	return 0;
}

/**
 * returning value:
 * -1: datapaths not existing
 * -2: slice already exists
 * -3: max number of slice reaches
 */
int virtualization_gateway::addSlice(const Json::Value& datapaths,
		const std::string& ip, const std::string& name,
		const std::string& ofversion, const int& port) {
std::cout << "creazione slice\n";
	//Validate switches and ports
	if ( !check_datapaths(datapaths, name) && !datapaths.empty() )
		return -1;
	else if (datapaths.empty())
	{
		//All datapaths with ofversion given
		std::list<va_switch*> datapaths_list = virtual_agent::get_all_va_switch( (ofversion=="1.0")?OF_VERSION_10:OF_VERSION_12 );
		std::cout << "dimensione lista " << datapaths_list.size() << "\n";
		for (std::list<va_switch*>::iterator it = datapaths_list.begin();
				it != datapaths_list.end();
				it++)
		{
			vector<std::string> port_list;
			va_switch* temp_sw = *it;
			std::vector<std::string>::iterator port_iter;
			for (port_iter = switch_manager::find_by_name(temp_sw->dp_name)->port_list.begin();
					port_iter != switch_manager::find_by_name(temp_sw->dp_name)->port_list.end();
					port_iter++)
			{
				std::string port = *port_iter;
				port_list.push_back(port);
			}

			try{
				slice* slice_to_add = new slice(temp_sw->dp_name,temp_sw->dp_id, name, port, ip,port_list,NULL );
				virtual_agent::add_slice(slice_to_add, true);
			}
			catch (eSliceExist) {
				ROFL_ERR("Slice already exist\n");
				return -2;
			}
			catch(eSliceConfigError)
			{
				ROFL_ERR("Slice error\n");
				return -3;
			}
		}
	}

	else{
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

/*		try {
		//	slice* slice_to_add = new slice(dpName, switch_manager::dpid_from_name(sw->dpname),name, address,ports_list);
		//	if ( virtual_agent::add_slice(slice_to_add, true) )
		//	{
				//cofctl* controller = NULL;
				//controller = sw->rpc_connect_to_ctl_return(address, controller);
				//sw->rpc_connect_to_ctl(address, controller);
				//if (!controller)
				//	printf("Controller null!\n");
				//else
				//	printf("Controller OK!\n");
				slice* slice_to_add = new slice(dpName, switch_manager::dpid_from_name(sw->dpname),name, port, ip.c_str(),ports_list,NULL);
				//virtual_agent::add_slice(slice_to_add, true);
				pthread_t thread1;
				int p1;
				//const char *message1 = "Thread 1";
				p1 = pthread_create( &thread1, NULL, try_thread, (void*) slice_to_add);
				if (p1)
					printf("Errore\n");

			//}
			ROFL_DEBUG("Slice %s added\n", slice_to_add->name.c_str());
		}*/
		try{
			slice* slice_to_add = new slice(dpName, switch_manager::dpid_from_name(sw->dpname),name, port, ip.c_str(),ports_list,NULL);
			virtual_agent::add_slice(slice_to_add, true);
		}
		catch (eSliceExist) {
			ROFL_ERR("Slice already exist\n");
			return -2;
		}
		catch(eSliceConfigError)
		{
			ROFL_ERR("Slice error\n");
			return -3;
		}

	}
	}

	virtual_agent::write_cfg("real-time-config.cfg");
	return 0;
}

/**
 * returning value:
 * -1 slice not exist
 */
int virtualization_gateway::deleteSlice(const std::string& name) {

	int count = 0;
	std::list<std::string> datapath_list = switch_manager::list_sw_names();
	for (std::list<std::string>::iterator it = datapath_list.begin();
			it != datapath_list.end();
			it++)
	{
		openflow_switch* sw = switch_manager::find_by_name(*it);
		if (virtual_agent::check_slice_existance(name,sw->dpname))
		{
			count++;
			slice* slice_to_remove = virtual_agent::list_switch_by_name[sw->dpname]->get_slice(name);
			sw->rpc_disconnect_from_ctl(slice_to_remove->address);
			std::string slice_name = slice_to_remove->name;
			virtual_agent::list_switch_by_name[sw->dpname]->slice_list.remove(slice_to_remove);
			ROFL_DEBUG("Slice %s removed from %s\n",slice_name.c_str(), sw->dpname.c_str());
		}
		else
			ROFL_ERR("Slice %s not present\n", name.c_str());
	}

	if (count ==0)
		return -1;
	else
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

	Json::Value slice_info;

	Json::Value dp_js;
	Json::Value rules_js;
	slice* slice = NULL;
	bool ip_port = false;
	std::list<std::string> datapath_list = switch_manager::list_sw_names();
	for (std::list<std::string>::iterator it = datapath_list.begin();
			it != datapath_list.end();
			it++)
	{
		slice = virtual_agent::check_slice_existance(name, *it);
		if (slice)
		{
			dp_js.append(*it);
			if (!ip_port)
			{
				slice_info["address"] = slice->address.c_str();
				ip_port = true;
			}
		}
	}

	if (!dp_js.empty())
		slice_info["datapath"] = dp_js;
	return slice_info;
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
			ROFL_DEBUG("Slice \"%s\" already present\n",name.c_str());
			return false;
		}

		if (!sw)
		{
			ROFL_ERR("Switch %s not found\n", dpName.c_str());
			return false;
		}

		//validate port
		Json::Value json_port_list = datapaths[dpName];
		for (unsigned int i = 0; i<json_port_list.size(); i++)
		{
			if (!sw->port_is_present(json_port_list[i].asString()))
			{
				ROFL_ERR("Port %s not present\n", json_port_list[i].asString().c_str());
				return false;
			}
			else
			{
				ROFL_DEBUG("Port %s present\n", json_port_list[i].asString().c_str());
			}
		}
	}
	return true;
}
