/*
 * vg.cc
 *
 *  Created on: Jun 16, 2014
 *      Author: daniel
 */

#include "../src/xdpd/virtual-agent/jsonRpc/stubclient.h"
#include <jsonrpc/rpc.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <stdlib.h>

using namespace jsonrpc;
using namespace std;

void help(){
	printf("./vg [number]\n");
	printf("Number should be:\n");
	printf("\t 1 = add_slice\n");
	printf("\t 2 = add_flowspace\n");
	printf("\t 3 = list_slices\n");
	printf("\t 4 = list_flowspace\n");
	printf("\t 5 = delete_slice\n");
	printf("\t 6 = list_datapaths\n");
	printf("\t 7 = list_slice info\n");
	printf("\t 8 = list_flowspace\n");
}

int main(int argc, char **argv){


	std::string ip = "127.0.0.1";
	std::stringstream sport;
	int port = 6600;
	sport << port;
	stubClient client(new HttpClient(ip+":"+sport.str()));

	int result = -1;

	try{

		if (argc != 2)
		{
			help();
			return 0;
		}

		switch (atoi(argv[1]))
		{
		case 1: {
			/**
			 * Add slice
			 */
			printf("Add slice\n");

			std::string slice_ip = "127.0.0.1";
			std::string slice_name = "fromVG";
			int slice_port = 6611;

			//datapaths info
			Json::Value port_list_dp1;
			port_list_dp1.append("tap1");
			port_list_dp1.append("tap2");

			Json::Value datapaths_list;
			datapaths_list["dp1"] = port_list_dp1;

			result = client.addSlice(datapaths_list,slice_ip.c_str(),slice_name.c_str(), "1.0", slice_port);
			printf("Result %i\n", result);
		}
			break;

		case 2: {
			printf("Add flowspace\n");
			Json::Value matches;
			matches["vlan_vid"] = 20;
			Json::Value dpts;
			dpts.append("dp1");
			std::string flowspaceName = "fromVGfs";
			int priority = 15;
			std::string sliceName = "fromVG";
			result = client.addFlowspace(dpts,matches,flowspaceName,priority,sliceName);
			printf("Result %i\n", result);
			}
				break;
		case 3:
			std::cout << "lista slice = " << client.listSlice();
			break;
		case 4:
			std::cout << "lista flowspace = " << client.listFlowspaces();
				break;

		case 5:
			printf("Remove slice\n");
			result = client.deleteSlice("alice");
			break;

		case 6:
			std::cout << "lista datapath = " << client.listDatapaths();
			break;

		case 7:
			std::cout << client.listSliceInfo("tom");
			break;

		default: help();
				break;
		}

		/**
		 * List datapaths
		 */



		/**
		 * Add flowspace
		 */


		/**
		 * Delete slice
		 */


		/**
		 * List Flowspaces
		 */



	}
	catch (JsonRpcException e)
	{
		printf("Errore %s\n",e.what());
	}

	return 0;
}
