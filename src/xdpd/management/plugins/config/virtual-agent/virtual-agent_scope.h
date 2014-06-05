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

#ifndef CONFIG_VIRTUALAGENT_PLUGIN_H
#define CONFIG_VIRTUALAGENT_PLUGIN_H

#include "../scope.h"


namespace xdpd {

class virtual_agent_scope:public scope {
	
public:
	virtual_agent_scope(std::string scope_name="virtual-agent", bool mandatory=true);
		
protected:
	virtual void post_validate(libconfig::Setting& setting, bool dry_run);
	virtual void pre_validate(libconfig::Setting& setting, bool dry_run);
	
};

}// namespace xdpd 

#endif /* CONFIG_VIRTUALAGENT_PLUGIN_H_ */


