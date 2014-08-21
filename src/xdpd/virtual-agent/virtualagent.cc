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
 * virtualagent.cpp
 *
 *  @author Daniel Depaoli <daniel.depaoli (at) create-net.org>
 */

#include "virtualagent.h"
//#include <libconfig.h++>


#include "slice.h"
#include <iomanip>
#include "../management/port_manager.h"
#include "../management/switch_manager.h"

#include <sstream>
#include <libconfig.h++>

#include "../openflow/openflow10/of10_translation_utils.h"

#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_flow_entry.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_action.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_match.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_instruction.h>

#define max_group_ID 1048576	//Group id must be smaller (from 20 bit)
#define max_slice_ID 4095		//Slice id must be smaller (from 12 bit)

#define group_ID_mask 0x000FFFFF	//last 20 bit
#define slice_ID_mask 0xFFF00000	//first 12 bit

using namespace xdpd;
using namespace libconfig;

// static initialization
std::map<uint64_t, va_switch*> virtual_agent::list_switch_by_id;
std::map<std::string, va_switch*> virtual_agent::list_switch_by_name;
std::map<std::string, uint32_t> virtual_agent::slice_id_map;
std::list<va_switch*> virtual_agent::list_va_switch;
std::list<slice*> virtual_agent::all_slices_list;
std::list<flowspace_struct_t*>virtual_agent::all_flowspaces_list;
std::list<va_switch*>virtual_agent::all_switches_list;
libconfig::Config virtual_agent::virtual_link_setting;
bool* virtual_agent::virtual_agent::active = NULL;
uint32_t virtual_agent::slice_counter = 0;


virtual_agent::virtual_agent() {
}


virtual_agent::~virtual_agent() {
	// TODO Auto-generated destructor stub
}

bool virtual_agent::add_flowspace(flowspace_struct_t* flowspace_to_add) {
	bool exist = false;
	for (std::list<flowspace_struct_t*>::iterator iter = virtual_agent::all_flowspaces_list.begin();
			iter != virtual_agent::all_flowspaces_list.end();
			iter++)
	{
		flowspace_struct_t* temp_flow = *iter;
		if (temp_flow->name == flowspace_to_add->name)
		{
			exist = true;
			continue;
		}
	}
	if (!exist)
	{
		all_flowspaces_list.push_back(flowspace_to_add);
	}
return true;
}

bool virtual_agent::add_slice(slice* slice_to_add, bool connect) {
	if (check_slice_existance(slice_to_add->name, slice_to_add->dp_name))
	{
		throw eSliceExist();
	}
	if (slice_to_add->ports_list.empty())
	{
		std::vector<std::string> ports_list;
		std::vector<std::string>::iterator port_iter;
		for (port_iter = switch_manager::find_by_name(slice_to_add->dp_name)->port_list.begin();
				port_iter != switch_manager::find_by_name(slice_to_add->dp_name)->port_list.end();
				port_iter++)
		{
			std::string port = *port_iter;
			ports_list.push_back(port);
		}
	}
	uint64_t id = slice_to_add->dp_id;
	std::string dp_name = slice_to_add->dp_name;
	uint32_t slice_ID = virtual_agent::increase_slice_counter(slice_to_add->name);
	if (slice_ID <= 4095)
	{
		slice_to_add->slice_id = slice_ID;
		virtual_agent::list_switch_by_id[id]->slice_list.push_front(slice_to_add);
		//virtual_agent::list_switch_by_name[dp_name]->slice_list.push_front(slice_to_add);
		if (!slice_to_add->controller)
		{
			printf("Aggiungo il controller preso da return_last_ctl()\n");
			virtual_agent::list_switch_by_id[id]->controller_map[slice_to_add->name] = switch_manager::find_by_dpid(id)->getEndpoint()->return_last_ctl();
			printf("%s:%s\n",
					virtual_agent::list_switch_by_id[id]->controller_map[slice_to_add->name]->get_peer_addr().c_str(),
					virtual_agent::list_switch_by_id[id]->controller_map[slice_to_add->name]->c_str());
		}
		else
		{
			printf("Aggiungo il controller passato %s\n", slice_to_add->controller->get_peer_addr().c_str());
			virtual_agent::list_switch_by_id[id]->controller_map[slice_to_add->name] = slice_to_add->controller;
			printf("%s:%s\n",
					virtual_agent::list_switch_by_id[id]->controller_map[slice_to_add->name]->get_peer_addr().c_str(),
					virtual_agent::list_switch_by_id[id]->controller_map[slice_to_add->name]->c_str());
		}

		bool exist = false;
		for (std::list<slice*>::iterator iter = all_slices_list.begin();
				iter != all_slices_list.end();
				iter++)
		{
			slice* iter_slice = *iter;
			if (iter_slice->name==slice_to_add->name)
			{
				exist = true;
				continue;
			}
		}
		if (!exist)
		{
			all_slices_list.push_back(slice_to_add);
		}

	}
	else
	{
		ROFL_ERR("Impossible to add slice %s. Max slice ID reaches\n", slice_to_add->name.c_str());
		throw eSliceConfigError();
	}
	std::cout << "VAswitch " << slice_to_add->dp_name.c_str() << " ha " << virtual_agent::list_switch_by_name[slice_to_add->dp_name.c_str()]->controller_map.size() << " slices\n";

	return true;

}


void virtual_agent::add_switch(va_switch* _switch) {

	try{
		std::string name = _switch->dp_name;
		uint64_t id = _switch->dp_id;
		virtual_agent::list_switch_by_id[id] = _switch;
		virtual_agent::list_switch_by_name[name] = _switch;
		all_switches_list.push_back(_switch);
	}
	catch (...)
	{
		ROFL_ERR("Some errors in virtual_agent::add_switch\n");
		exit(-1);
	}
}

bool virtual_agent::check_slice_existance(std::string slice_name, uint64_t switch_id) {

	slice* temp_slice;
	for (std::list<slice*>::iterator it = virtual_agent::list_switch_by_id[switch_id]->slice_list.begin();
			it != virtual_agent::list_switch_by_id[switch_id]->slice_list.end();
			it++)
	{
		temp_slice = *it;
		if (temp_slice->name == slice_name)
			return true;
	}

	return false;
}

slice* virtual_agent::check_slice_existance(std::string slice_name,
		std::string switch_name) {

	slice* temp_slice;
	for (std::list<slice*>::iterator it = virtual_agent::list_switch_by_name[switch_name]->slice_list.begin();
			it != virtual_agent::list_switch_by_name[switch_name]->slice_list.end();
			it++)
	{
		temp_slice = *it;
		if (temp_slice->name == slice_name)
			return temp_slice;
	}

	return NULL;
}

bool virtual_agent::is_active() {
	if (*virtual_agent::active)
		return true;
	else
		return false;
}

/**
 *
 * Set virtual agent active/deactive only at the beginning
 */
void virtual_agent::active_va(bool value) {
	if (virtual_agent::active == NULL)
	{
		ROFL_INFO("Set Virtual agent %s\n", (value)? "ACTIVE" : "NOT ACTIVE");
		virtual_agent::active = new bool;
		*virtual_agent::active = value;
	}
	else
		ROFL_INFO("Impossible to set virtual agent in runtime. Virtualization is %s\n",(*virtual_agent::active)? "ACTIVE" : "NOT ACTIVE");
}


uint32_t virtual_agent::increase_slice_counter(std::string slice_name) {

	if (virtual_agent::slice_id_map.find(slice_name) != virtual_agent::slice_id_map.end())
	{
		// Slice already exist
		return virtual_agent::slice_id_map[slice_name]; //Return id corresponding to this slice
	}
	else
	{
		// Slice doesn't exist
		virtual_agent::slice_counter += 1;
		virtual_agent::slice_id_map[slice_name] = virtual_agent::slice_counter;
	}

	return virtual_agent::slice_counter;
}

/**
 *
 * uint_32_t is used to store sliceID (first 12 bit)
 * and groupID (last 20 bit)
 */
uint32_t* virtual_agent::change_group_id(uint32_t groupID, uint32_t sliceID) {

	if (groupID <= max_group_ID && sliceID <= max_slice_ID)
	{
		uint32_t *new_int = new uint32_t;
		*new_int = 0;
		sliceID = sliceID << 20;
		*new_int = sliceID^groupID;
		return new_int;
	}

		return NULL;

}

/**
 *
 *
 * @param port number of switch
 * @param ctl controller
 * @param sw switch
 * @return if port of the swuitch is avaible for that slice
 */
bool virtual_agent::port_analysis(uint32_t port, cofctl* ctl, openflow_switch* sw) {
	if (port == 0)
		return true;

	if ( virtual_agent::list_switch_by_id[sw->dpid]->select_slice(ctl)->has_port(sw->num_to_port(port)) )
		return true;
	else
		return false;
}

/**
 *
 * Add action to new_action_group
 * only if virtualization agent permits it
 */
of1x_action_group_t* virtual_agent::action_group_analysis(cofctl* ctl,
		of1x_action_group_t* action_group, openflow_switch* sw, bool lldp) {

	std::string slice_to_send = virtual_agent::list_switch_by_id[sw->dpid]->select_slice(ctl)->name;

	of1x_action_group_t* new_action_group = of1x_init_action_group(NULL);

	of1x_packet_action_t* iter = action_group->head;
	for(iter=action_group->head;iter;iter = iter->next){

		bool add_action = true;

		// Every type must be controlled against other flowspace
		switch (iter->type){
		case OF1X_AT_NO_ACTION:
			break;
		case OF1X_AT_COPY_TTL_IN:
			break;
		case OF1X_AT_POP_VLAN:
			break;
		case OF1X_AT_POP_MPLS:
			break;
		case OF1X_AT_POP_GTP:
			break;
		case OF1X_AT_POP_PPPOE:
			break;
		case OF1X_AT_POP_PBB:
			break;
		case OF1X_AT_PUSH_PBB:
			break;
		case OF1X_AT_PUSH_PPPOE:
			break;
		case OF1X_AT_PUSH_GTP:
			break;
		case OF1X_AT_PUSH_MPLS:
			break;
		case OF1X_AT_PUSH_VLAN:
			break;
		case OF1X_AT_COPY_TTL_OUT:
			break;
		case OF1X_AT_DEC_NW_TTL:
			break;
		case OF1X_AT_DEC_MPLS_TTL:
			break;
		case OF1X_AT_SET_MPLS_TTL:
			break;
		case OF1X_AT_SET_NW_TTL:
			break;
		case OF1X_AT_SET_QUEUE:
			break;
		case OF1X_AT_SET_FIELD_ETH_DST:
			break;
		case OF1X_AT_SET_FIELD_ETH_SRC:
			break;
		case OF1X_AT_SET_FIELD_ETH_TYPE:
			break;
		case OF1X_AT_SET_FIELD_MPLS_LABEL:
			break;
		case OF1X_AT_SET_FIELD_MPLS_TC:
			break;
		case OF1X_AT_SET_FIELD_MPLS_BOS:
			break;
		case OF1X_AT_SET_FIELD_VLAN_VID:
			flowspace_struct_t* tempFlowspace;
			for (std::list<flowspace_struct*>::iterator it = virtual_agent::list_switch_by_id[sw->dpid]->flowspace_struct_list.begin();
						it != virtual_agent::list_switch_by_id[sw->dpid]->flowspace_struct_list.end();
						it++)
			{
				tempFlowspace = *it;
				std::list<flowspace_match_t*>::iterator _match;		// single match
					flowspace_match_t* tempMatch;
					for (_match = tempFlowspace->match_list.begin();
							_match !=tempFlowspace->match_list.end();
							_match ++)
					{
						tempMatch = *_match;
						if (tempMatch->type == FLOWSPACE_MATCH_VLAN_VID && // Flowspace with vlan_vid
								tempMatch->value->value.u16 == iter->field.u16 && // Value of flowspace equal to value of entry
								slice_to_send.compare(tempFlowspace->slice) != 0) // Comparision between slice and flowspace
						{
							add_action = false;
							break;
						}
					}
			}
			break;
		case OF1X_AT_SET_FIELD_VLAN_PCP:
			break;
		case OF1X_AT_SET_FIELD_ARP_OPCODE:
			break;
		case OF1X_AT_SET_FIELD_ARP_SHA:
			break;
		case OF1X_AT_SET_FIELD_ARP_SPA:
			break;
		case OF1X_AT_SET_FIELD_ARP_THA:
			break;
		case OF1X_AT_SET_FIELD_ARP_TPA:
			break;
		case OF1X_AT_SET_FIELD_NW_PROTO:
			break;
		case OF1X_AT_SET_FIELD_NW_SRC:
			break;
		case OF1X_AT_SET_FIELD_NW_DST:
			break;
		case OF1X_AT_SET_FIELD_IP_DSCP:
			break;
		case OF1X_AT_SET_FIELD_IP_ECN:
			break;
		case OF1X_AT_SET_FIELD_IP_PROTO:
			break;
		case OF1X_AT_SET_FIELD_IPV4_SRC:
			break;
		case OF1X_AT_SET_FIELD_IPV4_DST:
			break;
		case OF1X_AT_SET_FIELD_IPV6_SRC:
			break;
		case OF1X_AT_SET_FIELD_IPV6_DST:
			break;
		case OF1X_AT_SET_FIELD_IPV6_FLABEL:
			break;
		case OF1X_AT_SET_FIELD_IPV6_ND_TARGET:
			break;
		case OF1X_AT_SET_FIELD_IPV6_ND_SLL:
			break;
		case OF1X_AT_SET_FIELD_IPV6_ND_TLL:
			break;
		case OF1X_AT_SET_FIELD_IPV6_EXTHDR:
			break;
		case OF1X_AT_SET_FIELD_TCP_SRC:
			break;
		case OF1X_AT_SET_FIELD_TCP_DST:
			break;
		case OF1X_AT_SET_FIELD_UDP_SRC:
			break;
		case OF1X_AT_SET_FIELD_UDP_DST:
			break;
		case OF1X_AT_SET_FIELD_SCTP_SRC:
			break;
		case OF1X_AT_SET_FIELD_SCTP_DST:
			break;
		case OF1X_AT_SET_FIELD_TP_SRC:
			break;
		case OF1X_AT_SET_FIELD_TP_DST:
			break;
		case OF1X_AT_SET_FIELD_ICMPV4_TYPE:
			break;
		case OF1X_AT_SET_FIELD_ICMPV4_CODE:
			break;
		case OF1X_AT_SET_FIELD_ICMPV6_TYPE:
			break;
		case OF1X_AT_SET_FIELD_ICMPV6_CODE:
			break;
		case OF1X_AT_SET_FIELD_PBB_ISID:
			break;
		case OF1X_AT_SET_FIELD_TUNNEL_ID:
			break;
		case OF1X_AT_SET_FIELD_PPPOE_CODE:
			break;
		case OF1X_AT_SET_FIELD_PPPOE_TYPE:
			break;
		case OF1X_AT_SET_FIELD_PPPOE_SID:
			break;
		case OF1X_AT_SET_FIELD_PPP_PROT:
			break;
		case OF1X_AT_SET_FIELD_GTP_MSG_TYPE:
			break;
		case OF1X_AT_SET_FIELD_GTP_TEID:
			break;
		case OF1X_AT_GROUP:
			break;
		case OF1X_AT_EXPERIMENTER:
			break;
		case OF1X_AT_OUTPUT:
			switch (iter->field.u64){
			case OF1X_PORT_MAX:
					break;
			case OF1X_PORT_IN_PORT:
				if (!port_analysis(iter->field.u32, ctl,sw))
					add_action = false;
					break;
			case OF1X_PORT_TABLE:
					break;
			case OF1X_PORT_NORMAL:
					break;
					/**
					 *
					 * Translate flood with
					 * ports avaible for the slice
					 */
			case OF1X_PORT_FLOOD:
				for (std::vector<std::string>::iterator it = virtual_agent::list_switch_by_id[sw->dpid]->select_slice(ctl)->ports_list.begin();
						it != virtual_agent::list_switch_by_id[sw->dpid]->select_slice(ctl)->ports_list.end();
						it++)
				{
					std::string port_temp;
					port_temp = *it;
					if (port_analysis(sw->port_to_num(port_temp), ctl, sw)) // If port is present add action to output this port
					{
						wrap_uint_t field;
						memset(&field,0,sizeof(wrap_uint_t));
						of1x_packet_action_t *action = NULL;
						field.u64 = sw->port_to_num(port_temp);
						action = of1x_init_packet_action( OF1X_AT_OUTPUT, field, NULL, NULL);
						of1x_push_packet_action_to_group(new_action_group, action);
					}

				}
				add_action = false;
					break;
			case OF1X_PORT_ALL:
					break;
			case OF1X_PORT_CONTROLLER:
					break;
			case OF1X_PORT_LOCAL:
					break;
			case OF1X_PORT_ANY:
					break;
			default:
				uint16_t porta = iter->field.u64;
				slice* slice = virtual_agent::list_switch_by_id[sw->dpid]->select_slice(ctl);
				if (!slice->has_port(sw->num_to_port(porta)))
				{
					add_action = false;
				}

				break;
			}

		}
		if (add_action)
			of1x_push_packet_action_to_group(new_action_group, iter);

	}

	/**
	 *
	 * Case of LLDP: push vlan to packet to guarantee
	 * slicing on others switches
	 */
	if (lldp ){
			flowspace_struct_t* tempFlowspace;
			uint16_t vlanID = 0;
			for (std::list<flowspace_struct*>::iterator it = virtual_agent::list_switch_by_id[sw->dpid]->flowspace_struct_list.begin();
						it != virtual_agent::list_switch_by_id[sw->dpid]->flowspace_struct_list.end();
						it++)
			{
				tempFlowspace = *it;
				std::list<flowspace_match_t*>::iterator _match;		// single match
					flowspace_match_t* tempMatch;

					for (_match = tempFlowspace->match_list.begin();
							_match !=tempFlowspace->match_list.end();
							_match ++)
					{
						tempMatch = *_match;
						if (tempMatch->type == FLOWSPACE_MATCH_VLAN_VID && slice_to_send.compare(tempFlowspace->slice) == 0)
						{
							vlanID = tempMatch->value->value.u16;
							break;
						}
					}
			}

			if (vlanID != 0)
			{
				wrap_uint_t field2;
				memset(&field2,0,sizeof(wrap_uint_t));
				field2.u64 = be16toh(vlanID);
				of1x_packet_action_t *action2 = of1x_init_packet_action( OF1X_AT_SET_FIELD_VLAN_VID, field2, NULL, NULL);;
				action2->type = OF1X_AT_SET_FIELD_VLAN_VID;
				action2->field.u16 = vlanID;
				of1x_push_packet_action_to_group(new_action_group, action2);
			}

	}

	return new_action_group;
}

of1x_flow_entry_t* virtual_agent::flow_entry_analysis(cofctl* ctl,
		of1x_flow_entry_t* entry, openflow_switch* sw, of_version_t of_version) {

	slice* slice = virtual_agent::list_switch_by_id[sw->dpid]->select_slice(ctl);

	/**
	 * Vector to store if that match was add in first step
	 * so that in second step it will not add
	 */
	vector<bool> *match_vector = new vector<bool>(FLOWSPACE_MATCH_MAX, false);

	of1x_flow_entry_t* new_entry = of1x_init_flow_entry(NULL, NULL, entry->notify_removal);

	new_entry->priority 		= entry->priority;
	new_entry->cookie 			= entry->cookie;
	new_entry->cookie_mask 		= entry->cookie_mask;
	new_entry->timer_info.idle_timeout	= entry->timer_info.idle_timeout;
	new_entry->timer_info.hard_timeout	= entry->timer_info.hard_timeout;

	/**
	 *
	 * First step: add flowspace match
	 */
	if (add_flowspace_match(entry,new_entry, sw, of_version, slice, match_vector) != ROFL_SUCCESS)
		throw eFspNoMatch();

	/**
	 *
	 * Second step: add flow_mod Match and check if it correct
	 */
	if (add_flow_mod_match(entry,new_entry,sw,of_version,match_vector) != ROFL_SUCCESS)
		throw eFlowModUnknown();

	// Instructions
	// For OF specification only ONE instruction per type
	for (int i = 0; i <= OF1X_IT_MAX; i++)
	{
		switch (i){
		case (OF1X_IT_NO_INSTRUCTION-1):
			break;
		case (OF1X_IT_APPLY_ACTIONS-1):
			if (entry->inst_grp.instructions[OF1X_IT_APPLY_ACTIONS-1].apply_actions)
			{
				ROFL_INFO("apply actions\n");
				of1x_action_group_t *new_action_group = of1x_init_action_group(0);
				new_action_group = virtual_agent::action_group_analysis(ctl, entry->inst_grp.instructions[OF1X_IT_APPLY_ACTIONS-1].apply_actions, sw);
				of1x_add_instruction_to_group(
						&(new_entry->inst_grp),
						OF1X_IT_APPLY_ACTIONS,
						(of1x_action_group_t*)new_action_group,
						NULL,
						NULL,
						0);
			}
			break;
		case (OF1X_IT_CLEAR_ACTIONS-1):
			if (entry->inst_grp.instructions[OF1X_IT_CLEAR_ACTIONS-1].type == OF1X_IT_CLEAR_ACTIONS)
			{
				ROFL_INFO("clear actions\n");
				of1x_add_instruction_to_group(
						&(new_entry->inst_grp),
						OF1X_IT_CLEAR_ACTIONS,
						NULL,
						NULL,
						NULL,
						0);
			}

			break;
		case (OF1X_IT_WRITE_ACTIONS-1):
			if (entry->inst_grp.instructions[OF1X_IT_WRITE_ACTIONS-1].write_actions)
			{
				ROFL_INFO("write actions\n");
				of1x_write_actions_t* new_write_actions = of1x_init_write_actions();
				new_write_actions = virtual_agent::write_actions_analysis(ctl,entry->inst_grp.instructions[OF1X_IT_WRITE_ACTIONS-1].write_actions,sw);

				of1x_add_instruction_to_group(
					&(new_entry->inst_grp),
					OF1X_IT_WRITE_ACTIONS,
					NULL,
					new_write_actions,
					NULL,
					0);
			}
			break;
		case (OF1X_IT_WRITE_METADATA-1):
			/////
			//TODO:[VA]metadata analysis
			//metadata check
			/////
			if (entry->inst_grp.instructions[OF1X_IT_WRITE_METADATA-1].type == OF1X_IT_WRITE_METADATA)
			{
				ROFL_INFO("metadata actions\n");
			of1x_add_instruction_to_group(
					&(new_entry->inst_grp),
					OF1X_IT_WRITE_METADATA,
					NULL,
					NULL,
					&(entry->inst_grp.instructions[OF1X_IT_WRITE_METADATA-1].write_metadata),
					0);
			}
			break;
		case (OF1X_IT_EXPERIMENTER-1):
			if (entry->inst_grp.instructions[OF1X_IT_EXPERIMENTER-1].type == OF1X_IT_EXPERIMENTER)
			{
			ROFL_INFO("experimenter actions\n");
			of1x_add_instruction_to_group(
					&(new_entry->inst_grp),
					OF1X_IT_EXPERIMENTER,
					NULL,
					NULL,
					NULL,
					0);
			}
			break;
		case (OF1X_IT_GOTO_TABLE-1):
			if (entry->inst_grp.instructions[OF1X_IT_GOTO_TABLE-1].go_to_table)
			{
				ROFL_INFO("goto actions\n");
			of1x_add_instruction_to_group(
					&(new_entry->inst_grp),
					OF1X_IT_GOTO_TABLE,
					NULL,
					NULL,
					NULL,
					entry->inst_grp.instructions[OF1X_IT_GOTO_TABLE-1].go_to_table);
			}
			break;
		}
	}

	return new_entry;
}

rofl_result_t virtual_agent::add_flowspace_match(of1x_flow_entry_t* entry, of1x_flow_entry_t* new_entry, openflow_switch* sw,
		of_version_t of_version, slice* slice, vector<bool> *match_vector)
{
	flowspace_struct* tempFlowspace;
	//iterate all flowspaces
	for (std::list<flowspace_struct*>::iterator it = virtual_agent::list_switch_by_id[sw->dpid]->flowspace_struct_list.begin();
					it != virtual_agent::list_switch_by_id[sw->dpid]->flowspace_struct_list.end();
					it++)
		{
			tempFlowspace = *it;
			if (slice->name.compare(tempFlowspace->slice) == 0) // If slice_name and flowspace_destination_slice are the same
			{
				std::list<flowspace_match_t*>::iterator _match;		// single match
				flowspace_match_t* flowspaceMatch;
				for (_match = tempFlowspace->match_list.begin();
						_match !=tempFlowspace->match_list.end();
						_match ++)
					{
						flowspaceMatch = *_match;
						of1x_match_t* temp_match = NULL;
						//Create and add new flow_entry
						switch (flowspaceMatch->type)
						{
						case FLOWSPACE_MATCH_VLAN_VID:
							of1x_match_t *match;
							match = of1x_init_vlan_vid_match(
												/*prev*/NULL,
												/*next*/NULL,
												(of_version == OF_VERSION_10)?
														flowspaceMatch->value->value.u16|OF1X_VLAN_PRESENT_MASK:
														flowspaceMatch->value->value.u16,
												(of_version == OF_VERSION_10)?
														0x1FFF:~0);

							temp_match = check_match_existance(flowspaceMatch->type,entry);
							uint16_t vlan;

							if (temp_match !=NULL ) // Flow_mod has vlan tag
							{
								vlan = (of_version==OF_VERSION_10)?temp_match->value->value.u16^OF1X_VLAN_PRESENT_MASK:temp_match->value->value.u16;
								if (vlan != flowspaceMatch->value->value.u16) // flow_mod_vlan different flowspace_vlan
								{
									return ROFL_FAILURE;
								}

							}
							of1x_add_match_to_entry(new_entry, match);
							match_vector->at((int)FLOWSPACE_MATCH_VLAN_VID) = true;
							break;
						default:
							break;
						}
					}

			} // end flowspace of slice

		}
return ROFL_SUCCESS;
}

rofl_result_t virtual_agent::add_flow_mod_match( of1x_flow_entry_t* entry, of1x_flow_entry_t* new_entry,
		openflow_switch* sw, of_version_t of_version, vector<bool> *match_vector)
{
	// Matches
	of1x_match_t *match_iter = entry->matches.head;

	while(match_iter != NULL)
	{
		bool add_match = true;
		//match present in flow_entry
		switch (match_iter->type){
			case OF1X_MATCH_IN_PORT:
				break;
			case OF1X_MATCH_IN_PHY_PORT:
				break;
			case OF1X_MATCH_METADATA:
				break;
			case OF1X_MATCH_ETH_DST:
				break;
			case OF1X_MATCH_ETH_SRC:
				break;
			case OF1X_MATCH_ETH_TYPE:
				break;
			case OF1X_MATCH_VLAN_VID:
				if (match_vector->at(OF1X_MATCH_VLAN_VID))
				{
					add_match = false;
				}
				break;
			case OF1X_MATCH_VLAN_PCP:
				break;
			case OF1X_MATCH_MPLS_LABEL:
				break;
			case OF1X_MATCH_MPLS_TC:
				break;
			case OF1X_MATCH_MPLS_BOS:
				break;
			case OF1X_MATCH_ARP_OP:
				break;
			case OF1X_MATCH_ARP_SPA:
				break;
			case OF1X_MATCH_ARP_TPA:
				break;
			case OF1X_MATCH_ARP_SHA:
				break;
			case OF1X_MATCH_ARP_THA:
				break;
			case OF1X_MATCH_NW_PROTO:
				break;
			case OF1X_MATCH_NW_SRC:
				break;
			case OF1X_MATCH_NW_DST:
				break;
			case OF1X_MATCH_IP_DSCP:
				break;
			case OF1X_MATCH_IP_ECN:
				break;
			case OF1X_MATCH_IP_PROTO:
				break;
			case OF1X_MATCH_IPV4_SRC:
				break;
			case OF1X_MATCH_IPV4_DST:
				break;
			case OF1X_MATCH_IPV6_SRC:
				break;
			case OF1X_MATCH_IPV6_DST:
				break;
			case OF1X_MATCH_IPV6_FLABEL:
				break;
			case OF1X_MATCH_ICMPV6_TYPE:
				break;
			case OF1X_MATCH_ICMPV6_CODE:
				break;
			case OF1X_MATCH_IPV6_ND_TARGET:
				break;
			case OF1X_MATCH_IPV6_ND_SLL:
				break;
			case OF1X_MATCH_IPV6_ND_TLL:
				break;
			case OF1X_MATCH_IPV6_EXTHDR:
				break;
			case OF1X_MATCH_TP_SRC:
				break;
			case OF1X_MATCH_TP_DST:
				break;
			case OF1X_MATCH_TCP_SRC:
				break;
			case OF1X_MATCH_TCP_DST:
				break;
			case OF1X_MATCH_UDP_SRC:
				break;
			case OF1X_MATCH_UDP_DST:
				break;
			case OF1X_MATCH_SCTP_SRC:
				break;
			case OF1X_MATCH_SCTP_DST:
				break;
			case OF1X_MATCH_ICMPV4_TYPE:
				break;
			case OF1X_MATCH_ICMPV4_CODE:
				break;
			case OF1X_MATCH_PBB_ISID:
				break;
			case OF1X_MATCH_TUNNEL_ID:
				break;
			case OF1X_MATCH_PPPOE_CODE:
				break;
			case OF1X_MATCH_PPPOE_TYPE:
				break;
			case OF1X_MATCH_PPPOE_SID:
				break;
			case OF1X_MATCH_PPP_PROT:
				break;
			case OF1X_MATCH_GTP_MSG_TYPE:
				break;
			case OF1X_MATCH_GTP_TEID:
				break;
			case OF1X_MATCH_MAX:
				break;
		}

		//Match to add
		//Why using this change?
		of1x_match_t *next_match = match_iter->next;
		if (add_match)
		{
			// Because this function change match_iter
			// in function __of1x_match_group_push_back while counting
			of1x_add_match_to_entry(new_entry, match_iter);
		}
		match_iter = next_match;
	};

	return ROFL_SUCCESS;

}

va_switch* virtual_agent::get_vaswitch(uint64_t* id, std::string* name) {

	if (id && name )
		return NULL;

	for (std::list<va_switch*>::iterator it = list_va_switch.begin();
			it != list_va_switch.end();
			it++)
	{
		va_switch* vaswitch = *it;

		if (!id && name && vaswitch->dp_name == *name) //case with name passe
		{
			return vaswitch;
		}
		else if (id && !name && vaswitch->dp_id == *id) //case with id passed
		{
			return vaswitch;
		}
		else if (id && name && vaswitch->dp_id == *id && vaswitch->dp_name == *name) //case with both id and name passed
		{
			return vaswitch;
		}
		else
			return NULL;
	}

return NULL;
}

/**
 * Return the match of the flow_mod that correspond to flowspace_match_type
 */
of1x_match_t* virtual_agent::check_match_existance(flowspace_match_type type, of1x_flow_entry_t* entry) {

	of1x_match_t *match_iter = entry->matches.head;
	while(match_iter != NULL)
	{
		if (int(match_iter->type) == int(type)) //Valid because the two structures are equal;
		{
			return match_iter;
		}
		match_iter = match_iter->next;
	}
	return NULL;
}

/**
 * Given groupID modified from VA
 * returns sliceID
 */
uint32_t virtual_agent::obatin_sliceID(uint32_t groupID) {
	uint32_t new_value;

	//Set zero from 12 to 31
	new_value = groupID & (~group_ID_mask);

	//Shift 20 position to right
	new_value = new_value >> 20;

	return new_value;
}

/**
 * Given groupID modified from VA
 * returns groupID
 */
uint32_t virtual_agent::obatin_groupID(uint32_t groupID) {
	uint32_t new_value;

	//Set zero from 12 to 31
	new_value = groupID & (~slice_ID_mask);

	return new_value;
}


of1x_write_actions_t* virtual_agent::write_actions_analysis(cofctl* ctl,
		of1x_write_actions_t* write_action, openflow_switch* sw) {

	std::string slice_to_send = virtual_agent::list_switch_by_id[sw->dpid]->select_slice(ctl)->name;

	of1x_write_actions_t* new_write_actions = of1x_init_write_actions();

	for (int i=0; i< OF1X_AT_NUMBER; i++)
	{
		bool add_action = true;
		if ( write_action->write_actions[i].type == OF1X_AT_SET_FIELD_VLAN_VID )
		{
			flowspace_struct_t* tempFlowspace;
			for (std::list<flowspace_struct*>::iterator it = virtual_agent::list_switch_by_id[sw->dpid]->flowspace_struct_list.begin();
						it != virtual_agent::list_switch_by_id[sw->dpid]->flowspace_struct_list.end();
						it++)
			{
				tempFlowspace = *it;
				std::list<flowspace_match_t*>::iterator _match;		// single match
					flowspace_match_t* tempMatch;
					for (_match = tempFlowspace->match_list.begin();
							_match !=tempFlowspace->match_list.end();
							_match ++)
					{
						tempMatch = *_match;
						if (tempMatch->type == FLOWSPACE_MATCH_VLAN_VID && // Flowspace with vlan_vid
								tempMatch->value->value.u16 == write_action->write_actions[OF1X_AT_SET_FIELD_VLAN_VID].field.u16 && // Value of flowspace equal to value of entry
								slice_to_send.compare(tempFlowspace->slice) != 0) // Comparision between slice and flowspace
						{
							add_action = false;
							break;
						}
					}
			}
		}
		else if ( write_action->write_actions[i].type == OF1X_AT_OUTPUT )
		{
			switch (write_action->write_actions[OF1X_AT_OUTPUT].field.u64){
			case OF1X_PORT_MAX:
					break;
			case OF1X_PORT_IN_PORT:
				if (!port_analysis(write_action->write_actions[OF1X_AT_OUTPUT].field.u32, ctl,sw))
					add_action = false;
					break;
			case OF1X_PORT_TABLE:
					break;
			case OF1X_PORT_NORMAL:
					break;
			case OF1X_PORT_FLOOD:
				for (std::vector<std::string>::iterator it = virtual_agent::list_switch_by_id[sw->dpid]->select_slice(ctl)->ports_list.begin();
						it != virtual_agent::list_switch_by_id[sw->dpid]->select_slice(ctl)->ports_list.end();
						it++)
				{
					std::string port_temp;
					port_temp = *it;
					if (port_analysis(sw->port_to_num(port_temp), ctl, sw)) // If port is present add action to output this port
					{
						wrap_uint_t field;
						memset(&field,0,sizeof(wrap_uint_t));
						of1x_packet_action_t *action = NULL;
						field.u64 = sw->port_to_num(port_temp);
						action = of1x_init_packet_action( OF1X_AT_OUTPUT, field, NULL, NULL);
						of1x_set_packet_action_on_write_actions(new_write_actions, action);
					}

				}
				add_action = false;
					break;
			case OF1X_PORT_ALL:
					break;
			case OF1X_PORT_CONTROLLER:
					break;
			case OF1X_PORT_LOCAL:
					break;
			case OF1X_PORT_ANY:
					break;
			default:
				uint16_t porta = write_action->write_actions[OF1X_AT_OUTPUT].field.u64;
				slice* slice = virtual_agent::list_switch_by_id[sw->dpid]->select_slice(ctl);
				if (!slice->has_port(sw->num_to_port(porta)))
				{
				}

				break;
			}
		}
		if (add_action && write_action->write_actions[i].type != 0)
		{
			of1x_set_packet_action_on_write_actions(new_write_actions, &write_action->write_actions[i]);
		}

	}

	return new_write_actions;
}

bool virtual_agent::check_flowspace_existance(std::string flowspace_name,
		uint64_t* switch_id, std::string* switch_name) {

	flowspace_struct_t* temp_flowspace;
	if (switch_id)
	{
		for (std::list<flowspace_struct_t*>::iterator it = virtual_agent::list_switch_by_id[*switch_id]->flowspace_struct_list.begin();
				it != virtual_agent::list_switch_by_id[*switch_id]->flowspace_struct_list.end();
				it++)
		{
			temp_flowspace = *it;
			if (temp_flowspace->name == flowspace_name)
				return true;
		}
	}
	else if (switch_name)
	{
		for (std::list<flowspace_struct_t*>::iterator it = virtual_agent::list_switch_by_name[*switch_name]->flowspace_struct_list.begin();
				it != virtual_agent::list_switch_by_name[*switch_name]->flowspace_struct_list.end();
				it++)
		{
			temp_flowspace = *it;
			if (temp_flowspace->name == flowspace_name)
				return true;
		}
	}

	return false;
}

void virtual_agent::write_cfg(std::string name) {
	Config cfg;
	try{
		std::string command = "rm " + name;
		int res = system(command.c_str());
		std::cout << res;
	}
	catch(...)
	{
		std::cout << "File does not exists\n";
	}

	Setting &root = cfg.getRoot();
	Setting &config = root.add("config", Setting::TypeGroup);
	Setting &openflow = config.add("openflow", Setting::TypeGroup);
	Setting &logicalswitches = openflow.add("logical-switches", Setting::TypeGroup);

	//Logical switches
	va_switch* tempSwitch;
	for(std::map<uint64_t, va_switch*>::iterator it = virtual_agent::list_switch_by_id.begin();
			it != virtual_agent::list_switch_by_id.end();
			it++)
	{
		tempSwitch = it->second;
		Setting &newsw = logicalswitches.add(tempSwitch->dp_name.c_str(), Setting::TypeGroup);

		////////////////
		//Switch setting
		////////////////
		ostringstream dpid;
		//dpid << std::hex << setw(3) << tempSwitch->dp_id;
		dpid << std::hex  << tempSwitch->dp_id;
		newsw.add("dpid", Setting::TypeString) = "0x"+dpid.str();
		newsw.add("version", Setting::TypeFloat) = (tempSwitch->of_switch_vs == OF_VERSION_10)?1.0:1.2;
		newsw.add("num-of-tables", Setting::TypeInt) = 1;//(int)tempSwitch->no_table;

		Setting &port = newsw.add("ports", Setting::TypeList);
		for (std::list<std::string>::iterator port_iter = tempSwitch->port_list.begin();
				port_iter != tempSwitch->port_list.end();
				port_iter++)
		{
			std::string port_name = *port_iter;
			port.add(Setting::TypeString) = port_name;
		}


	}

	////////////////
	//Virtual interfaces
	///////////////

	Setting &vi = virtual_link_setting.getRoot();
	if (vi.getLength() > 0)
	{
		Setting &interfaces = config.add("interfaces", Setting::TypeGroup);
		Setting &virtuall = interfaces.add("virtual", Setting::TypeGroup);
		for(int i = 0; i<vi.getLength(); ++i){
			std::string key  ;
			std::string value  ;
			std::string name = vi[i].getName();
			vi[i].lookupValue("link", key);
			vi[i].lookupValue("lsi", value);

			libconfig::Setting &vif_cfg = virtuall.add(name, libconfig::Setting::TypeGroup);
			vif_cfg.add("link", libconfig::Setting::TypeString) = key;
			vif_cfg.add("lsi", libconfig::Setting::TypeString) = value;
		}
	}
	////////////////
	//Empty virtual agent
	////////////////
	root.add("virtual-agent", Setting::TypeGroup);

	////////////////
	//Slice setting
	////////////////
	Setting &slice_group = root.add("slice", Setting::TypeGroup);
	for (std::list<slice*>::iterator slice_iter = virtual_agent::all_slices_list.begin();
			slice_iter != all_slices_list.end();
			slice_iter++)
	{
		slice* temp_slice = *slice_iter;
		Setting &slice_cfg = slice_group.add(temp_slice->name.c_str(), Setting::TypeGroup);
		slice_cfg.add("ip", Setting::TypeString) = temp_slice->ip_string.c_str();
		slice_cfg.add("port", Setting::TypeInt) = temp_slice->port_int;

		for(std::map<uint64_t, va_switch*>::iterator it = virtual_agent::list_switch_by_id.begin();
				it != virtual_agent::list_switch_by_id.end();
				it++)
		{
			tempSwitch = it->second;
			slice* another_slice = virtual_agent::check_slice_existance(temp_slice->name, tempSwitch->dp_name);
			if (another_slice != NULL)
			{
				Setting &list_port = slice_cfg.add(tempSwitch->dp_name, Setting::TypeArray);
				for (std::vector<std::string>::iterator slice_port_iter = another_slice->ports_list.begin();
						slice_port_iter != another_slice->ports_list.end();
						slice_port_iter++)
				{
					list_port.add(Setting::TypeString) = *slice_port_iter;
				}
			}
		}
	}

	////////////////
	//Flowspace setting
	////////////////
	Setting &flowspace_group = root.add("flowspace", Setting::TypeGroup);
	for (std::list<flowspace_struct_t*>::iterator fs_it = all_flowspaces_list.begin();
			fs_it != all_flowspaces_list.end();
			fs_it++)
	{
		flowspace_struct_t *fs_temp = *fs_it;
		Setting &flowspace = flowspace_group.add(fs_temp->name, Setting::TypeGroup);
		flowspace.add("priority",Setting::TypeInt) = (int)fs_temp->priority;
		flowspace.add("vlan_vid", Setting::TypeInt) = fs_temp->match_list.front()->value->value.u16;
		flowspace.add("slice_action", Setting::TypeString) = fs_temp->slice.c_str();
		Setting &flowspace_dpname = flowspace.add("flowspace_dpname", Setting::TypeList);
		for(std::map<uint64_t, va_switch*>::iterator it = virtual_agent::list_switch_by_id.begin();
				it != virtual_agent::list_switch_by_id.end();
				it++)
		{
			tempSwitch = it->second;
			if (virtual_agent::check_flowspace_existance(fs_temp->name,NULL, &tempSwitch->dp_name))
			{
				flowspace_dpname.add(Setting::TypeString) = tempSwitch->dp_name;
			}
		}
	}

	try{
		cfg.writeFile(name.c_str());
	}
	catch(...)
	{
		std::cerr << "Error writing file\n";
	}

}

slice* virtual_agent::get_slice_by_name(std::string slice_name) {
	return NULL;
}

std::list<va_switch*> virtual_agent::get_all_va_switch(of_version_t version) {
	std::list<va_switch*> returned_list;
	//std::list<va_switch*> all_switches_list;
	for (std::list<va_switch*>::iterator it = all_switches_list.begin();
			it != all_switches_list.end();
			it++)
	{
		va_switch *tempSwitch = *it;
		if (tempSwitch->of_switch_vs == version)
			returned_list.push_back(tempSwitch);
	}

	return returned_list;
}
