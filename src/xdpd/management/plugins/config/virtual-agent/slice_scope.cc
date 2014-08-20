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
 * slicescope.cc
 *
 *  @author Daniel Depaoli <daniel.depaoli (at) create-net.org>
 */

#include "slice_scope.h"
#include "flowspace_scope.h"
#include "../../../../virtual-agent/virtualagent.h"
#include "../../../../management/switch_manager.h"

#include "../../../../virtual-agent/slice.h"

using namespace xdpd;
using namespace rofl;

#define SLICE_IP  "ip"
#define SLICE_PORT  "port"
#define SLICE_DPNAME "slice_dpname"
#define SLICE_PORTS_LIST "ports_list"

//lsi_scope::lsi_scope(std::string name, bool mandatory):scope(name, mandatory){
slice_scope::slice_scope(std::string name, bool mandatory):scope(name, mandatory){

	register_parameter(SLICE_IP, true);
	register_parameter(SLICE_PORT, true);
}

slice_scope::~slice_scope() {
	// TODO Auto-generated destructor stub
}

void slice_scope::parse_all_slice_settings(libconfig::Setting& setting,
		std::string name, std::string slice_ip, int port) {

}


void slice_scope::post_validate(libconfig::Setting& setting,
		bool dry_run) {

	std::string slice_ip = "127.0.0.1";
	int port=6633;

	if (setting.exists(SLICE_IP) && setting.exists(SLICE_PORT) )
	{
		std::string _slice_ip = setting[SLICE_IP];
		slice_ip = _slice_ip;
		std::string slice_name = setting.getName();

		port = setting[SLICE_PORT];
		if(port < 1 || port > 65535){
			ROFL_ERR("%s: invalid controller TCP port number %u. Must be [1-65535]\n", setting.getPath().c_str(), port);
			throw eConfParseError();
		}

			// If not dry run add slice
			if(!dry_run){
				std::list<std::string> datapath_list = switch_manager::list_sw_names();

				int count = 0;

				// Iterator for all datapath avaible
				for (std::list<std::string>::iterator it = datapath_list.begin();
						it != datapath_list.end();
						it++)
				{
					std::string dp_name = *it;

					// Check if datapath is present inside slice setting.
					// Else not add slice to this datapath
					if (setting.exists(dp_name.c_str()) )
					{
						std::vector<std::string> ports_list; //Empty port list
						openflow_switch* switch_temp = switch_manager::find_by_name(dp_name);

						// check port list existance

						// case with an array
						if ( setting[dp_name.c_str()].isArray() && setting[dp_name.c_str()].getLength()> 0 )
						{
							for(int i=0; i<setting[dp_name.c_str()].getLength(); ++i)
							{
								std::string port = setting[dp_name.c_str()][i];
								if ( switch_temp->port_is_present(port) )
								{
									ports_list.push_back(port);
								}
								else
								{
									ROFL_INFO("Port %s not present in datapath %s. Impossible to continue. Line %i\n",
											port.c_str(),dp_name.c_str(), setting[dp_name.c_str()].getSourceLine());
									throw eConfParseError();
								}
							}
						}
						else if ( !setting[dp_name.c_str()].isArray() )
						{
							ROFL_INFO("%s at line %i is not an array.\n\t Write: %s = [port1, port2, portX]\n",
									dp_name.c_str(), setting[dp_name.c_str()].getSourceLine(),dp_name.c_str());
							throw eConfParseError();
						}
						else if (setting[dp_name.c_str()].getLength() == 0)
						{
							std::vector<std::string>::iterator port_iter;
							for (port_iter = switch_manager::find_by_name(dp_name)->port_list.begin();
									port_iter != switch_manager::find_by_name(dp_name)->port_list.end();
									port_iter++)
							{
								std::string port = *port_iter;
								ports_list.push_back(port);
							}
							ROFL_INFO("%s at line %i is an empty array. Add all ports!\n", dp_name.c_str(), setting[dp_name.c_str()].getSourceLine());
							//ports_list.clear();
						}

						count = count + 1;

						// Create address for slice controller
						caddress address = caddress(AF_INET, slice_ip.c_str(), port);

						// Connect the slice controller to the datapath
						cofctl* controller = NULL;
						controller = switch_manager::find_by_name(dp_name)->rpc_connect_to_ctl_return(address, controller);

						//slice* slice_to_add = new slice(dp_name, switch_manager::dpid_from_name(dp_name), slice_name, address, ports_list, controller);
						slice* slice_to_add = new slice(dp_name, switch_manager::dpid_from_name(dp_name),slice_name, port, slice_ip, ports_list, controller );
						virtual_agent::add_slice(slice_to_add, true);

						ROFL_INFO("\nNew slice added %s, %s:%i. ID=%i\n", slice_name.c_str(), slice_ip.c_str(), port, slice_to_add->slice_id);
					}
				}

				if (count == 0)
				{
					ROFL_INFO("Slice %s is not installed in any datapath. Line %i\n",slice_name.c_str(), setting.getSourceLine());
					throw eConfParseError();
				}
			}
	}


}

xdpd::root_datapath_scope::root_datapath_scope(std::string scope_name,
		bool mandatory):scope(name, mandatory) {
ROFL_INFO("New root_datapath_scope\n");
}

void root_datapath_scope::pre_validate(libconfig::Setting& setting,
		bool dry_run) {

	if(setting.getLength() != 0)
	{
	 	for(int i = 0; i<setting.getLength(); ++i){
			ROFL_INFO("Slice %s with datapath %s\n",setting.getParent().getName(), setting[i].getName());
			if (switch_manager::find_by_name(setting[i].getName()) == NULL && !dry_run)
			{
				ROFL_ERR("Datapath %s does not exist. Line %i\n", setting[i].getName(), setting.getSourceLine());
				throw eConfParseError();
			}
			else
				ROFL_INFO("Datapath %s exists! Line %i\n", setting[i].getName(), setting.getSourceLine());
			register_parameter(std::string(setting[i].getName()), false);
		}

	}
	else
		ROFL_INFO("For slice %s all switches and all ports \n", setting.getParent().getName());

}
