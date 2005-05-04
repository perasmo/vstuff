#ifndef _INTF_H
#define _INTF_H

#include <netinet/in.h>
#include <lapd.h>

#include "list.h"
#include "channel.h"
#include "timer.h"
#include "dlc.h"
#include "callref.h"
#include "call.h"
#include "global.h"

#define report_intf(intf, lvl, format, arg...)		\
	(intf)->lib->report((lvl), format, ## arg)

enum q931_interface_type
{
	Q931_INTF_TYPE_BRA_POINT_TO_POINT,
	Q931_INTF_TYPE_BRA_MULTIPOINT,
	Q931_INTF_TYPE_PRA,
};

struct q931_call;
struct q931_interface
{
	struct list_head node;

	struct q931_lib *lib;

	char *name;

	enum q931_interface_type type;
	enum lapd_role role;

	int master_socket;	// Multipoint master_socket
	struct q931_dlc bc_dlc;	// Broadcast DLC for multipoint interfaces
	struct q931_dlc dlc; 

	q931_callref next_call_reference;
	int call_reference_len;

	int ncalls;
	// TODO: Use a HASH for improved scalability
	struct list_head calls;

	struct q931_channel channels[32];
	int n_channels;

	struct q931_global_call global_call;

	int tones_option;

	__u8 sendbuf[260]; // FIXME (size should be N202-dependent)

	longtime_t T301;
	longtime_t T302;
	longtime_t T303;
	longtime_t T304;
	longtime_t T305;
	longtime_t T306;
	longtime_t T307;
	longtime_t T308;
	longtime_t T309;
	longtime_t T310;
	longtime_t T312;
	longtime_t T313;
	longtime_t T314;
	longtime_t T316;
	longtime_t T317;
	longtime_t T318;
	longtime_t T319;
	longtime_t T320;
	longtime_t T321;
	longtime_t T322;
};

inline static void q931_intf_add_call(
	struct q931_interface *intf,
	struct q931_call *call)
{
	list_add_tail(&call->calls_node, &intf->calls);
	intf->ncalls++;
}

inline static void q931_intf_del_call(
	struct q931_call *call)
{
	list_del(&call->calls_node);
	
	call->intf->ncalls--;
}

struct q931_interface *q931_open_interface(
	struct q931_lib *lib,
	const char *name);
void q931_close_interface(struct q931_interface *intf);

q931_callref q931_alloc_call_reference(struct q931_interface *intf);

#endif
