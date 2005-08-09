/* visdn.c - pppd plugin to implement PPPovISDN protocol.
 *
 * Copyright 2000 Mitchell Blank Jr.
 * Based in part on work from Jens Axboe and Paul Mackerras.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  26 May 2003 dennis@yellowtuna.co.nz - modified for pppd 2.4.2
 */
#include "pppd.h"
#include "pathnames.h"
#include "fsm.h" /* Needed for lcp.h to include cleanly */
#include "lcp.h"
#include <sys/stat.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include "/root/isdn--devel/lapd/lapd_user.h"

static int visdn_accept = 0;
static bool llc_encaps = 0;
static bool vc_encaps = 0;
static int device_got_set = 0;
static int visdn_max_mtu, visdn_max_mru;

char pppd_version[] = VERSION;
extern int new_style_driver;	/* From sys-linux.c */

static int visdn_setdevname(char *cmd, char **argv, int doit);

static option_t visdn_options[] = {
	{ "device name", o_wild, (void *) &visdn_setdevname, "vISDN device name",
		OPT_DEVNAM | OPT_PRIVFIX | OPT_NOARG | OPT_A2STRVAL | OPT_STATIC, devnam},
	{ NULL }
};


struct channel visdn_channel;

/* returns:
 *  -1 if there's a problem with setting the device
 *   0 if we can't parse "cp" as a valid name of a device
 *   1 if "cp" is a reasonable thing to name a device
 * Note that we don't actually open the device at this point
 * We do need to fill in:
 *   devnam: a string representation of the device
 *   devstat: a stat structure of the device.  In this case
 *     we're not opening a device, so we just make sure
 *     to set up S_ISCHR(devstat.st_mode) != 1, so we
 *     don't get confused that we're on stdin.
 */
static int visdn_setdevname(char *cmd, char **argv, int doit)
{
	extern struct stat devstat;

	info("PPPovISDN visdn_setdevname %s", cmd);

	if (!strstr(cmd, "isdn"))
		return 0;

//	if (device_got_set)
//		return 0;

	strlcpy(devnam, cmd, sizeof(devnam));

	devstat.st_mode = S_IFSOCK;

	info("PPPovISDN visdn_setdevname - SUCCESS %s", cmd);

	device_got_set = 1;

	if (the_channel != &visdn_channel) {
		the_channel = &visdn_channel;
		modem = 0;
		lcp_wantoptions[0].neg_accompression = 0;
		lcp_allowoptions[0].neg_accompression = 0;
		lcp_wantoptions[0].neg_asyncmap = 0;
		lcp_allowoptions[0].neg_asyncmap = 0;
		lcp_wantoptions[0].neg_pcompression = 0;
	}

	return 1;
}

#define _PATH_VISDNOPT _ROOT_PATH "/etc/ppp/options."

static void visdn_process_extra_options(void)
{
	info("visdn_process_extra_options");

	char buf[256];

	snprintf(buf, 256, _PATH_VISDNOPT "%s",devnam);

	if(!options_from_file(buf, 0, 0, 1))
		exit(EXIT_OPTION_ERROR);
}

static void no_device_given_visdn(void)
{
	fatal("No device specified");
}


#define VISDN_SET_BEARER_PPP  12345678

static int visdn_connect(void)
{
	int fd;
	info("PPPovISDN - open device %s", ifname);

	if (!device_got_set)
		no_device_given_visdn();

	fd = socket(AF_LAPD, SOCK_DGRAM, 0);
	if (fd < 0)
		fatal("failed to create socket: %m");

	struct ifreq ifr;

	strlcpy(ifr.ifr_name, devnam , sizeof(ifr.ifr_name));

	if (ioctl(fd, VISDN_SET_BEARER_PPP, (caddr_t) &ifr) < 0)
		fatal("ioctl(VISDN_SET_BEARER_PPP): %m");

	strlcpy(ppp_devnam, devnam, sizeof(ppp_devnam));

	return fd;
}

static void post_open_setup_visdn(void)
{
		    /* NOTHING */
}

static void pre_close_restore_visdn(void)
{
		    /* NOTHING */
}

static void visdn_disconnect(void)
{
	/* NOTHING */
}

static void visdn_send_config(int mtu, u_int32_t asyncmap, int pcomp, int accomp)
{
	int sock;
	struct ifreq ifr;
/*
	if (mtu > visdn_max_mtu)
		error("Couldn't increase MTU to %d", mtu);
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
		fatal("Couldn't create IP socket: %m");
	strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	ifr.ifr_mtu = mtu;
	if (ioctl(sock, SIOCSIFMTU, (caddr_t) &ifr) < 0)
		fatal("ioctl(SIOCSIFMTU): %m");
	(void) close (sock);*/
}

static void visdn_recv_config(int mru, u_int32_t asyncmap, int pcomp, int accomp)
{
//	if (mru > visdn_max_mru)
//		error("Couldn't increase MRU to %d", mru);
}

static void set_xaccm_visdn(int unit, ext_accm accm)
{
	/* NOTHING */
}

void plugin_init(void)
{

	static char *bad_options[] = {
		"noaccomp", "-ac",
		"default-asyncmap", "-am", "asyncmap", "-as", "escape",
		"receive-all",
		"crtscts", "-crtscts", "nocrtscts",
		"cdtrcts", "nocdtrcts",
		"xonxoff",
		"modem", "local", "sync",
		NULL };

	if (!ppp_available() && !new_style_driver)
		fatal("Kernel doesn't support ppp_generic - needed for PPP over vISDN");

	add_options(visdn_options);
	info("VISDN plugin_init");
//  {
//    char **a;
//    for (a = bad_options; *a != NULL; a++)
//      remove_option(*a);
//  }
}

struct channel visdn_channel = {
	options:		visdn_options,
	process_extra_options:	visdn_process_extra_options,
	check_options:		NULL,
	connect:		visdn_connect,
	disconnect:		visdn_disconnect,
	establish_ppp:		generic_establish_ppp,
	disestablish_ppp:	generic_disestablish_ppp,
	send_config:		visdn_send_config,
	recv_config:		visdn_recv_config,
	close:			NULL,
	cleanup:		NULL
};
