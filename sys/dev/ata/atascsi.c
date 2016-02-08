/*	$OpenBSD: atascsi.c,v 1.11 2007/03/07 03:04:28 pascoe Exp $ */

/*
 * Copyright (c) 2007 David Gwynne <dlg@openbsd.org>
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/device.h>
#include <sys/proc.h>
#include <sys/pool.h>
#include <sys/queue.h>

#include <scsi/scsi_all.h>
#include <scsi/scsi_disk.h>
#include <scsi/scsiconf.h>

#include <dev/ata/atascsi.h>

/* XXX ata_identify should be in atareg.h */

#define ATA_C_IDENTIFY		0xec

struct ata_identify {
	u_int16_t	config;		/*  0 */
	u_int16_t	ncyls;		/*  1 (OBSOLETE) */
	u_int16_t	reserved1;	/*  2 */
	u_int16_t	nheads;		/*  3 (OBSOLETE) */
	u_int16_t	track_size;	/*  4 (OBSOLETE) */
	u_int16_t	sector_size;	/*  5 (OBSOLETE) */
	u_int16_t	nsectors;	/*  6 (OBSOLETE) */
	u_int16_t	reserved2[3];	/*  7 vendor unique */
	u_int8_t	serial[20];	/* 10 */
	u_int16_t	buffer_type;	/* 20 */
	u_int16_t	buffer_size;	/* 21 */
	u_int16_t	ecc;		/* 22 */
	u_int8_t	firmware[8];	/* 23 */
	u_int8_t	model[40];	/* 27 */
	u_int16_t	multi;		/* 47 */
	u_int16_t	dwcap;		/* 48 */
	u_int16_t	cap;		/* 49 */
	u_int16_t	reserved3;	/* 50 */
	/* XXX there's a LOT more of this */
} __packed;

struct atascsi {
	struct device		*as_dev;
	void			*as_cookie;

	struct ata_port		**as_ports;

	struct atascsi_methods	*as_methods;
	struct scsi_adapter	as_switch;
	struct scsi_link	as_link;
	struct scsibus_softc	*as_scsibus;
};

int		atascsi_cmd(struct scsi_xfer *);

/* template */
struct scsi_adapter atascsi_switch = {
	atascsi_cmd,		/* scsi_cmd */
	minphys,		/* scsi_minphys */
	NULL,
	NULL,
	NULL			/* ioctl */
};

struct scsi_device atascsi_device = {
	NULL, NULL, NULL, NULL
};

int		atascsi_probe(struct atascsi *, int);

struct ata_xfer *ata_setup_identify(struct ata_port *, int);
void		ata_free_identify(struct ata_xfer *);
void		ata_complete_identify(struct ata_xfer *,
		    struct ata_identify *);

int		atascsi_disk_cmd(struct scsi_xfer *);
int		atascsi_disk_inq(struct scsi_xfer *);
void		atascsi_disk_inq_done(struct ata_xfer *);
int		atascsi_disk_capacity(struct scsi_xfer *);
void		atascsi_disk_capacity_done(struct ata_xfer *);
int		atascsi_disk_sync(struct scsi_xfer *);
int		atascsi_disk_sense(struct scsi_xfer *);

int		atascsi_atapi_cmd(struct scsi_xfer *);

int		atascsi_stuffup(struct scsi_xfer *);


int		ata_running = 0;
struct pool	ata_xfer_pool;

void		ata_init(void);
void		ata_destroy(void);

int		ata_exec(struct atascsi *, struct ata_xfer *);

struct ata_xfer	*ata_get_xfer(struct ata_port *, int);
void		ata_put_xfer(struct ata_xfer *);

struct atascsi *
atascsi_attach(struct device *self, struct atascsi_attach_args *aaa)
{
	struct scsibus_attach_args	saa;
	struct atascsi			*as;
	int				i;

	ata_init();

	as = malloc(sizeof(struct atascsi), M_DEVBUF, M_WAITOK);
	bzero(as, sizeof(struct atascsi));

	as->as_dev = self;
	as->as_cookie = aaa->aaa_cookie;
	as->as_methods = aaa->aaa_methods;

	/* copy from template and modify for ourselves */
	as->as_switch = atascsi_switch;
	as->as_switch.scsi_minphys = aaa->aaa_minphys;

	/* fill in our scsi_link */
	as->as_link.device = &atascsi_device;
	as->as_link.adapter = &as->as_switch;
	as->as_link.adapter_softc = as;
	as->as_link.adapter_buswidth = aaa->aaa_nports;
	as->as_link.luns = 1; /* XXX port multiplier as luns */
	as->as_link.adapter_target = aaa->aaa_nports;
	as->as_link.openings = aaa->aaa_ncmds;

	as->as_ports = malloc(sizeof(struct ata_port *) * aaa->aaa_nports,
	    M_DEVBUF, M_WAITOK);
	bzero(as->as_ports, sizeof(struct ata_port *) * aaa->aaa_nports);

	/* fill in the port array with the type of devices there */
	for (i = 0; i < as->as_link.adapter_buswidth; i++)
		atascsi_probe(as, i);

	bzero(&saa, sizeof(saa));
	saa.saa_sc_link = &as->as_link;

	/* stash the scsibus so we can do hotplug on it */
	as->as_scsibus = (struct scsibus_softc *)config_found(self, &saa,
            scsiprint);

	return (as);
}

int
atascsi_detach(struct atascsi *as)
{
	ata_destroy();

	return (0);
}

int
atascsi_probe(struct atascsi *as, int port)
{
	struct ata_port		*ap;
	int			type;

	if (port > as->as_link.adapter_buswidth)
		return (ENXIO);

	type = as->as_methods->probe(as->as_cookie, port);
	if (type != ATA_PORT_T_DISK) /* XXX ATAPI too one day */
		return (ENXIO);

	ap = malloc(sizeof(struct ata_port), M_DEVBUF, M_WAITOK);
	ap->ap_as = as;
	ap->ap_port = port;
	ap->ap_type = type;

	as->as_ports[port] = ap;

	return (0);
}

struct ata_xfer *
ata_setup_identify(struct ata_port *ap, int nosleep)
{
	struct ata_xfer		*xa;
	int			s;

	s = splbio();
	xa = ata_get_xfer(ap, nosleep);
	splx(s);
	if (xa == NULL)
		return (NULL);

	xa->data = malloc(512, M_TEMP, nosleep ? M_NOWAIT : M_WAITOK);
	if (xa->data == NULL) {
		s = splbio();
		ata_put_xfer(xa);
		splx(s);
		return (NULL);
	}
	bzero(xa->data, 512);
	xa->datalen = 512;

	xa->cmd.command = ATA_C_IDENTIFY;
	xa->cmd.st_bmask = 0x40; /* XXX magic WDCS_DRDY */;
	xa->cmd.st_pmask = 0x00;

	xa->flags = ATA_F_READ;

	return (xa);
}

void
ata_free_identify(struct ata_xfer *xa)
{
	free(xa->data, M_TEMP);
	ata_put_xfer(xa);
}

void
ata_complete_identify(struct ata_xfer *xa, struct ata_identify *id)
{
	u_int16_t		*swap;
	int			i;

	bcopy(xa->data, id, sizeof(struct ata_identify));
	ata_free_identify(xa);

	swap = (u_int16_t *)id->serial;
	for (i = 0; i < sizeof(id->serial) / sizeof(u_int16_t); i++)
		swap[i] = swap16(swap[i]);

	swap = (u_int16_t *)id->firmware;
	for (i = 0; i < sizeof(id->firmware) / sizeof(u_int16_t); i++)
		swap[i] = swap16(swap[i]);

	swap = (u_int16_t *)id->model;
	for (i = 0; i < sizeof(id->model) / sizeof(u_int16_t); i++)
		swap[i] = swap16(swap[i]);
}

int
atascsi_cmd(struct scsi_xfer *xs)
{
	struct scsi_link	*link = xs->sc_link;
	struct atascsi		*as = link->adapter_softc;
	struct ata_port		*ap = as->as_ports[link->target];

	if (ap == NULL)
		return (atascsi_stuffup(xs));

	switch (ap->ap_type) {
	case ATA_PORT_T_DISK:
		return (atascsi_disk_cmd(xs));
	case ATA_PORT_T_ATAPI:
		return (atascsi_atapi_cmd(xs));

	case ATA_PORT_T_NONE:
	default:
		return (atascsi_stuffup(xs));
	}
}

int
atascsi_disk_cmd(struct scsi_xfer *xs)
{
#if 0
	struct scsi_link	*link = xs->sc_link;
	struct atascsi		*as = link->adapter_softc;
	struct ata_port		*ap = as->as_ports[link->target];
	int			s;
#endif

	switch (xs->cmd->opcode) {
	case READ_BIG:
	case WRITE_BIG:
	case READ_COMMAND:
	case WRITE_COMMAND:
		/* deal with io outside the switch */
		break;

	case SYNCHRONIZE_CACHE:
		return (atascsi_disk_sync(xs));
	case REQUEST_SENSE:
		return (atascsi_disk_sense(xs));
	case INQUIRY:
		return (atascsi_disk_inq(xs));
	case READ_CAPACITY:
		return (atascsi_disk_capacity(xs));

	case TEST_UNIT_READY:
	case START_STOP:
	case PREVENT_ALLOW:
		return (COMPLETE);

	default:
		return (atascsi_stuffup(xs));
	}

	return (atascsi_stuffup(xs));
}

int
atascsi_disk_inq(struct scsi_xfer *xs)
{
	struct scsi_link	*link = xs->sc_link;
	struct atascsi		*as = link->adapter_softc;
	struct ata_port		*ap = as->as_ports[link->target];
	struct ata_xfer		*xa;
	int			s;

	xa = ata_setup_identify(ap, xs->flags & SCSI_NOSLEEP);
	if (xa == NULL)
		return (atascsi_stuffup(xs));

	xa->complete = atascsi_disk_inq_done;
	xa->atascsi_private = xs;
	if (xs->flags & SCSI_POLL)
		xa->flags |= ATA_F_POLL;

	switch (ata_exec(as, xa)) {
	case ATA_COMPLETE:
		return (COMPLETE);
	case ATA_QUEUED:
		return (SUCCESSFULLY_QUEUED);
	case ATA_ERROR:
		s = splbio();
		ata_free_identify(xa);
		splx(s);
		return (atascsi_stuffup(xs));
	default:
		panic("unexpected return from ata_exec");
	}
}

void
atascsi_disk_inq_done(struct ata_xfer *xa)
{
	struct scsi_xfer	*xs = xa->atascsi_private;
	struct ata_identify	id;
	struct scsi_inquiry_data inq;

	ata_complete_identify(xa, &id);

	bzero(&inq, sizeof(inq));

	inq.device = T_DIRECT;
	inq.version = 2;
	inq.response_format = 2;
	inq.additional_length = 32;
	bcopy("ATA     ", inq.vendor, sizeof(inq.vendor));
	bcopy(id.model, inq.product, sizeof(inq.product));
	bcopy(id.firmware, inq.revision, sizeof(inq.revision));

	bcopy(&inq, xs->data, MIN(sizeof(inq), xs->datalen));
	xs->error = XS_NOERROR;
	xs->flags |= ITSDONE;

	scsi_done(xs);
}

int
atascsi_disk_sync(struct scsi_xfer *xs)
{
	return (atascsi_stuffup(xs));
}

int
atascsi_disk_capacity(struct scsi_xfer *xs)
{
	struct scsi_link	*link = xs->sc_link;
	struct atascsi		*as = link->adapter_softc;
	struct ata_port		*ap = as->as_ports[link->target];
	struct ata_xfer		*xa;
	int			s;

	xa = ata_setup_identify(ap, xs->flags & SCSI_NOSLEEP);
	if (xa == NULL)
		return (atascsi_stuffup(xs));

	xa->complete = atascsi_disk_capacity_done;
	xa->atascsi_private = xs;
	if (xs->flags & SCSI_POLL)
		xa->flags |= ATA_F_POLL;

	switch (ata_exec(as, xa)) {
	case ATA_COMPLETE:
		return (COMPLETE);
	case ATA_QUEUED:
		return (SUCCESSFULLY_QUEUED);
	case ATA_ERROR:
		s = splbio();
		ata_free_identify(xa);
		splx(s);
		return (atascsi_stuffup(xs));
	default:
		panic("unexpected return from ata_exec");
	}
}

void
atascsi_disk_capacity_done(struct ata_xfer *xa)
{
	struct scsi_xfer	*xs = xa->atascsi_private;
	struct ata_identify	id;
	struct scsi_read_cap_data rcd;

	ata_complete_identify(xa, &id);

	bzero(&rcd, sizeof(rcd));
	_lto4b(1024 * 1024 * 1024, rcd.addr);
	_lto4b(512, rcd.length);

	bcopy(&rcd, xs->data, MIN(sizeof(rcd), xs->datalen));
	xs->error = XS_NOERROR;
	xs->flags |= ITSDONE;

	scsi_done(xs);
}

int
atascsi_disk_sense(struct scsi_xfer *xs)
{
	struct scsi_sense_data	*sd = (struct scsi_sense_data *)xs->data;
	int			s;

	bzero(xs->data, xs->datalen);
	/* check datalen > sizeof(struct scsi_sense_data)? */
	sd->error_code = 0x70; /* XXX magic */
	sd->flags = SKEY_NO_SENSE;

	xs->error = XS_NOERROR;
	xs->flags |= ITSDONE;

	s = splbio();
	scsi_done(xs);
	splx(s);
	return (COMPLETE);
}


int
atascsi_atapi_cmd(struct scsi_xfer *xs)
{
	return (atascsi_stuffup(xs));
}

int
atascsi_stuffup(struct scsi_xfer *xs)
{
	int			s;

	xs->error = XS_DRIVER_STUFFUP;
	xs->flags |= ITSDONE;

	s = splbio();
	scsi_done(xs);
	splx(s);
	return (COMPLETE);
}

void
ata_init(void)
{
	if (ata_running++)
		return;

	pool_init(&ata_xfer_pool, sizeof(struct ata_xfer), 0, 0, 0, "xapl",
	    NULL);
}

void
ata_destroy(void)
{
	if (--ata_running)
		return;

	pool_destroy(&ata_xfer_pool);
}
 
int
ata_exec(struct atascsi *as, struct ata_xfer *xa)
{
	xa->state = ATA_S_PENDING;
	return (as->as_methods->ata_cmd(as->as_cookie, xa));
}

struct ata_xfer *
ata_get_xfer(struct ata_port *ap, int nosleep)
{
	struct ata_xfer		*xa;

	xa = pool_get(&ata_xfer_pool, nosleep ? PR_NOWAIT : PR_WAITOK);
	if (xa != NULL) {
		bzero(&xa->cmd, sizeof(xa->cmd));
		xa->port = ap;
		xa->state = ATA_S_SETUP;
	}

	return (xa);
}

void
ata_put_xfer(struct ata_xfer *xa)
{
	pool_put(&ata_xfer_pool, xa);
}