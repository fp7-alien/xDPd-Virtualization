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
 * virtualagent.h
 *
 *  @author Daniel Depaoli <daniel.depaoli (at) create-net.org>
 */

#include <rofl/platform/unix/cunixenv.h>
#include <rofl/datapath/afa/fwd_module.h>
#include <rofl/common/utils/c_logger.h>
#include "../management/switch_manager.h"
#include "../management/port_manager.h"
#include <pthread.h>

#include "../openflow/openflow_switch.h"
#include "slice.h"
#include "flowspace.h"
#include "va_switch.h"

#include <libconfig.h++>

#ifndef VIRTUALAGENT_H_
#define VIRTUALAGENT_H_
using namespace xdpd;

class eVAbase		: public cerror {};	// base error class for virtualization agent
class eSliceConfigError: public eVAbase {};
class eSliceExist	: public eVAbase {};
class eSliceNotFound : public eVAbase {};

class virtual_agent {

private:
	static std::list<va_switch*> list_va_switch;
	static std::list<slice*> all_slices_list;
	static std::list<flowspace_struct_t*> all_flowspaces_list;

public:

	// List of switch
	//TODO:[VA]use double linked list
	static va_switch* get_vaswitch(uint64_t* id = NULL, std::string* name = NULL);
	static std::map<uint64_t, va_switch*> list_switch_by_id;
	static std::map<std::string, va_switch*> list_switch_by_name;
	static libconfig::Config virtual_link_setting;

	static bool add_slice(slice* slice_to_add, bool connect=true);
	static bool add_flowspace(flowspace_struct_t* flowspace_to_add);
	static void add_switch(va_switch* _switch);

	//TODO:[VA]merge into single function
	static bool check_slice_existance(std::string slice_name, uint64_t switch_id);
	static slice* check_slice_existance(std::string slice_name, std::string switch_name);

	static bool check_flowspace_existance(std::string flowspace_name, uint64_t* switch_id, std::string* switch_name);

	virtual_agent();
	virtual ~virtual_agent();

	static bool order_by_priority(flowspace_struct_t* first, flowspace_struct_t* second) {
		return first->priority > second->priority;
	}

	/**
	 * Functions to activate only one time the state of virtual agent
	 *
	 */
	static void active_va(bool value);

	/**
	 * return the state of VA
	 */
	static bool is_active();

	/**
	 *
	 * when adding new slice a counter stores an incremental number
	 */
	static uint32_t increase_slice_counter(std::string slice_name);

	/**
	 * Used in packet out and flow_mod with apply-action
	 *
	 * @param ctl
	 * @param action_group
	 * @param sw
	 * @param lldp
	 * @return the modified action group based on virtualization
	 */
	static of1x_action_group_t* action_group_analysis(cofctl *ctl, of1x_action_group_t *action_group, openflow_switch* sw, bool lldp = false );

	/**
	 * Used in flow_mod
	 *
	 * @param ctl
	 * @param entry
	 * @param sw
	 * @param of_version
	 * @return modified version of flow_entry
	 */
	static of1x_flow_entry_t* flow_entry_analysis(cofctl *ctl,of1x_flow_entry_t *entry, openflow_switch* sw, of_version_t of_version);

	/**
	 *
	 * Functions to manage groups
	 */
	static uint32_t* change_group_id(uint32_t groupID, uint32_t sliceID);
	static uint32_t obatin_sliceID(uint32_t groupID);
	static uint32_t obatin_groupID(uint32_t groupID);

	static void write_cfg(std::string name);


private:

	static bool* active;
	static uint32_t slice_counter;

	/**
	 *
	 * @return if the port is belong to slice
	 */
	static bool port_analysis(uint32_t port, cofctl* ctl, openflow_switch* sw);

	/**
	 * Variable used to guarantee that all slices in the virtual switches have
	 * the same id
	 */
	static std::map<std::string, uint32_t> slice_id_map;

	/**
	 *
	 * Add the flowspace match to flow entry match for guarantee the isolation
	 */
	static rofl_result_t add_flowspace_match(of1x_flow_entry_t* entry, of1x_flow_entry_t* new_entry, openflow_switch* sw, of_version_t of_version, slice* slice, vector<bool> *match_vector);

	/**
	 *
	 * Analyze the flow_mod match and add to flow entry
	 */
	static rofl_result_t add_flow_mod_match(of1x_flow_entry_t* entry, of1x_flow_entry_t* new_entry, openflow_switch* sw, of_version_t of_version, vector<bool> *match_vector);

	/**
	 *
	 * Analysis of write actions.
	 * Very similar to action_group_analysis
	 */
	static of1x_write_actions_t* write_actions_analysis(cofctl *ctl, of1x_write_actions_t *action_group, openflow_switch* sw);

	static of1x_match_t* check_match_existance(flowspace_match_type type, of1x_flow_entry_t* entry);

	static slice* get_slice_by_name(std::string slice_name);


};

#endif /* VIRTUALAGENT_H_ */
