#include "management/switch_manager.h"
#include "management/port_manager.h"
#include "openflow/openflow_switch.h"
#include <rofl/datapath/afa/cmm.h>
#include <rofl/datapath/afa/openflow/openflow1x/of1x_cmm.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_switch.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_flow_entry.h>
#include <rofl/datapath/pipeline/physical_switch.h>

#include "virtual-agent/virtualagent.h"
#include "virtual-agent/flowspace.h"

using namespace xdpd;

/*
* Dispatching of platform related messages comming from the fwd_module 
*/

afa_result_t cmm_notify_port_add(switch_port_t* port){
	
	openflow_switch* sw;
	
	if (!port || !port->attached_sw){
		return AFA_FAILURE;
	}
	if( (sw=switch_manager::find_by_dpid(port->attached_sw->dpid)) == NULL)
		return AFA_FAILURE;	

	//Notify MGMT framework
	//TODO:

	return sw->notify_port_add(port);
}


afa_result_t cmm_notify_port_delete(switch_port_t* port){
	
	openflow_switch* sw;
	
	if (!port || !port->attached_sw){
		return AFA_FAILURE;
	}
	if( (sw=switch_manager::find_by_dpid(port->attached_sw->dpid)) == NULL)
		return AFA_FAILURE;	

	//Notify MGMT framework
	//TODO:

	return sw->notify_port_delete(port);
}


afa_result_t cmm_notify_port_status_changed(switch_port_t* port){
	
	openflow_switch* sw;
	
	if (!port || !port->attached_sw){
		return AFA_FAILURE;
	}
	if( (sw=switch_manager::find_by_dpid(port->attached_sw->dpid)) == NULL)
		return AFA_FAILURE;	

	//Notify MGMT framework
	//TODO:

	return sw->notify_port_status_changed(port);
}

/*
* Driver CMM Openflow calls. Demultiplexing to the appropiate openflow_switch instance.
*/ 
afa_result_t cmm_process_of1x_packet_in(const of1x_switch_t* sw,
					uint8_t table_id,
					uint8_t reason,
					uint32_t in_port,
					uint32_t buffer_id,
					uint8_t* pkt_buffer,
					uint32_t buf_len,
					uint16_t total_len,
					of1x_packet_matches_t matches)
{

	openflow_switch* dp=NULL;
	
	if (!sw) 
		return AFA_FAILURE;

	dp = switch_manager::find_by_dpid(sw->dpid);

	if(!dp)
		return AFA_FAILURE;

/**
 * Virtualization agent select the controller based on
 * his database and on of1x_packet_matches_t matches
 *
 * Daniel Depaoli <daniel.depaoli (at) create-net.org>
 */
	if (virtual_agent::is_active())
	{
			va_switch* vaswitch = virtual_agent::list_switch_by_id[dp->dpid];

			std::list<flowspace_struct_t*>::iterator _flowspace; //declaration of single flowspace
			std::list<flowspace_match_t*>::iterator _match;		// declaration of single match
			flowspace_struct_t* tempFlowspace;
			slice* slice = NULL;

			bool matched = false;

			/**
			 * iterate throgh the flowspaces of the switch
			 */
			for (_flowspace = vaswitch->flowspace_struct_list.begin();
					_flowspace != vaswitch->flowspace_struct_list.end();
					_flowspace++)
			{
				tempFlowspace = *_flowspace;
				if ( vaswitch->check_match(matches,tempFlowspace->match_list) )
				{
					matched = true;
					slice = vaswitch->get_slice(tempFlowspace->slice);
					break;
				}
			}

			cofctl* controller = NULL;

			if (matched)
			{
				controller = virtual_agent::list_switch_by_id[dp->dpid]->controller_map[slice->name];
			}

			if (controller != NULL)
				return dp->process_packet_in(table_id,
							reason,
							in_port,
							buffer_id,
							pkt_buffer,
							buf_len,
							total_len,
							matches,
							controller);
			else
			{
				/**
				 * if no controller was found
				 * return success to not stop.
				 * Packet in is not send.
				 */
				return AFA_SUCCESS;
			}
	}

	else
		return dp->process_packet_in(table_id,
						reason,
						in_port,
						buffer_id,
						pkt_buffer,
						buf_len,
						total_len,
						matches);
}

afa_result_t cmm_process_of1x_flow_removed(const of1x_switch_t* sw, uint8_t reason, of1x_flow_entry_t* removed_flow_entry){

	openflow_switch* dp=NULL;
	
	if (!sw) 
		return AFA_FAILURE;
	
	dp = switch_manager::find_by_dpid(sw->dpid);

	if(!dp)
		return AFA_FAILURE;

	return dp->process_flow_removed(reason, removed_flow_entry);
}
