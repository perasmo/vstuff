#ifndef _DLC_H
#define _DLC_H

#define report_dlc(dlc, lvl, format, arg...)				\
	(dlc)->interface->lib->report((lvl), format, ## arg)

enum q931_dlc_status
{
	DLC_DISCONNECTED,
	DLC_AWAITING_CONNECTION,
	DLC_AWAITING_DISCONNECTION,
	DLC_CONNECTED,
};

struct q931_interface;
struct q931_dlc
{
	struct q931_lib *lib;

	int socket;
	int poll_id;
	struct q931_interface *interface;
	enum q931_dlc_status status;
	int tei;
};

static inline void q931_init_dlc(
	struct q931_dlc *dlc,
	struct q931_interface *interface,
	int socket)
{
	dlc->socket = socket;
	dlc->interface = interface;
}

#endif