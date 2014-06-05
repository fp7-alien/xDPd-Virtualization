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
 * slicescope.h
 *
 *  @author Daniel Depaoli <daniel.depaoli (at) create-net.org>
 */

#ifndef SLICESCOPE_H_
#define SLICESCOPE_H_

#include "../scope.h"

#include <rofl/common/caddress.h>

namespace xdpd {

class slice_scope: public scope {

public:
	slice_scope(std::string scope_name="slice", bool mandatory=true);
	virtual ~slice_scope();

protected:
	virtual void post_validate(libconfig::Setting& setting, bool dry_run);
	void parse_all_slice_settings(libconfig::Setting& setting,
			std::string name,
			std::string ip,
			int port);
	void parse_slice_ip(libconfig::Setting& setting, rofl::caddress& address, rofl::caddress& slave_controller, unsigned int* reconnect_time);


	//virtual void pre_validate(libconfig::Setting& setting, bool dry_run);
};

class root_datapath_scope: public scope {

public:
	root_datapath_scope(std::string scope_name="datapath", bool mandatory=false);

protected:

	virtual void pre_validate(libconfig::Setting& setting, bool dry_run);
};

}  // namespace xdpd



#endif /* SLICESCOPE_H_ */
