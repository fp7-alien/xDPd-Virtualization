/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef OPENFLOW12_SWITCH_H
#define OPENFLOW12_SWITCH_H 

#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_switch.h>
#include <rofl/datapath/afa/fwd_module.h>
#include <rofl/datapath/afa/openflow/openflow1x/of1x_fwd_module.h>
#include "../openflow_switch.h"
#include "../openflow10/openflow10_switch.h"
#include "of12_endpoint.h"

/**
* @file openflow12_switch.h
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief OF1.2 switch implementation 
*/

using namespace rofl;

namespace xdpd{

/**
* @brief Implementation of a (logical) OpenFlow v1.2 switch.
* @ingroup cmm_of 
*
* @description This includes the OF endpoint and the binding with the
* forwarding plane (through AFA)
*/
class openflow12_switch : public openflow_switch {


public:
	/**
	 * Constructor
	 */
	openflow12_switch(uint64_t dpid,
				std::string const& dpname,
				unsigned int num_of_tables,
				int* ma_list,
				int reconnect_start_timeout,
				caddress const& controller_addr = caddress(AF_INET, "127.0.0.1", 6633),
				caddress const& binding_addr = caddress(AF_INET, "0.0.0.0", 0)) throw (eOfSmVersionNotSupported);


	/**
	 * Destructor
	 */
	virtual ~openflow12_switch(void);

	/*
	* Public interface for the instance
	*/
	
	virtual afa_result_t process_packet_in(uint8_t table_id,
					uint8_t reason,
					uint32_t in_port,
					uint32_t buffer_id,
					uint8_t* pkt_buffer,
					uint32_t buf_len,
					uint16_t total_len,
					of1x_packet_matches_t matches,
					cofctl* controller);


	virtual afa_result_t process_flow_removed(uint8_t reason, of1x_flow_entry_t* removed_flow_entry);

	/*
	* Port async notifications
	*/
	virtual afa_result_t notify_port_add(switch_port_t* port);
	
	virtual afa_result_t notify_port_delete(switch_port_t* port);
	
	virtual afa_result_t notify_port_status_changed(switch_port_t* port);

	 /*
	 * Connecting and disconnecting from a controller entity
	 */
	virtual void rpc_connect_to_ctl(caddress const& controller_addr);

	virtual void rpc_disconnect_from_ctl(caddress const& controller_addr);

	/**
	 * Constructor with no controller
	 */
	openflow12_switch(uint64_t dpid,
				std::string const& dpname,
				unsigned int num_of_tables,
				int* ma_list) throw (eOfSmVersionNotSupported);

};

}// namespace rofl

#endif /* OPENFLOW12_SWITCH_H_ */
