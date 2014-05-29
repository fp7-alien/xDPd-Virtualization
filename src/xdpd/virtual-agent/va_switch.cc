/*
 * va_switch.cc
 *
 *  @author Daniel Depaoli <daniel.depaoli (at) create-net.org>
 */

#include "va_switch.h"
#include "virtualagent.h"
#include "flowspace.h"
#include <bitset>


va_switch::va_switch() {
	// TODO Auto-generated constructor stub
}

va_switch::va_switch(std::string dp_name ,uint64_t dp_id)
{
	this->dp_id = dp_id;
	this->dp_name = dp_name;
}



va_switch::~va_switch() {
	// TODO Auto-generated destructor stub
}


slice* va_switch::select_slice(cofctl* controller) {

	std::map<std::string, cofctl*>::iterator iter;

	//iterate through controller map
	for (iter = this->controller_map.begin();
			iter != this->controller_map.end();
			iter ++)
	{
		if (iter->second == controller)
			return this->get_slice(iter->first);
	}

	return NULL;

}

slice* va_switch::get_slice(std::string name) {

	slice* _slice;
	for (std::list<slice*>::iterator it = this->slice_list.begin();
			it != this->slice_list.end();
			it++)
	{
		_slice = *it;
		if (strcmp(_slice->name.c_str(), name.c_str()) == 0)
			return _slice;
	}

	return NULL;

}


bool va_switch::check_match(const of1x_packet_matches_t pkt, std::list<flowspace_match_t*> it) {

	std::list<flowspace_match_t*>::iterator _match;		// single match
	flowspace_match_t* tempMatch;

	for (_match = it.begin();
			_match != it.end();
			_match ++)
	{
		tempMatch = *_match;
		if (!va_switch::compare_match_flow(&pkt, tempMatch) )
		{
			return false;
		}

	}
		return true;
}


bool va_switch::compare_match_flow(const of1x_packet_matches_t* pkt,
		flowspace_match_t* it) {
	if(!it)
		return false;
	switch(it->type){
		//Phy
		case FLOWSPACE_MATCH_IN_PORT: return __utern_compare32(it->value,pkt->port_in);
		case FLOWSPACE_MATCH_IN_PHY_PORT: if(!pkt->port_in) return false; //According to spec
					return __utern_compare32(it->value,pkt->phy_port_in);
		//Metadata
	  	case FLOWSPACE_MATCH_METADATA: return __utern_compare64(it->value,pkt->metadata);

		//802
   		case FLOWSPACE_MATCH_ETH_DST:  return __utern_compare64(it->value,pkt->eth_dst);
   		case FLOWSPACE_MATCH_ETH_SRC:  return __utern_compare64(it->value,pkt->eth_src);
   		case FLOWSPACE_MATCH_ETH_TYPE: return __utern_compare16(it->value,pkt->eth_type);

		//802.1q
   		case FLOWSPACE_MATCH_VLAN_VID:
					return __utern_compare16(it->value,pkt->vlan_vid);
   		case FLOWSPACE_MATCH_VLAN_PCP: if(!pkt->has_vlan) return false;
					return __utern_compare8(it->value,pkt->vlan_pcp);

		//MPLS
   		case FLOWSPACE_MATCH_MPLS_LABEL: if(!(pkt->eth_type == OF1X_ETH_TYPE_MPLS_UNICAST || pkt->eth_type == OF1X_ETH_TYPE_MPLS_MULTICAST )) return false;
					return __utern_compare32(it->value,pkt->mpls_label);
   		case FLOWSPACE_MATCH_MPLS_TC: if(!(pkt->eth_type == OF1X_ETH_TYPE_MPLS_UNICAST || pkt->eth_type == OF1X_ETH_TYPE_MPLS_MULTICAST )) return false;
					return __utern_compare8(it->value,pkt->mpls_tc);
   		case FLOWSPACE_MATCH_MPLS_BOS: if(!(pkt->eth_type == OF1X_ETH_TYPE_MPLS_UNICAST || pkt->eth_type == OF1X_ETH_TYPE_MPLS_MULTICAST )) return false;
					return __utern_compare8(it->value,pkt->mpls_bos);

		//ARP
   		case FLOWSPACE_MATCH_ARP_OP: if(!(pkt->eth_type == OF1X_ETH_TYPE_ARP)) return false;
   					return __utern_compare16(it->value,pkt->arp_opcode);
   		case FLOWSPACE_MATCH_ARP_SHA: if(!(pkt->eth_type == OF1X_ETH_TYPE_ARP)) return false;
   					return __utern_compare64(it->value,pkt->arp_sha);
   		case FLOWSPACE_MATCH_ARP_SPA: if(!(pkt->eth_type == OF1X_ETH_TYPE_ARP)) return false;
					return __utern_compare32(it->value, pkt->arp_spa);
   		case FLOWSPACE_MATCH_ARP_THA: if(!(pkt->eth_type == OF1X_ETH_TYPE_ARP)) return false;
   					return __utern_compare64(it->value,pkt->arp_tha);
   		case FLOWSPACE_MATCH_ARP_TPA: if(!(pkt->eth_type == OF1X_ETH_TYPE_ARP)) return false;
					return __utern_compare32(it->value, pkt->arp_tpa);

		//NW (OF1.0 only)
   		case FLOWSPACE_MATCH_NW_PROTO: if(!(pkt->eth_type == OF1X_ETH_TYPE_IPV4 || pkt->eth_type == OF1X_ETH_TYPE_IPV6 || pkt->eth_type == OF1X_ETH_TYPE_ARP || (pkt->eth_type == OF1X_ETH_TYPE_PPPOE_SESSION && (pkt->ppp_proto == OF1X_PPP_PROTO_IP4 || pkt->ppp_proto == OF1X_PPP_PROTO_IP6) ))) return false;
					if(pkt->eth_type == OF1X_ETH_TYPE_ARP)
						return __utern_compare8(it->value,pkt->arp_opcode);
					else
						return __utern_compare8(it->value,pkt->ip_proto);

   		case FLOWSPACE_MATCH_NW_SRC:	if((pkt->eth_type == OF1X_ETH_TYPE_IPV4 || (pkt->eth_type == OF1X_ETH_TYPE_PPPOE_SESSION && pkt->ppp_proto == OF1X_PPP_PROTO_IP4 )))
						return __utern_compare32(it->value, pkt->ipv4_src);
					if(pkt->eth_type == OF1X_ETH_TYPE_ARP)
						return __utern_compare32(it->value, pkt->arp_spa);
					return false;
   		case FLOWSPACE_MATCH_NW_DST:	if((pkt->eth_type == OF1X_ETH_TYPE_IPV4 ||(pkt->eth_type == OF1X_ETH_TYPE_PPPOE_SESSION && pkt->ppp_proto == OF1X_PPP_PROTO_IP4 )))
						return __utern_compare32(it->value, pkt->ipv4_dst);
					if(pkt->eth_type == OF1X_ETH_TYPE_ARP)
						return __utern_compare32(it->value, pkt->arp_tpa);
					return false;
		//IP
   		case FLOWSPACE_MATCH_IP_PROTO: if(!(pkt->eth_type == OF1X_ETH_TYPE_IPV4 || pkt->eth_type == OF1X_ETH_TYPE_IPV6 || (pkt->eth_type == OF1X_ETH_TYPE_PPPOE_SESSION && (pkt->ppp_proto == OF1X_PPP_PROTO_IP4 || pkt->ppp_proto == OF1X_PPP_PROTO_IP6) ))) return false;
					return __utern_compare8(it->value,pkt->ip_proto);
		case FLOWSPACE_MATCH_IP_ECN: if(!(pkt->eth_type == OF1X_ETH_TYPE_IPV4 || pkt->eth_type == OF1X_ETH_TYPE_IPV6 || (pkt->eth_type == OF1X_ETH_TYPE_PPPOE_SESSION && pkt->ppp_proto == OF1X_PPP_PROTO_IP4 ))) return false; //NOTE OF1X_PPP_PROTO_IP6
					return __utern_compare8(it->value,pkt->ip_ecn);

		case FLOWSPACE_MATCH_IP_DSCP: if(!(pkt->eth_type == OF1X_ETH_TYPE_IPV4 || pkt->eth_type == OF1X_ETH_TYPE_IPV6 || (pkt->eth_type == OF1X_ETH_TYPE_PPPOE_SESSION && pkt->ppp_proto == OF1X_PPP_PROTO_IP4 ))) return false; //NOTE OF1X_PPP_PROTO_IP6
					return __utern_compare8(it->value,pkt->ip_dscp);

		//IPv4
   		case FLOWSPACE_MATCH_IPV4_SRC: if(!(pkt->eth_type == OF1X_ETH_TYPE_IPV4 || (pkt->eth_type == OF1X_ETH_TYPE_PPPOE_SESSION && pkt->ppp_proto == OF1X_PPP_PROTO_IP4 ))) return false;
					return __utern_compare32(it->value, pkt->ipv4_src);
   		case FLOWSPACE_MATCH_IPV4_DST:if(!(pkt->eth_type == OF1X_ETH_TYPE_IPV4 ||(pkt->eth_type == OF1X_ETH_TYPE_PPPOE_SESSION && pkt->ppp_proto == OF1X_PPP_PROTO_IP4 ))) return false;
					return __utern_compare32(it->value, pkt->ipv4_dst);

		//TCP
   		case FLOWSPACE_MATCH_TCP_SRC:
   			if(!(pkt->ip_proto == OF1X_IP_PROTO_TCP)) return false;
					return __utern_compare16(it->value,pkt->tcp_src);
   		case FLOWSPACE_MATCH_TCP_DST:
   			if(!(pkt->ip_proto == OF1X_IP_PROTO_TCP)) return false;
					return __utern_compare16(it->value,pkt->tcp_dst);

		//UDP
   		case FLOWSPACE_MATCH_UDP_SRC: if(!(pkt->ip_proto == OF1X_IP_PROTO_UDP)) return false;
					return __utern_compare16(it->value,pkt->udp_src);
   		case FLOWSPACE_MATCH_UDP_DST: if(!(pkt->ip_proto == OF1X_IP_PROTO_UDP)) return false;
					return __utern_compare16(it->value,pkt->udp_dst);
		//SCTP
   		case FLOWSPACE_MATCH_SCTP_SRC: if(!(pkt->ip_proto == OF1X_IP_PROTO_SCTP)) return false;
					return __utern_compare16(it->value,pkt->tcp_src);
   		case FLOWSPACE_MATCH_SCTP_DST: if(!(pkt->ip_proto == OF1X_IP_PROTO_SCTP)) return false;
					return __utern_compare16(it->value,pkt->tcp_dst);

		//TP (OF1.0 only)
   		case FLOWSPACE_MATCH_TP_SRC: if((pkt->ip_proto == OF1X_IP_PROTO_TCP))
						return __utern_compare16(it->value,pkt->tcp_src);
   					if((pkt->ip_proto == OF1X_IP_PROTO_UDP))
						return __utern_compare16(it->value,pkt->udp_src);
					if((pkt->ip_proto == OF1X_IP_PROTO_ICMPV4))
						return __utern_compare16(it->value,pkt->icmpv4_type);
					return false;

   		case FLOWSPACE_MATCH_TP_DST: if((pkt->ip_proto == OF1X_IP_PROTO_TCP))
						return __utern_compare16(it->value,pkt->tcp_dst);
   					if((pkt->ip_proto == OF1X_IP_PROTO_UDP))
						return __utern_compare16(it->value,pkt->udp_dst);
					if((pkt->ip_proto == OF1X_IP_PROTO_ICMPV4))
						return __utern_compare16(it->value,pkt->icmpv4_code);
					return false;

		//ICMPv4
		case FLOWSPACE_MATCH_ICMPV4_TYPE: if(!(pkt->ip_proto == OF1X_IP_PROTO_ICMPV4)) return false;
					return __utern_compare8(it->value,pkt->icmpv4_type);
   		case FLOWSPACE_MATCH_ICMPV4_CODE: if(!(pkt->ip_proto == OF1X_IP_PROTO_ICMPV4)) return false;
					return __utern_compare8(it->value,pkt->icmpv4_code);

		//IPv6
		case FLOWSPACE_MATCH_IPV6_SRC: if(!(pkt->eth_type == OF1X_ETH_TYPE_IPV6 || (pkt->eth_type == OF1X_ETH_TYPE_PPPOE_SESSION && pkt->ppp_proto == OF1X_PPP_PROTO_IP6 ))) return false;
					return __utern_compare128(it->value, pkt->ipv6_src);
		case FLOWSPACE_MATCH_IPV6_DST: if(!(pkt->eth_type == OF1X_ETH_TYPE_IPV6 || (pkt->eth_type == OF1X_ETH_TYPE_PPPOE_SESSION && pkt->ppp_proto == OF1X_PPP_PROTO_IP6 ))) return false;
					return __utern_compare128(it->value, pkt->ipv6_dst);
		case FLOWSPACE_MATCH_IPV6_FLABEL: if(!(pkt->eth_type == OF1X_ETH_TYPE_IPV6 || (pkt->eth_type == OF1X_ETH_TYPE_PPPOE_SESSION && pkt->ppp_proto == OF1X_PPP_PROTO_IP6 ))) return false;
					return __utern_compare64(it->value, pkt->ipv6_flabel);
		case FLOWSPACE_MATCH_IPV6_ND_TARGET: if(!(pkt->ip_proto == OF1X_IP_PROTO_ICMPV6)) return false;
					return __utern_compare128(it->value,pkt->ipv6_nd_target);
		case FLOWSPACE_MATCH_IPV6_ND_SLL: if(!(pkt->ip_proto == OF1X_IP_PROTO_ICMPV6 && pkt->ipv6_nd_sll)) return false; //NOTE OPTION SLL active
					return __utern_compare64(it->value, pkt->ipv6_nd_sll);
		case FLOWSPACE_MATCH_IPV6_ND_TLL: if(!(pkt->ip_proto == OF1X_IP_PROTO_ICMPV6 && pkt->ipv6_nd_tll)) return false; //NOTE OPTION TLL active
					return __utern_compare64(it->value, pkt->ipv6_nd_tll);
		case FLOWSPACE_MATCH_IPV6_EXTHDR: //TODO not yet implemented.
			return false;
			break;

		//ICMPv6
		case FLOWSPACE_MATCH_ICMPV6_TYPE: if(!(pkt->ip_proto == OF1X_IP_PROTO_ICMPV6)) return false;
					return __utern_compare64(it->value, pkt->icmpv6_type);
		case FLOWSPACE_MATCH_ICMPV6_CODE: if(!(pkt->ip_proto == OF1X_IP_PROTO_ICMPV6 )) return false;
					return __utern_compare64(it->value, pkt->icmpv6_code);

		//PPPoE related extensions
   		case FLOWSPACE_MATCH_PPPOE_CODE: if(!(pkt->eth_type == OF1X_ETH_TYPE_PPPOE_DISCOVERY || pkt->eth_type == OF1X_ETH_TYPE_PPPOE_SESSION )) return false;
						return __utern_compare8(it->value,pkt->pppoe_code);
   		case FLOWSPACE_MATCH_PPPOE_TYPE: if(!(pkt->eth_type == OF1X_ETH_TYPE_PPPOE_DISCOVERY || pkt->eth_type == OF1X_ETH_TYPE_PPPOE_SESSION )) return false;
						return __utern_compare8(it->value,pkt->pppoe_type);
   		case FLOWSPACE_MATCH_PPPOE_SID: if(!(pkt->eth_type == OF1X_ETH_TYPE_PPPOE_DISCOVERY || pkt->eth_type == OF1X_ETH_TYPE_PPPOE_SESSION )) return false;
						return __utern_compare16(it->value,pkt->pppoe_sid);

		//PPP
   		case FLOWSPACE_MATCH_PPP_PROT: if(!(pkt->eth_type == OF1X_ETH_TYPE_PPPOE_SESSION )) return false;
						return __utern_compare16(it->value,pkt->ppp_proto);

		//PBB
   		case FLOWSPACE_MATCH_PBB_ISID: if(pkt->eth_type == OF1X_ETH_TYPE_PBB) return false;
						return __utern_compare32(it->value,pkt->pbb_isid);
	 	//TUNNEL id
   		case FLOWSPACE_MATCH_TUNNEL_ID: return __utern_compare64(it->value,pkt->tunnel_id);

		//GTP
   		case FLOWSPACE_MATCH_GTP_MSG_TYPE: if (!(pkt->ip_proto == OF1X_IP_PROTO_UDP || pkt->udp_dst == OF1X_UDP_DST_PORT_GTPU)) return false;
   						return __utern_compare8(it->value,pkt->gtp_msg_type);
   		case FLOWSPACE_MATCH_GTP_TEID: if (!(pkt->ip_proto == OF1X_IP_PROTO_UDP || pkt->udp_dst == OF1X_UDP_DST_PORT_GTPU)) return false;
   						return __utern_compare32(it->value,pkt->gtp_teid);
		case FLOWSPACE_MATCH_MAX:
				break;
	}

	assert(0);
	return NULL;
}


