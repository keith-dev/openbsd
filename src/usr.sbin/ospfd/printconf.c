/*	$OpenBSD: printconf.c,v 1.6 2006/11/09 03:59:54 joel Exp $ */

/*
 * Copyright (c) 2004, 2005 Esben Norby <norby@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/queue.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>

#include "ospf.h"
#include "ospfd.h"
#include "ospfe.h"

void	print_mainconf(struct ospfd_conf *);
const char *print_no(u_int16_t);
void	print_redistribute(struct ospfd_conf *);
void	print_iface(struct iface *);

void
print_mainconf(struct ospfd_conf *conf)
{
	printf("router-id %s\n", inet_ntoa(conf->rtr_id));

	if (conf->flags & OSPFD_FLAG_NO_FIB_UPDATE)
		printf("fib-update no\n");
	else
		printf("fib-update yes\n");

	if (conf->rfc1583compat)
		printf("rfc1583compat yes\n");
	else
		printf("rfc1583compat no\n");

	print_redistribute(conf);

	printf("spf-delay %u\n", conf->spf_delay);
	printf("spf-holdtime %u\n", conf->spf_hold_time);
}

const char *
print_no(u_int16_t type)
{
	if (type & REDIST_NO)
		return ("no ");
	else
		return ("");
}

void
print_redistribute(struct ospfd_conf *conf)
{
	struct redistribute	*r;

	if (conf->redistribute & REDISTRIBUTE_DEFAULT)
		printf("redistribute default\n");

	SIMPLEQ_FOREACH(r, &conf->redist_list, entry) {
		switch (r->type & ~REDIST_NO) {
		case REDIST_STATIC:
			printf("%sredistribute static\n", print_no(r->type));
			break;
		case REDIST_CONNECTED:
			printf("%sredistribute connected\n", print_no(r->type));
			break;
		case REDIST_LABEL:
			printf("%sredistribute rtlabel %s\n",
			    print_no(r->type), rtlabel_id2name(r->label));
			break;
		case REDIST_ADDR:
			printf("%ssredistribute %s/%d\n",
			    print_no(r->type), inet_ntoa(r->addr),
			    mask2prefixlen(r->mask.s_addr));
			break;
		}
	}
}

void
print_iface(struct iface *iface)
{
	struct auth_md	*md;

	printf("\tinterface %s {\n", iface->name);

	printf("\t\thello-interval %d\n", iface->hello_interval);
	printf("\t\tmetric %d\n", iface->metric);

	if (iface->passive)
		printf("\t\tpassive\n");

	printf("\t\tretransmit-interval %d\n", iface->rxmt_interval);
	printf("\t\trouter-dead-time %d\n", iface->dead_interval);
	printf("\t\trouter-priority %d\n", iface->priority);
	printf("\t\ttransmit-delay %d\n", iface->transmit_delay);

	printf("\t\tauth-type %s\n", if_auth_name(iface->auth_type));
	switch (iface->auth_type) {
	case AUTH_TYPE_NONE:
		break;
	case AUTH_TYPE_SIMPLE:
		printf("\t\tauth-key XXXXXX\n");
		break;
	case AUTH_TYPE_CRYPT:
		printf("\t\tauth-md-keyid %d\n", iface->auth_keyid);
		TAILQ_FOREACH(md, &iface->auth_md_list, entry)
			printf("\t\tauth-md %d XXXXXX\n", md->keyid);
		break;
	default:
		printf("\t\tunknown auth type!\n");
		break;
	}

	printf("\t}\n");
}

void
print_config(struct ospfd_conf *conf)
{
	struct area	*area;
	struct iface	*iface;

	printf("\n");
	print_mainconf(conf);
	printf("\n");

	LIST_FOREACH(area, &conf->area_list, entry) {
		printf("area %s {\n", inet_ntoa(area->id));
		LIST_FOREACH(iface, &area->iface_list, entry) {
			print_iface(iface);
		}
		printf("}\n\n");
	}
}