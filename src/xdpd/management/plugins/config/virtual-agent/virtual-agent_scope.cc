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

#include "virtual-agent_scope.h"
#include <vector>
#include <stdlib.h>
#include <inttypes.h>
//#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_pipeline.h>
//#include "../../../switch_manager.h"
//#include "../../../port_manager.h"
//#include "../../../../openflow/openflow_switch.h"

#include "../../../../virtual-agent/virtualagent.h"

#define VIRTUAL_AGENT_ACTIVE "virtualization"
#define VIRTUAL_GATEWAY_IP "virtual-gateway-ip"
#define VIRTUAL_GATEWAY_PORT "virtual-gateway-port"


using namespace xdpd;
using namespace rofl;



virtual_agent_scope::virtual_agent_scope(std::string name, bool mandatory):scope(name, mandatory){
	
	register_parameter(VIRTUAL_AGENT_ACTIVE, false);
	register_parameter(VIRTUAL_GATEWAY_IP, false);
	register_parameter(VIRTUAL_GATEWAY_PORT, false);

}

void xdpd::virtual_agent_scope::post_validate(libconfig::Setting& setting,
		bool dry_run) {

}

void xdpd::virtual_agent_scope::pre_validate(libconfig::Setting& setting,
		bool dry_run) {

		if (setting.exists(VIRTUAL_AGENT_ACTIVE))
		{
			std::string value = setting[VIRTUAL_AGENT_ACTIVE];
			if (value.compare("true") == 0 )
			{
				virtual_agent::active_va(true);
			}
			else if (value.compare("false") == 0 )
			{
				virtual_agent::active_va(false);
			}
			else
			{
				ROFL_ERR("[Virtual Agent] Value not valid. Error at line %i\n", setting[VIRTUAL_AGENT_ACTIVE].getSourceLine());
				throw eConfParseError();
			}
		}
}
