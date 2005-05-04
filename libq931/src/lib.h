#ifndef _LIB_H
#define _LIB_H

#include "dlc.h"
#include "list.h"
#include "logging.h"
#include "call.h"
#include "intf.h"

enum q931_mode
{
	UNKNOWN_MODE,
	CIRCUIT_MODE,
	PACKET_MODE
};

struct q931_lib
{
	struct list_head timers;
	struct list_head intfs;

	void (*report)(int level, const char *format, ...);
};

static inline void q931_set_logger_func(
	struct q931_lib *lib,
	void (*report)(int level, const char *format, ...))
{
	lib->report = report;
}

struct q931_lib *q931_init();
void q931_leave(struct q931_lib *lib);
void q931_receive(struct q931_dlc *dlc);

#ifdef Q931_PRIVATE

void q931_dl_establish_confirm(struct q931_dlc *dlc);

#endif

#endif