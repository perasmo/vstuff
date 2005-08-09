/*
 *
 *
 * Copyright (C) 2005 Daniele Orlandi
 *
 * Daniele "Vihai" Orlandi <daniele@orlandi.com> 
 *
 * This program is free software and may be modified and
 * distributed under the terms of the GNU Public License.
 *
 */

#ifndef _HFC_ST_PORT_H
#define _HFC_ST_PORT_H

#include <visdn.h>

#include "chan.h"
//#include "fifo.h"

#define to_st_port(port) container_of(port, struct hfc_st_port, visdn_port)

#ifdef DEBUG
#define hfc_debug_port(port, dbglevel, format, arg...)		\
	if (debug_level >= dbglevel)				\
		printk(KERN_DEBUG hfc_DRIVER_PREFIX		\
			"%s-%s:"				\
			"st%d:"					\
			format,					\
			(port)->card->pcidev->dev.bus->name,	\
			(port)->card->pcidev->dev.bus_id,	\
			(port)->id,				\
			## arg)
#else
#define hfc_debug_port(port, dbglevel, format, arg...) do {} while (0)
#endif

#define hfc_msg_port(port, level, format, arg...)		\
	printk(level hfc_DRIVER_PREFIX				\
		"%s-%s:"					\
		"st%d:"						\
		format,						\
		(port)->card->pcidev->dev.bus->name,		\
		(port)->card->pcidev->dev.bus_id,		\
		(port)->id,					\
		## arg)

#define D 0
#define B1 1
#define B2 2
#define E 3
#define SQ 4

struct hfc_st_port
{
	struct hfc_card *card;

	int id;

//	int enabled;
	u8 l1_state;
	int clock_delay;
	int sampling_comp;

	struct hfc_chan_duplex chans[5];

	// This struct contains a copy of some registers whose bits may be
	// changed independently.
	struct
	{
		u8 st_ctrl_0;
		u8 st_ctrl_2;
	} regs;

	struct visdn_port visdn_port;
};

extern struct visdn_port_ops hfc_st_port_ops;

void hfc_st_port_check_l1_up(struct hfc_st_port *port);
void hfc_st_port__do_set_role(struct hfc_st_port *port, int nt_mode);

#endif