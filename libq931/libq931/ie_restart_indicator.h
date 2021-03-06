/*
 * vISDN DSSS-1/q.931 signalling library
 *
 * Copyright (C) 2004-2005 Daniele Orlandi
 *
 * Authors: Daniele "Vihai" Orlandi <daniele@orlandi.com>
 *
 * This program is free software and may be modified and distributed
 * under the terms and conditions of the GNU General Public License.
 *
 */

#ifndef _LIBQ931_IE_RESTART_INDICATOR_H
#define _LIBQ931_IE_RESTART_INDICATOR_H

#include <libq931/ie.h>

/*********************** Restart Indicator *************************/

enum q931_ie_restart_indicator_restart_class
{
	Q931_IE_RI_C_INDICATED		= 0x0,
	Q931_IE_RI_C_SINGLE_INTERFACE	= 0x6,
	Q931_IE_RI_C_ALL_INTERFACES	= 0x7
};

struct q931_ie_restart_indicator
{
	struct q931_ie ie;

	enum q931_ie_restart_indicator_restart_class restart_class;
};

struct q931_ie_restart_indicator *q931_ie_restart_indicator_alloc(void);
struct q931_ie *q931_ie_restart_indicator_alloc_abstract(void);

int q931_ie_restart_indicator_read_from_buf(
	struct q931_ie *abstract_ie,
	void *buf,
	int len,
	void (*report_func)(int level, const char *format, ...),
	struct q931_interface *intf);

int q931_ie_restart_indicator_write_to_buf(
	const struct q931_ie *generic_ie,
	void *buf,
	int max_size);

void q931_ie_restart_indicator_dump(
	const struct q931_ie *ie,
	void (*report)(int level, const char *format, ...),
	const char *prefix);

#ifdef Q931_PRIVATE

struct q931_ie_restart_indicator_onwire_3
{
	union { struct {
#if __BYTE_ORDER == __BIG_ENDIAN
	__u8 ext:1;
	__u8 :4;
	__u8 restart_class:3;
#else
	__u8 restart_class:3;
	__u8 :4;
	__u8 ext:1;
#endif
	}; __u8 raw; };
} __attribute__ ((__packed__));

void q931_ie_restart_indicator_register(
	const struct q931_ie_class *ie_class);

#endif
#endif
