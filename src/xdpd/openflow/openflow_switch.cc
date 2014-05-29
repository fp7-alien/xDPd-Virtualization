#include "openflow_switch.h"

using namespace rofl;
using namespace xdpd;

openflow_switch::openflow_switch(const uint64_t dpid, const std::string &dpname, const of_version_t version) :
		ofswitch(NULL),
		endpoint(NULL),
		dpid(dpid),
		dpname(dpname),
		version(version)
{

}

bool xdpd::openflow_switch::port_is_present(std::string port) {
	std::vector<std::string>::iterator port_iter;
	for (port_iter = this->port_list.begin();
			port_iter != this->port_list.end();
			port_iter++)
	{
		const char *string1 = port.c_str();
		const char *string2 = port_iter->c_str();
		if ( strcmp(string1, string2) == 0 )
			return true;
	}
return false;
}

std::string xdpd::openflow_switch::num_to_port(uint64_t num) {
	if (num <= this->port_list.size() )
		return this->port_list[num-1];
	else
		return "NULL";
}

uint16_t xdpd::openflow_switch::port_to_num(std::string port_name) {

	std::vector<std::string>::iterator port_iter;
	uint16_t count = 0;
	for (port_iter = this->port_list.begin();
			port_iter != this->port_list.end();
			port_iter++)
	{
		count++;
		if (port_name.compare(port_iter->c_str())==0)
			return count;
	}
	return 0;
}
