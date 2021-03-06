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

#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <time.h>

#define Q931_PRIVATE

#include <libq931/lib.h>
#include <libq931/ie_datetime.h>

static const struct q931_ie_class *my_class;

void q931_ie_datetime_register(
	const struct q931_ie_class *ie_class)
{
	my_class = ie_class;
}

struct q931_ie_datetime *q931_ie_datetime_alloc(void)
{
	struct q931_ie_datetime *ie;
	ie = malloc(sizeof(*ie));
	assert(ie);

	memset(ie, 0, sizeof(*ie));

	ie->ie.refcnt = 1;
	ie->ie.cls = my_class;

	return ie;
}

struct q931_ie *q931_ie_datetime_alloc_abstract(void)
{
	return &q931_ie_datetime_alloc()->ie;
}

int q931_ie_datetime_read_from_buf(
	struct q931_ie *abstract_ie,
	void *buf,
	int len,
	void (*report_func)(int level, const char *format, ...),
	struct q931_interface *intf)
{
	assert(abstract_ie->cls == my_class);

	struct q931_ie_datetime *ie =
		container_of(abstract_ie,
			struct q931_ie_datetime, ie);

	struct q931_ie_datetime_onwire_3 *oct_3 =
		(struct q931_ie_datetime_onwire_3 *)
		(buf + 0);

	if (len < 2) {
		report_ie(abstract_ie, LOG_ERR, "IE len < 2\n");
		return FALSE;
	}

	if (len > 8) {
		report_ie(abstract_ie, LOG_ERR, "IE len > 8\n");
		return FALSE;
	}

	struct tm tm = { 0, 0, 0, 1, 0, 0, 0, 0, 0 };

	/* Year */
	if (oct_3->year > 99) {
		report_ie(abstract_ie, LOG_ERR,
			"Invalid year %d > 99\n", oct_3->year);
		return FALSE;
	}

	tm.tm_year = oct_3->year;

	/* Month */
	if (len >= 4) {
		if (oct_3->month < 1 || oct_3->month > 12) {
			report_ie(abstract_ie, LOG_ERR,
				"Invalid month %d\n", oct_3->month);
			return FALSE;
		}

		tm.tm_mon = oct_3->month - 1;
	}

	/* Day */
	if (len >= 5) {
		if (oct_3->day < 1 || oct_3->day > 31) {
			report_ie(abstract_ie, LOG_ERR,
				"Invalid day %d\n", oct_3->day);
			return FALSE;
		}

		tm.tm_mday = oct_3->day;
	}

	/* Hour */
	if (len >= 6) {
		if (oct_3->hour > 24) {
			report_ie(abstract_ie, LOG_ERR,
				"Invalid hour %d\n", oct_3->hour);
			return FALSE;
		}

		tm.tm_hour = oct_3->hour;
	}

	/* Minute */
	if (len >= 7) {
		if (oct_3->minute > 59) {
			report_ie(abstract_ie, LOG_ERR,
				"Invalid minute %d\n", oct_3->minute);
			return FALSE;
		}

		tm.tm_min = oct_3->minute;
	}

	/* Second */
	if (len >= 8) {
		if (oct_3->second > 61) {
			report_ie(abstract_ie, LOG_ERR,
				"Invalid second %d\n", oct_3->second);
			return FALSE;
		}

		tm.tm_sec = oct_3->second;
	}

	ie->time = mktime(&tm);

	return TRUE;
}

int q931_ie_datetime_write_to_buf(
	const struct q931_ie *abstract_ie,
	void *buf,
	int max_size)
{
	int len = 0;
	struct q931_ie_datetime *ie =
		container_of(abstract_ie, struct q931_ie_datetime, ie);

	struct tm *tm;
	tm = localtime(&ie->time);

	struct q931_ie_datetime_onwire_3 *oct_3 =
		(struct q931_ie_datetime_onwire_3 *)
		(buf + len);

	oct_3->year = tm->tm_year % 100;
	oct_3->month = tm->tm_mon + 1;
	oct_3->day = tm->tm_mday;
	oct_3->hour = tm->tm_hour;
	oct_3->minute = tm->tm_min;
	oct_3->second = tm->tm_sec;

	len += sizeof(*oct_3);

	return len;
}

void q931_ie_datetime_dump(
	const struct q931_ie *abstract_ie,
	void (*report_func)(int level, const char *format, ...),
	const char *prefix)
{
	struct q931_ie_datetime *ie =
		container_of(abstract_ie, struct q931_ie_datetime, ie);

	report_ie_dump(abstract_ie,
		"%sDateTime = %s\n", prefix, ctime(&ie->time));
}
