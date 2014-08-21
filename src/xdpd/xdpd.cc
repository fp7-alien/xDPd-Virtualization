/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <rofl/platform/unix/cunixenv.h>
#include <rofl/datapath/afa/fwd_module.h>
#include <rofl/common/utils/c_logger.h>
#include "management/switch_manager.h"
#include "management/port_manager.h"
#include "management/plugin_manager.h"

//Virtual agent
//Daniel Depaoli <daniel.depaoli (at) create-net.org>
//#include "virtual-agent/slice.h"
//#include "virtual-agent/structures.h"
#include "virtual-agent/virtualagent.h"
#include "virtual-agent/virtualizationgateway.h"

using namespace rofl;
using namespace xdpd;

extern int optind;


//TODO: Redirect C loggers to the output log
#define XDPD_LOG_FILE "xdpd.log"

//Handler to stop ciosrv
void interrupt_handler(int dummy=0) {
	//Only stop ciosrv 
	ciosrv::stop();
}

//Prints version and build numbers and exits
void dump_version(){
	//Print version and exit
	ROFL_INFO("The eXtensible OpenFlow Datapath daemon (xDPd)\n");
	ROFL_INFO("Version: with Virtualization Agent\n");

#ifdef XDPD_BUILD
	ROFL_INFO("Build: %s\n",XDPD_BUILD);
	ROFL_INFO("Compiled in branch: %s\n",XDPD_BRANCH);
	ROFL_INFO("%s\n",XDPD_DESCRIBE);
#endif
	ROFL_INFO("\n[Libraries: ROFL]\n");
	ROFL_INFO("ROFL version: %s\n",ROFL_VERSION);
	ROFL_INFO("ROFL build: %s\n",ROFL_BUILD_NUM);
	ROFL_INFO("ROFL compiled in branch: %s\n",ROFL_BUILD_BRANCH);
	ROFL_INFO("%s\n",ROFL_BUILD_DESCRIBE);

	exit(EXIT_SUCCESS);

}

/*
 * xDPd Main routine
 */
int main(int argc, char** argv){


	//Check for root privileges 
	if(geteuid() != 0){
		ROFL_ERR("ERROR: Root permissions are required to run %s\n",argv[0]);	
		exit(EXIT_FAILURE);	
	}

	//Capture control+C
	signal(SIGINT, interrupt_handler);

#if DEBUG && VERBOSE_DEBUG
	//Set verbose debug if necessary
	rofl_set_logging_level(/*cn,*/ DBG_VERBOSE_LEVEL);
#endif
	
	/* Add additional arguments */
	char s_dbg[32];
	memset(s_dbg, 0, sizeof(s_dbg));
	snprintf(s_dbg, sizeof(s_dbg)-1, "%d", (int)csyslog::DBG);

	bool vg = false;
	{ //Make valgrind happy
		cunixenv env_parser(argc, argv);
		
		/* update defaults */
		env_parser.update_default_option("logfile", XDPD_LOG_FILE);
		env_parser.add_option(coption(true, NO_ARGUMENT, 'v', "version", "Retrieve xDPd version and exit", std::string("")));

		/**
		 * If set -n it works like normal xdpd
		 */
		env_parser.add_option(coption(true, NO_ARGUMENT, 'n', "no-virtualization", "Deactivate virtualization", std::string("") ));

		/**
		 *
		 */
		env_parser.add_option(coption(true, NO_ARGUMENT, 'g', "virtual-gateway", "Activate Virtualization Gateway listening", std::string("") ));

		//Parse
		env_parser.parse_args();

		if (env_parser.is_arg_set("version")) {
			dump_version();
		}

		if (not env_parser.is_arg_set("daemonize")) {
			// only do this in non
			std::string ident(XDPD_LOG_FILE);

			csyslog::initlog(csyslog::LOGTYPE_FILE,
					static_cast<csyslog::DebugLevel>(atoi(env_parser.get_arg("debug").c_str())), // todo needs checking
					env_parser.get_arg("logfile"),
					ident.c_str());
		}

		/**
		 * Set virtualization active true/false
		 * It's not possible changing this in runtime
		 */
		if (env_parser.is_arg_set("no-virtualization"))
			virtual_agent::active_va(false);
		else
		{
			virtual_agent::active_va(true);
			if (env_parser.is_arg_set("virtual-gateway"))
				vg = true;
		}
	}
	//Forwarding module initialization
	if(fwd_module_init() != AFA_SUCCESS){
		ROFL_INFO("Init driver failed\n");	
		exit(-1);
	}
	//Init the ciosrv.
	ciosrv::init();

	//Load plugins
	optind=0;
	plugin_manager::init(argc, argv);
	if (virtual_agent::is_active() && vg)
	{
		virtualization_gateway*  vg= new virtualization_gateway(6600);
		vg->nothing();
	}
	virtual_agent::write_cfg("real-time-config.cfg");
	//ciosrv run. Only will stop in Ctrl+C
	ciosrv::run();

	//Printing nice trace
	ROFL_INFO("\nCleaning the house...\n");	

	//Destroy all state
	switch_manager::destroy_all_switches();

	//Let plugin manager destroy all registered plugins
	plugin_manager::destroy();
	
	//ciosrv destroy
	ciosrv::destroy();

	//Call fwd_module to shutdown
	fwd_module_destroy();
	
	ROFL_INFO("House cleaned!\nGoodbye\n");
	
	exit(EXIT_SUCCESS);
}




