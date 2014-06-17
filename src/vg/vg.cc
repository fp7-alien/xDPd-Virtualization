/*
 * vg.cc
 *
 *  Created on: Jun 16, 2014
 *      Author: daniel
 */



#include "../xdpd/virtual-agent/jsonRpc/stubclient.h"
#include <jsonrpc/rpc.h>
#include <iostream>
#include <stdio.h>
#include <string>

using namespace jsonrpc;
using namespace std;

int main(){
	std::string ip = "127.0.0.1";
	std::stringstream sport;
	int port = 6600;
	sport << port;
	stubClient client(new HttpClient(ip+":"+sport.str()));

	int result = -1;

	try{
//		printf("Invio hello\n");
//		cout << client.sayHello("nome cognome") << endl;
//		client.notifyServer();

		/**
		 * List datapaths
		 */
//		Json::Value valore = client.listDatapaths();
//		Json::FastWriter writer;
//		printf("%s\n",writer.write(valore).c_str());



		/**
		 * Add slice
		 */
		Json::Value datapaths_list;
		Json::Value port_list_dp1;
		port_list_dp1.append("port1_1");
		port_list_dp1.append("port2_1");
		port_list_dp1.append("port3_1");
		datapaths_list["dp1"] = port_list_dp1;

//		Json::Value port_list_dp2;
//		port_list_dp2.append("port1_2");
//		port_list_dp2.append("port2_2");
//		port_list_dp2.append("port3_2");
//		datapaths_list["dp2"] = port_list_dp2;

		std::string slice_ip = "127.0.0.1";
		std::string slice_name = "daniel";
		int slice_port = 6611;
		result = client.addSlice(datapaths_list,slice_ip.c_str(),slice_name.c_str(), slice_port);
		printf("Result %i\n", result);


		/**
		 * Add flowspace
		 */
		Json::Value matches;
		matches["vlan_vid"] = 10;
		Json::Value dpts;
		std::string flowspaceName = "FS1";
		int priority = 15;
		std::string sliceName = "daniel";
		result = client.addFlowspace(dpts,matches,flowspaceName,priority,sliceName);
		printf("Result %i\n", result);


		/**
		 * Delete slice
		 */
//		std::string slice_name = "alice";
//		client.deleteSlice(slice_name);


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
