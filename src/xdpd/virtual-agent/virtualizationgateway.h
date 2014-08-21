/*
 * virtualizationgateway.h
 *
 *  Created on: Jun 16, 2014
 *      Author: daniel
 */

#ifndef VIRTUALIZATIONGATEWAY_H_
#define VIRTUALIZATIONGATEWAY_H_

#include "jsonRpc/abstractstubserver.h"

class virtualization_gateway: public AbstractstubServer {
public:
	int nothing();
	virtualization_gateway(int port);
	virtual ~virtualization_gateway();

    virtual int addFlowspace(const Json::Value& listDatapaths, const Json::Value& matches, const std::string& name, const int& priority, const std::string& slice);
    virtual int addSlice(const Json::Value& datapaths, const std::string& ip, const std::string& name, const std::string& ofversion, const int& port);
    virtual int deleteSlice(const std::string& name);
    virtual Json::Value listDatapaths();
    virtual Json::Value listFlowspaceInfo(const std::string& name);
    virtual Json::Value listFlowspaces();
    virtual Json::Value listSlice();
    virtual Json::Value listSliceInfo(const std::string& name);
    virtual int removeFlowspace(const std::string& name);

private:

    int parse_priority(int priority);
    bool check_datapaths(Json::Value datapaths, std::string name);
};

#endif /* VIRTUALIZATIONGATEWAY_H_ */
