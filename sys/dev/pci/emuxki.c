/*	$OpenBSD: emuxki.c,v 1.12 2003/02/18 14:32:13 jmc Exp $	*/
/*	$NetBSD: emuxki.c,v 1.1 2001/10/17 18:39:41 jdolecek Exp $	*/

/*-
 * Copyright (c) 2001 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Yannick Montulet.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the NetBSD
 *	Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Driver for Creative Labs SBLive! series and probably PCI512.
 * 
 * Known bugs:
 * - inversed stereo at ac97 codec level
 *   (XXX jdolecek - don't see the problem? maybe because auvia(4) has
 *    it swapped too?)
 * - bass disapear when you plug rear jack-in on Cambridge FPS2000 speakers
 *   (and presumably all speakers that support front and rear jack-in)
 *
 * TODO:
 * - Digital Outputs
 * - (midi/mpu),joystick support
 * - Single source recording
 * - Multiple voices play (problem with /dev/audio architecture)
 * - Multiple sources recording (Pb with audio(4))
 * - Independant modification of each channel's parameters (via mixer ?)
 * - DSP FX patches (to make fx like chipmunk)
 */

#include <sys/types.h>
#include <sys/device.h>
#include <sys/errno.h>
#include <sys/malloc.h>
#include <sys/systm.h>
#include <sys/param.h>
#include <sys/audioio.h>
#include <sys/select.h>

#include <dev/pci/pcireg.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcidevs.h>

#include <dev/audio_if.h>
#include <dev/audiovar.h>
#include <dev/auconv.h>
#include <dev/mulaw.h>
#include <dev/ic/ac97.h>

#include <dev/pci/emuxkireg.h>
#include <dev/pci/emuxkivar.h>

/* autconf goo */
int  emuxki_match(struct device *, void *, void *);
void emuxki_attach(struct device *, struct device *, void *);
int  emuxki_detach(struct device *, int);
int  emuxki_scinit(struct emuxki_softc *sc);
void emuxki_pci_shutdown(struct emuxki_softc *sc);

/* dma mem mgmt */
struct dmamem *emuxki_dmamem_alloc(bus_dma_tag_t, size_t, bus_size_t,
				 int, int, int);
void	emuxki_dmamem_free(struct dmamem *, int);
void	emuxki_dmamem_delete(struct dmamem *mem, int type);

struct emuxki_mem *emuxki_mem_new(struct emuxki_softc *sc, int ptbidx,
	size_t size, int type, int flags);
void emuxki_mem_delete(struct emuxki_mem *mem, int type);

/* Emu10k1 init & shutdown */
int  emuxki_init(struct emuxki_softc *);
void emuxki_shutdown(struct emuxki_softc *);

/* Emu10k1 mem mgmt */
void   *emuxki_pmem_alloc(struct emuxki_softc *, size_t,int,int);
void   *emuxki_rmem_alloc(struct emuxki_softc *, size_t,int,int);

/*
 * Emu10k1 channels funcs : There is no direct access to channels, everything
 * is done through voices I will at least provide channel based fx params
 * modification, later...
 */

/* Emu10k1 voice mgmt */
struct emuxki_voice *emuxki_voice_new(struct emuxki_softc *, u_int8_t);
void   emuxki_voice_delete(struct emuxki_voice *);
int    emuxki_voice_set_audioparms(struct emuxki_voice *, u_int8_t, u_int8_t, u_int32_t);
/* emuxki_voice_set_fxparms will come later, it'll need channel distinction */
int emuxki_voice_set_bufparms(struct emuxki_voice *, void *, u_int32_t, u_int16_t);
int emuxki_voice_set_stereo(struct emuxki_voice *voice, u_int8_t stereo);
int emuxki_voice_dataloc_create(struct emuxki_voice *voice);
void emuxki_voice_dataloc_destroy(struct emuxki_voice *voice);
void emuxki_voice_commit_parms(struct emuxki_voice *);
void emuxki_voice_recsrc_release(struct emuxki_softc *sc, emuxki_recsrc_t source);
u_int32_t emuxki_voice_curaddr(struct emuxki_voice *);
int emuxki_set_vparms(struct emuxki_voice *voice, struct audio_params *p);
int emuxki_voice_set_srate(struct emuxki_voice *voice, u_int32_t srate);
void emuxki_voice_start(struct emuxki_voice *, void (*) (void *), void *);
void emuxki_voice_halt(struct emuxki_voice *);
int emuxki_voice_channel_create(struct emuxki_voice *voice);
void emuxki_voice_channel_destroy(struct emuxki_voice *voice);

struct emuxki_channel *emuxki_channel_new(struct emuxki_voice *voice, u_int8_t num);
void emuxki_channel_delete(struct emuxki_channel *chan);
void emuxki_channel_start(struct emuxki_channel *chan);
void emuxki_channel_stop(struct emuxki_channel *chan);
void emuxki_channel_commit_parms(struct emuxki_channel *chan);
void emuxki_channel_set_bufparms(struct emuxki_channel *chan, u_int32_t start, u_int32_t end);
void emuxki_channel_set_srate(struct emuxki_channel *chan, u_int32_t srate);
void emuxki_channel_set_fxsend(struct emuxki_channel *chan,
	struct emuxki_chanparms_fxsend *fxsend);
void emuxki_chanparms_set_defaults(struct emuxki_channel *chan);

void emuxki_resched_timer(struct emuxki_softc *sc);

/*
 * Emu10k1 stream mgmt : not done yet
 */
#if 0
struct emuxki_stream *emuxki_stream_new(struct emu10k1 *);
void   emuxki_stream_delete(struct emuxki_stream *);
int    emuxki_stream_set_audio_params(struct emuxki_stream *, u_int8_t,
					    u_int8_t, u_int8_t, u_int16_t);
void   emuxki_stream_start(struct emuxki_stream *);
void   emuxki_stream_halt(struct emuxki_stream *);
#endif

/* fx interface */
void emuxki_initfx(struct emuxki_softc *sc);
void emuxki_dsp_addop(struct emuxki_softc *sc, u_int16_t *pc, u_int8_t op,
	u_int16_t r, u_int16_t a, u_int16_t x, u_int16_t y);
void emuxki_write_micro(struct emuxki_softc *sc, u_int32_t pc, u_int32_t data);

/* audio interface callbacks */

int	emuxki_open(void *, int);
void	emuxki_close(void *);

int	emuxki_query_encoding(void *, struct audio_encoding *);
int	emuxki_set_params(void *, int, int,
				      struct audio_params *,
				      struct audio_params *);

size_t	emuxki_round_buffersize(void *, int, size_t);

int	emuxki_trigger_output(void *, void *, void *, int, void (*)(void *),
	    void *, struct audio_params *);
int	emuxki_trigger_input(void *, void *, void *, int, void (*) (void *),
	    void *, struct audio_params *);
int	emuxki_halt_output(void *);
int	emuxki_halt_input(void *);

int	emuxki_getdev(void *, struct audio_device *);
int	emuxki_set_port(void *, mixer_ctrl_t *);
int	emuxki_get_port(void *, mixer_ctrl_t *);
int	emuxki_query_devinfo(void *, mixer_devinfo_t *);

void   *emuxki_allocm(void *, int, size_t, int, int);
void	emuxki_freem(void *, void *, int);

paddr_t	emuxki_mappage(void *, void *, off_t, int);
int	emuxki_get_props(void *);

/* Interrupt handler */
int  emuxki_intr(void *);

/* Emu10k1 AC97 interface callbacks */
int  emuxki_ac97_init(struct emuxki_softc *sc);
int  emuxki_ac97_attach(void *, struct ac97_codec_if *);
int  emuxki_ac97_read(void *, u_int8_t, u_int16_t *);
int  emuxki_ac97_write(void *, u_int8_t, u_int16_t);
void emuxki_ac97_reset(void *);

/*
 * Autoconfig goo.
 */
struct cfdriver emu_cd = {
	NULL, "emu", DV_DULL
};

struct cfattach emu_ca = {
        sizeof(struct emuxki_softc),
        emuxki_match,
        emuxki_attach,
	emuxki_detach,
	NULL		  /* config activate */
};

struct audio_hw_if emuxki_hw_if = {
	emuxki_open,
	emuxki_close,
	NULL,			/* drain */
	emuxki_query_encoding,
	emuxki_set_params,
	NULL,			/* round blocksize */
	NULL,			/* commit settings */
	NULL,			/* init_output */
	NULL,			/* init_input */
	NULL,			/* start_output */
	NULL,			/* start_input */
	emuxki_halt_output,
	emuxki_halt_input,
	NULL,			/* speaker_ctl */
	emuxki_getdev,
	NULL,			/* setfd */
	emuxki_set_port,
	emuxki_get_port,
	emuxki_query_devinfo,
	emuxki_allocm,
	emuxki_freem,
	emuxki_round_buffersize,
	emuxki_mappage,
	emuxki_get_props,
	emuxki_trigger_output,
	emuxki_trigger_input,
};

/*
 * Dma memory mgmt
 */

void
emuxki_dmamem_delete(struct dmamem *mem, int type)
{
	free(mem->segs, type);
	free(mem, type);
}

struct dmamem *
emuxki_dmamem_alloc(bus_dma_tag_t dmat, size_t size, bus_size_t align,
	     int nsegs, int type, int flags)
{
	struct dmamem	*mem;
	int		bus_dma_flags;

	/* Allocate memory for structure */
	if ((mem = malloc(sizeof(*mem), type, flags)) == NULL)
		return (NULL);
	mem->dmat = dmat;
	mem->size = size;
	mem->align = align;
	mem->nsegs = nsegs;
	mem->bound = 0;

	mem->segs = malloc(mem->nsegs * sizeof(*(mem->segs)), type, flags);
	if (mem->segs == NULL) {
		free(mem, type);
		return (NULL);
	}

	bus_dma_flags = (flags & M_NOWAIT) ? BUS_DMA_NOWAIT : BUS_DMA_WAITOK;
	if (bus_dmamem_alloc(dmat, mem->size, mem->align, mem->bound,
			     mem->segs, mem->nsegs, &(mem->rsegs),
			     bus_dma_flags)) {
		emuxki_dmamem_delete(mem, type);
		return (NULL);
	}

	if (bus_dmamem_map(dmat, mem->segs, mem->nsegs, mem->size,
			   &(mem->kaddr), bus_dma_flags | BUS_DMA_COHERENT)) {
		bus_dmamem_free(dmat, mem->segs, mem->nsegs);
		emuxki_dmamem_delete(mem, type);
		return (NULL);
	}

	if (bus_dmamap_create(dmat, mem->size, mem->nsegs, mem->size,
			      mem->bound, bus_dma_flags, &(mem->map))) {
		bus_dmamem_unmap(dmat, mem->kaddr, mem->size);
		bus_dmamem_free(dmat, mem->segs, mem->nsegs);
		emuxki_dmamem_delete(mem, type);
		return (NULL);
	}

	if (bus_dmamap_load(dmat, mem->map, mem->kaddr, 
			    mem->size, NULL, bus_dma_flags)) {
		bus_dmamap_destroy(dmat, mem->map);
		bus_dmamem_unmap(dmat, mem->kaddr, mem->size);
		bus_dmamem_free(dmat, mem->segs, mem->nsegs);
		emuxki_dmamem_delete(mem, type);
		return (NULL);
	}

	return (mem);
}

void
emuxki_dmamem_free(struct dmamem *mem, int type)
{
	bus_dmamap_unload(mem->dmat, mem->map);
	bus_dmamap_destroy(mem->dmat, mem->map);
	bus_dmamem_unmap(mem->dmat, mem->kaddr, mem->size);
	bus_dmamem_free(mem->dmat, mem->segs, mem->nsegs);
	emuxki_dmamem_delete(mem, type);
}


/*
 * Autoconf device callbacks : attach and detach
 */

void
emuxki_pci_shutdown(struct emuxki_softc *sc)
{
	if (sc->sc_ih != NULL)
		pci_intr_disestablish(sc->sc_pc, sc->sc_ih);
	if (sc->sc_ios)
		bus_space_unmap(sc->sc_iot, sc->sc_ioh, sc->sc_ios);
}

int
emuxki_scinit(struct emuxki_softc *sc)
{
	int             err;

	bus_space_write_4(sc->sc_iot, sc->sc_ioh, EMU_HCFG,
		/* enable spdif(?) output on non-APS */
		(sc->sc_type == EMUXKI_APS? 0 : EMU_HCFG_GPOUTPUT0) |
		EMU_HCFG_LOCKSOUNDCACHE | EMU_HCFG_LOCKTANKCACHE_MASK |
		EMU_HCFG_MUTEBUTTONENABLE);
	bus_space_write_4(sc->sc_iot, sc->sc_ioh, EMU_INTE,
		EMU_INTE_SAMPLERATER | EMU_INTE_PCIERRENABLE);

	if ((err = emuxki_init(sc)))
		return (err);

	bus_space_write_4(sc->sc_iot, sc->sc_ioh, EMU_HCFG,
		EMU_HCFG_AUDIOENABLE |
		EMU_HCFG_LOCKTANKCACHE_MASK | EMU_HCFG_AUTOMUTE);
	bus_space_write_4(sc->sc_iot, sc->sc_ioh, EMU_INTE,
		bus_space_read_4(sc->sc_iot, sc->sc_ioh, EMU_INTE) |
		EMU_INTE_VOLINCRENABLE | EMU_INTE_VOLDECRENABLE |
		EMU_INTE_MUTEENABLE);

	/* No multiple voice support for now */
	sc->pvoice = sc->rvoice = NULL;

	return (0);
}

int
emuxki_ac97_init(struct emuxki_softc *sc)
{
	sc->hostif.arg = sc;
	sc->hostif.attach = emuxki_ac97_attach;
	sc->hostif.read = emuxki_ac97_read;
	sc->hostif.write = emuxki_ac97_write;
	sc->hostif.reset = emuxki_ac97_reset;
	sc->hostif.flags = NULL;
	return (ac97_attach(&(sc->hostif)));
}

int
emuxki_match(struct device *parent, void *match, void *aux)
{
	struct pci_attach_args *pa = aux;

	if (PCI_VENDOR(pa->pa_id) == PCI_VENDOR_CREATIVELABS &&
	    PCI_PRODUCT(pa->pa_id) == PCI_PRODUCT_CREATIVELABS_SBLIVE)
		return (1);

	return (0);
}

void
emuxki_attach(struct device *parent, struct device *self, void *aux)
{
	struct emuxki_softc *sc = (struct emuxki_softc *) self;
	struct pci_attach_args *pa = aux;
	pci_intr_handle_t ih;
	const char     *intrstr;

	if (pci_mapreg_map(pa, EMU_PCI_CBIO, PCI_MAPREG_TYPE_IO, 0,
	    &(sc->sc_iot), &(sc->sc_ioh), &(sc->sc_iob), &(sc->sc_ios), 0)) {
		printf(": can't map iospace\n");
		return;
	}

	sc->sc_pc   = pa->pa_pc;
	sc->sc_dmat = pa->pa_dmat;
	pci_conf_write(pa->pa_pc, pa->pa_tag, PCI_COMMAND_STATUS_REG,
		pci_conf_read(pa->pa_pc, pa->pa_tag,
		(PCI_COMMAND_STATUS_REG) | PCI_COMMAND_MASTER_ENABLE));

	if (pci_intr_map(pa, &ih)) {
		printf(": couldn't map interrupt\n");
		bus_space_unmap(sc->sc_iot, sc->sc_ioh, sc->sc_ios);
		return;
	}

	intrstr = pci_intr_string(pa->pa_pc, ih);
	sc->sc_ih = pci_intr_establish(pa->pa_pc, ih, IPL_AUDIO, emuxki_intr,
		sc, sc->sc_dev.dv_xname);
	if (sc->sc_ih == NULL) {
		printf(": couldn't establish interrupt");
		if (intrstr != NULL)
			printf(" at %s", intrstr);
		printf("\n");
		bus_space_unmap(sc->sc_iot, sc->sc_ioh, sc->sc_ios);
		return;
	}
	printf(": %s\n", intrstr);

	/* XXX it's unknown wheather APS is made from Audigy as well */
	if (PCI_PRODUCT(pa->pa_id) == PCI_PRODUCT_CREATIVELABS_AUDIGY) {
		sc->sc_type = EMUXKI_AUDIGY;
		strcpy(sc->sc_audv.name, "Audigy");
	} else if (pci_conf_read(pa->pa_pc, pa->pa_tag,
	    PCI_SUBSYS_ID_REG) == EMU_SUBSYS_APS) {
		sc->sc_type = EMUXKI_APS;
		strcpy(sc->sc_audv.name, "E-mu APS");
	} else {
		sc->sc_type = EMUXKI_SBLIVE;
		strcpy(sc->sc_audv.name, "SB Live!");
	}
	sprintf(sc->sc_audv.version, "0x%02x", PCI_REVISION(pa->pa_class));
	strcpy(sc->sc_audv.config, "emuxki");

	if (emuxki_scinit(sc) ||
	    /* APS has no ac97 XXX */
	    (sc->sc_type == EMUXKI_APS || emuxki_ac97_init(sc)) ||
	    (sc->sc_audev = audio_attach_mi(&emuxki_hw_if, sc, self)) == NULL)
		emuxki_pci_shutdown(sc);
}

int
emuxki_detach(struct device *self, int flags)
{
	struct emuxki_softc *sc = (struct emuxki_softc *) self;
	int             err = 0;

        if (sc->sc_audev != NULL) /* Test in case audio didn't attach */
	        err = config_detach(sc->sc_audev, 0);

	/* All voices should be stopped now but add some code here if not */

	bus_space_write_4(sc->sc_iot, sc->sc_ioh, EMU_HCFG,
		EMU_HCFG_LOCKSOUNDCACHE | EMU_HCFG_LOCKTANKCACHE_MASK |
		EMU_HCFG_MUTEBUTTONENABLE);
	bus_space_write_4(sc->sc_iot, sc->sc_ioh, EMU_INTE, 0);

	emuxki_shutdown(sc);

	emuxki_pci_shutdown(sc);

	return (0);
}


/* Misc stuff relative to emu10k1 */

static __inline u_int32_t
emuxki_rate_to_pitch(u_int32_t rate)
{
	static const u_int32_t logMagTable[128] = {
		0x00000, 0x02dfc, 0x05b9e, 0x088e6, 0x0b5d6, 0x0e26f, 0x10eb3,
		0x13aa2, 0x1663f, 0x1918a, 0x1bc84, 0x1e72e, 0x2118b, 0x23b9a,
		0x2655d, 0x28ed5, 0x2b803, 0x2e0e8, 0x30985, 0x331db, 0x359eb,
		0x381b6, 0x3a93d, 0x3d081, 0x3f782, 0x41e42, 0x444c1, 0x46b01,
		0x49101, 0x4b6c4, 0x4dc49, 0x50191, 0x5269e, 0x54b6f, 0x57006,
		0x59463, 0x5b888, 0x5dc74, 0x60029, 0x623a7, 0x646ee, 0x66a00,
		0x68cdd, 0x6af86, 0x6d1fa, 0x6f43c, 0x7164b, 0x73829, 0x759d4,
		0x77b4f, 0x79c9a, 0x7bdb5, 0x7dea1, 0x7ff5e, 0x81fed, 0x8404e,
		0x86082, 0x88089, 0x8a064, 0x8c014, 0x8df98, 0x8fef1, 0x91e20,
		0x93d26, 0x95c01, 0x97ab4, 0x9993e, 0x9b79f, 0x9d5d9, 0x9f3ec,
		0xa11d8, 0xa2f9d, 0xa4d3c, 0xa6ab5, 0xa8808, 0xaa537, 0xac241,
		0xadf26, 0xafbe7, 0xb1885, 0xb3500, 0xb5157, 0xb6d8c, 0xb899f,
		0xba58f, 0xbc15e, 0xbdd0c, 0xbf899, 0xc1404, 0xc2f50, 0xc4a7b,
		0xc6587, 0xc8073, 0xc9b3f, 0xcb5ed, 0xcd07c, 0xceaec, 0xd053f,
		0xd1f73, 0xd398a, 0xd5384, 0xd6d60, 0xd8720, 0xda0c3, 0xdba4a,
		0xdd3b4, 0xded03, 0xe0636, 0xe1f4e, 0xe384a, 0xe512c, 0xe69f3,
		0xe829f, 0xe9b31, 0xeb3a9, 0xecc08, 0xee44c, 0xefc78, 0xf148a,
		0xf2c83, 0xf4463, 0xf5c2a, 0xf73da, 0xf8b71, 0xfa2f0, 0xfba57,
		0xfd1a7, 0xfe8df
	};
	static const u_int8_t logSlopeTable[128] = {
		0x5c, 0x5c, 0x5b, 0x5a, 0x5a, 0x59, 0x58, 0x58,
		0x57, 0x56, 0x56, 0x55, 0x55, 0x54, 0x53, 0x53,
		0x52, 0x52, 0x51, 0x51, 0x50, 0x50, 0x4f, 0x4f,
		0x4e, 0x4d, 0x4d, 0x4d, 0x4c, 0x4c, 0x4b, 0x4b,
		0x4a, 0x4a, 0x49, 0x49, 0x48, 0x48, 0x47, 0x47,
		0x47, 0x46, 0x46, 0x45, 0x45, 0x45, 0x44, 0x44,
		0x43, 0x43, 0x43, 0x42, 0x42, 0x42, 0x41, 0x41,
		0x41, 0x40, 0x40, 0x40, 0x3f, 0x3f, 0x3f, 0x3e,
		0x3e, 0x3e, 0x3d, 0x3d, 0x3d, 0x3c, 0x3c, 0x3c,
		0x3b, 0x3b, 0x3b, 0x3b, 0x3a, 0x3a, 0x3a, 0x39,
		0x39, 0x39, 0x39, 0x38, 0x38, 0x38, 0x38, 0x37,
		0x37, 0x37, 0x37, 0x36, 0x36, 0x36, 0x36, 0x35,
		0x35, 0x35, 0x35, 0x34, 0x34, 0x34, 0x34, 0x34,
		0x33, 0x33, 0x33, 0x33, 0x32, 0x32, 0x32, 0x32,
		0x32, 0x31, 0x31, 0x31, 0x31, 0x31, 0x30, 0x30,
		0x30, 0x30, 0x30, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f
	};
	int8_t          i;

	if (rate == 0)
		return 0;	/* Bail out if no leading "1" */
	rate *= 11185;		/* Scale 48000 to 0x20002380 */
	for (i = 31; i > 0; i--) {
		if (rate & 0x80000000) {	/* Detect leading "1" */
			return (((u_int32_t) (i - 15) << 20) +
				logMagTable[0x7f & (rate >> 24)] +
				(0x7f & (rate >> 17)) *
				logSlopeTable[0x7f & (rate >> 24)]);
		}
		rate <<= 1;
	}

	return 0;		/* Should never reach this point */
}

/* Emu10k1 Low level */

static __inline u_int32_t
emuxki_read(struct emuxki_softc *sc, u_int16_t chano, u_int32_t reg)
{
	u_int32_t       ptr, mask = 0xffffffff;
	u_int8_t        size, offset = 0;
	int             s;

	ptr = ((((u_int32_t) reg) << 16) & EMU_PTR_ADDR_MASK) |
		(chano & EMU_PTR_CHNO_MASK);
	if (reg & 0xff000000) {
		size = (reg >> 24) & 0x3f;
		offset = (reg >> 16) & 0x1f;
		mask = ((1 << size) - 1) << offset;
	}

	s = splaudio();
	bus_space_write_4(sc->sc_iot, sc->sc_ioh, EMU_PTR, ptr);
	ptr = (bus_space_read_4(sc->sc_iot, sc->sc_ioh, EMU_DATA) & mask)
		>> offset;
	splx(s);

	return (ptr);
}

static __inline void
emuxki_write(struct emuxki_softc *sc, u_int16_t chano,
	      u_int32_t reg, u_int32_t data)
{
	u_int32_t       ptr, mask;
	u_int8_t        size, offset;
	int             s;

	ptr = ((((u_int32_t) reg) << 16) & EMU_PTR_ADDR_MASK) |
		(chano & EMU_PTR_CHNO_MASK);

	/* BE CAREFUL WITH THAT AXE, EUGENE */
	if (ptr == 0x52 || ptr == 0x53)
		return;

	if (reg & 0xff000000) {
		size = (reg >> 24) & 0x3f;
		offset = (reg >> 16) & 0x1f;
		mask = ((1 << size) - 1) << offset;
		data = ((data << offset) & mask) |
			(emuxki_read(sc, chano, reg & 0xffff) & ~mask);
	}

	s = splaudio();
	bus_space_write_4(sc->sc_iot, sc->sc_ioh, EMU_PTR, ptr);
	bus_space_write_4(sc->sc_iot, sc->sc_ioh, EMU_DATA, data);
	splx(s);
}

/* Microcode should this go in /sys/dev/microcode ? */

void
emuxki_write_micro(struct emuxki_softc *sc, u_int32_t pc, u_int32_t data)
{
	emuxki_write(sc, 0, EMU_MICROCODEBASE + pc, data);
}

void
emuxki_dsp_addop(struct emuxki_softc *sc, u_int16_t *pc, u_int8_t op,
		  u_int16_t r, u_int16_t a, u_int16_t x, u_int16_t y)
{
	emuxki_write_micro(sc, *pc << 1,
		((x << 10) & EMU_DSP_LOWORD_OPX_MASK) |
		(y & EMU_DSP_LOWORD_OPY_MASK));
	emuxki_write_micro(sc, (*pc << 1) + 1,
		((op << 20) & EMU_DSP_HIWORD_OPCODE_MASK) |
		((r << 10) & EMU_DSP_HIWORD_RESULT_MASK) |
		(a & EMU_DSP_HIWORD_OPA_MASK));
	(*pc)++;
}

/* init and shutdown */

void
emuxki_initfx(struct emuxki_softc *sc)
{
	u_int16_t       pc;

	/* Set all GPRs to 0 */
	for (pc = 0; pc < 256; pc++)
		emuxki_write(sc, 0, EMU_DSP_GPR(pc), 0);
	for (pc = 0; pc < 160; pc++) {
		emuxki_write(sc, 0, EMU_TANKMEMDATAREGBASE + pc, 0);
		emuxki_write(sc, 0, EMU_TANKMEMADDRREGBASE + pc, 0);
	}
	pc = 0;
	/* AC97 Out (l/r) = AC97 In (l/r) + FX[0/1] * 4 */
	emuxki_dsp_addop(sc, &pc, EMU_DSP_OP_MACINTS,
			  EMU_DSP_OUTL(EMU_DSP_OUT_AC97),
			  EMU_DSP_CST(0),
			  EMU_DSP_FX(0), EMU_DSP_CST(4));
	emuxki_dsp_addop(sc, &pc, EMU_DSP_OP_MACINTS,
			  EMU_DSP_OUTR(EMU_DSP_OUT_AC97),
			  EMU_DSP_CST(0),
			  EMU_DSP_FX(1), EMU_DSP_CST(4));

	/* Rear channel OUT (l/r) = FX[2/3] * 4 */
#if 0
	emuxki_dsp_addop(sc, &pc, EMU_DSP_OP_MACINTS,
			  EMU_DSP_OUTL(EMU_DSP_OUT_RCHAN),
			  EMU_DSP_OUTL(EMU_DSP_OUT_AC97),
			  EMU_DSP_FX(0), EMU_DSP_CST(4));
	emuxki_dsp_addop(sc, &pc, EMU_DSP_OP_MACINTS,
			  EMU_DSP_OUTR(EMU_DSP_OUT_RCHAN),
			  EMU_DSP_OUTR(EMU_DSP_OUT_AC97),
			  EMU_DSP_FX(1), EMU_DSP_CST(4));
#endif
	/* zero out the rest of the microcode */
	while (pc < 512)
		emuxki_dsp_addop(sc, &pc, EMU_DSP_OP_ACC3,
				  EMU_DSP_CST(0), EMU_DSP_CST(0),
				  EMU_DSP_CST(0), EMU_DSP_CST(0));

	emuxki_write(sc, 0, EMU_DBG, 0);	/* Is it really necessary ? */
}

int
emuxki_init(struct emuxki_softc *sc)
{
	u_int16_t       i;
	u_int32_t       spcs, *ptb;
	bus_addr_t      silentpage;

	/* disable any channel interrupt */
	emuxki_write(sc, 0, EMU_CLIEL, 0);
	emuxki_write(sc, 0, EMU_CLIEH, 0);
	emuxki_write(sc, 0, EMU_SOLEL, 0);
	emuxki_write(sc, 0, EMU_SOLEH, 0);

	/* Set recording buffers sizes to zero */
	emuxki_write(sc, 0, EMU_MICBS, EMU_RECBS_BUFSIZE_NONE);
	emuxki_write(sc, 0, EMU_MICBA, 0);
	emuxki_write(sc, 0, EMU_FXBS, EMU_RECBS_BUFSIZE_NONE);
	emuxki_write(sc, 0, EMU_FXBA, 0);
	emuxki_write(sc, 0, EMU_ADCBS, EMU_RECBS_BUFSIZE_NONE);
	emuxki_write(sc, 0, EMU_ADCBA, 0);

	/* Initialize all channels to stopped and no effects */
	for (i = 0; i < EMU_NUMCHAN; i++) {
		emuxki_write(sc, i, EMU_CHAN_DCYSUSV, 0);
		emuxki_write(sc, i, EMU_CHAN_IP, 0);
		emuxki_write(sc, i, EMU_CHAN_VTFT, 0xffff);
		emuxki_write(sc, i, EMU_CHAN_CVCF, 0xffff);
		emuxki_write(sc, i, EMU_CHAN_PTRX, 0);
		emuxki_write(sc, i, EMU_CHAN_CPF, 0);
		emuxki_write(sc, i, EMU_CHAN_CCR, 0);
		emuxki_write(sc, i, EMU_CHAN_PSST, 0);
		emuxki_write(sc, i, EMU_CHAN_DSL, 0x10);	/* Why 16 ? */
		emuxki_write(sc, i, EMU_CHAN_CCCA, 0);
		emuxki_write(sc, i, EMU_CHAN_Z1, 0);
		emuxki_write(sc, i, EMU_CHAN_Z2, 0);
		emuxki_write(sc, i, EMU_CHAN_FXRT, 0x32100000);
		emuxki_write(sc, i, EMU_CHAN_ATKHLDM, 0);
		emuxki_write(sc, i, EMU_CHAN_DCYSUSM, 0);
		emuxki_write(sc, i, EMU_CHAN_IFATN, 0xffff);
		emuxki_write(sc, i, EMU_CHAN_PEFE, 0);
		emuxki_write(sc, i, EMU_CHAN_FMMOD, 0);
		emuxki_write(sc, i, EMU_CHAN_TREMFRQ, 24);
		emuxki_write(sc, i, EMU_CHAN_FM2FRQ2, 24);
		emuxki_write(sc, i, EMU_CHAN_TEMPENV, 0);

		/* these are last so OFF prevents writing */
		emuxki_write(sc, i, EMU_CHAN_LFOVAL2, 0);
		emuxki_write(sc, i, EMU_CHAN_LFOVAL1, 0);
		emuxki_write(sc, i, EMU_CHAN_ATKHLDV, 0);
		emuxki_write(sc, i, EMU_CHAN_ENVVOL, 0);
		emuxki_write(sc, i, EMU_CHAN_ENVVAL, 0);
	}

	/* set digital outputs format */
	spcs = (EMU_SPCS_CLKACCY_1000PPM | EMU_SPCS_SAMPLERATE_48 |
	      EMU_SPCS_CHANNELNUM_LEFT | EMU_SPCS_SOURCENUM_UNSPEC |
		EMU_SPCS_GENERATIONSTATUS | 0x00001200 /* Cat code. */ |
		0x00000000 /* IEC-958 Mode */ | EMU_SPCS_EMPHASIS_NONE |
		EMU_SPCS_COPYRIGHT);
	emuxki_write(sc, 0, EMU_SPCS0, spcs);
	emuxki_write(sc, 0, EMU_SPCS1, spcs);
	emuxki_write(sc, 0, EMU_SPCS2, spcs);

	/* Let's play with sound processor */
	emuxki_initfx(sc);

	/* Here is our Page Table */
	if ((sc->ptb = emuxki_dmamem_alloc(sc->sc_dmat,
	    EMU_MAXPTE * sizeof(u_int32_t),
	    EMU_DMA_ALIGN, EMU_DMAMEM_NSEG,
	    M_DEVBUF, M_WAITOK)) == NULL)
		return (ENOMEM);

	/* This is necessary unless you like Metallic noise... */
	if ((sc->silentpage = emuxki_dmamem_alloc(sc->sc_dmat, EMU_PTESIZE,
	    EMU_DMA_ALIGN, EMU_DMAMEM_NSEG, M_DEVBUF, M_WAITOK))==NULL){
		emuxki_dmamem_free(sc->ptb, M_DEVBUF);
		return (ENOMEM);
	}

	/* Zero out the silent page */
	/* This might not be always true, it might be 128 for 8bit channels */
	memset(KERNADDR(sc->silentpage), 0, DMASIZE(sc->silentpage));

	/*
	 * Set all the PTB Entries to the silent page We shift the physical
	 * address by one and OR it with the page number. I don't know what
	 * the ORed index is for, might be a very useful unused feature...
	 */
	silentpage = DMAADDR(sc->silentpage) << 1;
	ptb = KERNADDR(sc->ptb);
	for (i = 0; i < EMU_MAXPTE; i++)
		ptb[i] = silentpage | i;

	/* Write PTB address and set TCB to none */
	emuxki_write(sc, 0, EMU_PTB, DMAADDR(sc->ptb));
	emuxki_write(sc, 0, EMU_TCBS, 0);	/* This means 16K TCB */
	emuxki_write(sc, 0, EMU_TCB, 0);	/* No TCB use for now */

	/*
	 * Set channels MAPs to the silent page.
	 * I don't know what MAPs are for.
	 */
	silentpage |= EMU_CHAN_MAP_PTI_MASK;
	for (i = 0; i < EMU_NUMCHAN; i++) {
		emuxki_write(sc, i, EMU_CHAN_MAPA, silentpage);
		emuxki_write(sc, i, EMU_CHAN_MAPB, silentpage);
		sc->channel[i] = NULL;
	}

	/* Init voices list */
	LIST_INIT(&(sc->voices));

	/* Timer is stopped */
	sc->timerstate &= ~EMU_TIMER_STATE_ENABLED;
	return (0);
}

void
emuxki_shutdown(struct emuxki_softc *sc)
{
	u_int32_t       i;

	/* Disable any Channels interrupts */
	emuxki_write(sc, 0, EMU_CLIEL, 0);
	emuxki_write(sc, 0, EMU_CLIEH, 0);
	emuxki_write(sc, 0, EMU_SOLEL, 0);
	emuxki_write(sc, 0, EMU_SOLEH, 0);

	/*
	 * Should do some voice(stream) stopping stuff here, that's what will
	 * stop and deallocate all channels.
	 */

	/* Stop all channels */
	/* XXX This shouldn't be necessary, I'll remove once everything works */
	for (i = 0; i < EMU_NUMCHAN; i++)
		emuxki_write(sc, i, EMU_CHAN_DCYSUSV, 0);
	for (i = 0; i < EMU_NUMCHAN; i++) {
		emuxki_write(sc, i, EMU_CHAN_VTFT, 0);
		emuxki_write(sc, i, EMU_CHAN_CVCF, 0);
		emuxki_write(sc, i, EMU_CHAN_PTRX, 0);
		emuxki_write(sc, i, EMU_CHAN_CPF, 0);
	}

	/*
	 * Deallocate Emu10k1 caches and recording buffers. Again it will be
	 * removed because it will be done in voice shutdown.
	 */
	emuxki_write(sc, 0, EMU_MICBS, EMU_RECBS_BUFSIZE_NONE);
	emuxki_write(sc, 0, EMU_MICBA, 0);
	emuxki_write(sc, 0, EMU_FXBS, EMU_RECBS_BUFSIZE_NONE);
	emuxki_write(sc, 0, EMU_FXBA, 0);
	emuxki_write(sc, 0, EMU_FXWC, 0);
	emuxki_write(sc, 0, EMU_ADCBS, EMU_RECBS_BUFSIZE_NONE);
	emuxki_write(sc, 0, EMU_ADCBA, 0);

	/*
	 * XXX I don't know yet how I will handle tank cache buffer,
	 * I don't even clearly  know what it is for.
	 */
	emuxki_write(sc, 0, EMU_TCB, 0);	/* 16K again */
	emuxki_write(sc, 0, EMU_TCBS, 0);

	emuxki_write(sc, 0, EMU_DBG, 0x8000);	/* necessary ? */

	emuxki_dmamem_free(sc->silentpage, M_DEVBUF);
	emuxki_dmamem_free(sc->ptb, M_DEVBUF);
}

/* Emu10k1 Memory managment */

struct emuxki_mem *
emuxki_mem_new(struct emuxki_softc *sc, int ptbidx,
		size_t size, int type, int flags)
{
	struct emuxki_mem *mem;

	if ((mem = malloc(sizeof(*mem), type, flags)) == NULL)
		return (NULL);

	mem->ptbidx = ptbidx;
	if ((mem->dmamem = emuxki_dmamem_alloc(sc->sc_dmat, size,
	    EMU_DMA_ALIGN, EMU_DMAMEM_NSEG, type, flags)) == NULL) {
		free(mem, type);
		return (NULL);
	}
	return (mem);
}

void
emuxki_mem_delete(struct emuxki_mem *mem, int type)
{
	emuxki_dmamem_free(mem->dmamem, type);
	free(mem, type);
}

void *
emuxki_pmem_alloc(struct emuxki_softc *sc, size_t size, int type, int flags)
{
	int             i, j, s;
	size_t          numblocks;
	struct emuxki_mem *mem;
	u_int32_t      *ptb, silentpage;

	ptb = KERNADDR(sc->ptb);
	silentpage = DMAADDR(sc->silentpage) << 1;
	numblocks = size / EMU_PTESIZE;
	if (size % EMU_PTESIZE)
		numblocks++;

	for (i = 0; i < EMU_MAXPTE; i++)
		if ((ptb[i] & EMU_CHAN_MAP_PTE_MASK) == silentpage) {
			/* We look for a free PTE */
			s = splaudio();
			for (j = 0; j < numblocks; j++)
				if ((ptb[i + j] & EMU_CHAN_MAP_PTE_MASK)
				    != silentpage)
					break;
			if (j == numblocks) {
				if ((mem = emuxki_mem_new(sc, i,
						size, type, flags)) == NULL) {
					splx(s);
					return (NULL);
				}
				for (j = 0; j < numblocks; j++)
					ptb[i + j] =
						(((DMAADDR(mem->dmamem) +
						 j * EMU_PTESIZE)) << 1)
						| (i + j);
				LIST_INSERT_HEAD(&(sc->mem), mem, next);
				splx(s);
				return (KERNADDR(mem->dmamem));
			} else
				i += j;
			splx(s);
		}
	return (NULL);
}

void *
emuxki_rmem_alloc(struct emuxki_softc *sc, size_t size, int type, int flags)
{
	struct emuxki_mem *mem;
	int             s;

	mem = emuxki_mem_new(sc, EMU_RMEM, size, type, flags);
	if (mem == NULL)
		return (NULL);

	s = splaudio();
	LIST_INSERT_HEAD(&(sc->mem), mem, next);
	splx(s);

	return (KERNADDR(mem->dmamem));
}

/*
 * emuxki_channel_* : Channel managment functions
 * emuxki_chanparms_* : Channel parameters modification functions
 */

/*
 * is splaudio necessary here, can the same voice be manipulated by two
 * different threads at a time ?
 */
void
emuxki_chanparms_set_defaults(struct emuxki_channel *chan)
{
	chan->fxsend.a.level = chan->fxsend.b.level =
	chan->fxsend.c.level = chan->fxsend.d.level = 0xc0;	/* not max */
	chan->fxsend.a.dest = 0x0;
	chan->fxsend.b.dest = 0x1;
	chan->fxsend.c.dest = 0x2;
	chan->fxsend.d.dest = 0x3;

	chan->pitch.initial = 0x0000;	/* shouldn't it be 0xE000 ? */
	chan->pitch.current = 0x0000;	/* should it be 0x0400 */
	chan->pitch.target = 0x0000;	/* the unity pitch shift ? */
	chan->pitch.envelope_amount = 0x00;	/* none */

	chan->initial_attenuation = 0x00;	/* no attenuation */
	chan->volume.current = 0x0000;	/* no volume */
	chan->volume.target = 0xffff;
	chan->volume.envelope.current_state = 0x8000;	/* 0 msec delay */
	chan->volume.envelope.hold_time = 0x7f;	/* 0 msec */
	chan->volume.envelope.attack_time = 0x7F;	/* 5.5msec */
	chan->volume.envelope.sustain_level = 0x7F;	/* full  */
	chan->volume.envelope.decay_time = 0x7F;	/* 22msec  */

	chan->filter.initial_cutoff_frequency = 0xff;	/* no filter */
	chan->filter.current_cutoff_frequency = 0xffff;	/* no filtering */
	chan->filter.target_cutoff_frequency = 0xffff;	/* no filtering */
	chan->filter.lowpass_resonance_height = 0x0;
	chan->filter.interpolation_ROM = 0x1;	/* full band */
	chan->filter.envelope_amount = 0x7f;	/* none */
	chan->filter.LFO_modulation_depth = 0x00;	/* none */

	chan->loop.start = 0x000000;
	chan->loop.end = 0x000010;	/* Why ? */

	chan->modulation.envelope.current_state = 0x8000;
	chan->modulation.envelope.hold_time = 0x00;	/* 127 better ? */
	chan->modulation.envelope.attack_time = 0x00;	/* infinite */
	chan->modulation.envelope.sustain_level = 0x00;	/* off */
	chan->modulation.envelope.decay_time = 0x7f;	/* 22 msec */
	chan->modulation.LFO_state = 0x8000;

	chan->vibrato_LFO.state = 0x8000;
	chan->vibrato_LFO.modulation_depth = 0x00;	/* none */
	chan->vibrato_LFO.vibrato_depth = 0x00;
	chan->vibrato_LFO.frequency = 0x00;	/* Why set to 24 when
						 * initialized ? */

	chan->tremolo_depth = 0x00;
}

/* only call it at splaudio */
struct emuxki_channel *
emuxki_channel_new(struct emuxki_voice *voice, u_int8_t num)
{
	struct emuxki_channel *chan;

	chan = malloc(sizeof(struct emuxki_channel), M_DEVBUF, M_WAITOK);
	if (chan == NULL)
		return (NULL);

	chan->voice = voice;
	chan->num = num;
	emuxki_chanparms_set_defaults(chan);
	chan->voice->sc->channel[num] = chan;
	return (chan);
}

/* only call it at splaudio */
void
emuxki_channel_delete(struct emuxki_channel *chan)
{
	chan->voice->sc->channel[chan->num] = NULL;
	free(chan, M_DEVBUF);
}

void
emuxki_channel_set_fxsend(struct emuxki_channel *chan,
			   struct emuxki_chanparms_fxsend *fxsend)
{
	/* Could do a memcpy ...*/
	chan->fxsend.a.level = fxsend->a.level;
	chan->fxsend.b.level = fxsend->b.level;
	chan->fxsend.c.level = fxsend->c.level;
	chan->fxsend.d.level = fxsend->d.level;
	chan->fxsend.a.dest = fxsend->a.dest;
	chan->fxsend.b.dest = fxsend->b.dest;
	chan->fxsend.c.dest = fxsend->c.dest;
	chan->fxsend.d.dest = fxsend->d.dest;
}

void
emuxki_channel_set_srate(struct emuxki_channel *chan, u_int32_t srate)
{
	chan->pitch.target = (srate << 8) / 375;
	chan->pitch.target = (chan->pitch.target >> 1) +
		(chan->pitch.target & 1);
	chan->pitch.target &= 0xffff;
	chan->pitch.current = chan->pitch.target;
	chan->pitch.initial =
		(emuxki_rate_to_pitch(srate) >> 8) & EMU_CHAN_IP_MASK;
}

/* voice params must be set before calling this */
void
emuxki_channel_set_bufparms(struct emuxki_channel *chan,
			     u_int32_t start, u_int32_t end)
{
	u_int8_t        shift;
	struct emuxki_voice *voice = chan->voice;

	shift = voice->stereo + voice->b16;
	chan->loop.start = start & EMU_CHAN_PSST_LOOPSTARTADDR_MASK;
	chan->loop.end = end & EMU_CHAN_DSL_LOOPENDADDR_MASK;
}

void
emuxki_channel_commit_parms(struct emuxki_channel *chan)
{
	struct emuxki_voice *voice = chan->voice;
	struct emuxki_softc *sc = voice->sc;
	u_int32_t start, mapval;
	u_int8_t chano = chan->num;
	int s;

	start = chan->loop.start +
		(voice->stereo ? 28 : 30) * (voice->b16 + 1);
	mapval = DMAADDR(sc->silentpage) << 1 | EMU_CHAN_MAP_PTI_MASK;

	s = splaudio();
	emuxki_write(sc, chano, EMU_CHAN_CPF_STEREO, voice->stereo);
	emuxki_write(sc, chano, EMU_CHAN_FXRT,
		(chan->fxsend.d.dest << 28) | (chan->fxsend.c.dest << 24) |
		(chan->fxsend.b.dest << 20) | (chan->fxsend.a.dest << 16));
	emuxki_write(sc, chano, 0x10000000 | EMU_CHAN_PTRX,
		(chan->fxsend.a.level << 8) | chan->fxsend.b.level);
	emuxki_write(sc, chano, EMU_CHAN_DSL,
		(chan->fxsend.d.level << 24) | chan->loop.end);
	emuxki_write(sc, chano, EMU_CHAN_PSST,
		(chan->fxsend.c.level << 24) | chan->loop.start);
	emuxki_write(sc, chano, EMU_CHAN_CCCA,
		(chan->filter.lowpass_resonance_height << 28) |
		(chan->filter.interpolation_ROM << 25) |
		(voice->b16 ? 0 : EMU_CHAN_CCCA_8BITSELECT) | start);
	emuxki_write(sc, chano, EMU_CHAN_Z1, 0);
	emuxki_write(sc, chano, EMU_CHAN_Z2, 0);
	emuxki_write(sc, chano, EMU_CHAN_MAPA, mapval);
	emuxki_write(sc, chano, EMU_CHAN_MAPB, mapval);
	emuxki_write(sc, chano, EMU_CHAN_CVCF_CURRFILTER,
		chan->filter.current_cutoff_frequency);
	emuxki_write(sc, chano, EMU_CHAN_VTFT_FILTERTARGET,
		chan->filter.target_cutoff_frequency);
	emuxki_write(sc, chano, EMU_CHAN_ATKHLDM,
		(chan->modulation.envelope.hold_time << 8) |
		chan->modulation.envelope.attack_time);
	emuxki_write(sc, chano, EMU_CHAN_DCYSUSM,
		(chan->modulation.envelope.sustain_level << 8) |
		chan->modulation.envelope.decay_time);
	emuxki_write(sc, chano, EMU_CHAN_LFOVAL1,
		chan->modulation.LFO_state);
	emuxki_write(sc, chano, EMU_CHAN_LFOVAL2,
		chan->vibrato_LFO.state);
	emuxki_write(sc, chano, EMU_CHAN_FMMOD,
		(chan->vibrato_LFO.modulation_depth << 8) |
		chan->filter.LFO_modulation_depth);
	emuxki_write(sc, chano, EMU_CHAN_TREMFRQ,
		(chan->tremolo_depth << 8));
	emuxki_write(sc, chano, EMU_CHAN_FM2FRQ2,
		(chan->vibrato_LFO.vibrato_depth << 8) |
		chan->vibrato_LFO.frequency);
	emuxki_write(sc, chano, EMU_CHAN_ENVVAL,
		chan->modulation.envelope.current_state);
	emuxki_write(sc, chano, EMU_CHAN_ATKHLDV,
		(chan->volume.envelope.hold_time << 8) |
		chan->volume.envelope.attack_time);
	emuxki_write(sc, chano, EMU_CHAN_ENVVOL,
		chan->volume.envelope.current_state);
	emuxki_write(sc, chano, EMU_CHAN_PEFE,
		(chan->pitch.envelope_amount << 8) |
		chan->filter.envelope_amount);
	splx(s);
}

void
emuxki_channel_start(struct emuxki_channel *chan)
{
	struct emuxki_voice *voice = chan->voice;
	struct emuxki_softc *sc = voice->sc;
	u_int8_t        cache_sample, cache_invalid_size, chano = chan->num;
	u_int32_t       sample;
	int             s;

	cache_sample = voice->stereo ? 4 : 2;
	sample = voice->b16 ? 0x00000000 : 0x80808080;
	cache_invalid_size = (voice->stereo ? 28 : 30) * (voice->b16 + 1);

	s = splaudio();
	while (cache_sample--) {
		emuxki_write(sc, chano, EMU_CHAN_CD0 + cache_sample,
			sample);
	}
	emuxki_write(sc, chano, EMU_CHAN_CCR_CACHEINVALIDSIZE, 0);
	emuxki_write(sc, chano, EMU_CHAN_CCR_READADDRESS, 64);
	emuxki_write(sc, chano, EMU_CHAN_CCR_CACHEINVALIDSIZE,
		cache_invalid_size);
	emuxki_write(sc, chano, EMU_CHAN_IFATN,
		(chan->filter.target_cutoff_frequency << 8) |
		chan->initial_attenuation);
	emuxki_write(sc, chano, EMU_CHAN_VTFT_VOLUMETARGET,
		chan->volume.target);
	emuxki_write(sc, chano, EMU_CHAN_CVCF_CURRVOL,
		chan->volume.current);
	emuxki_write(sc, 0,
		EMU_MKSUBREG(1, chano, EMU_SOLEL + (chano >> 5)),
		0);	/* Clear stop on loop */
	emuxki_write(sc, 0,
		EMU_MKSUBREG(1, chano, EMU_CLIEL + (chano >> 5)),
		0);	/* Clear loop interrupt */
	emuxki_write(sc, chano, EMU_CHAN_DCYSUSV,
		(chan->volume.envelope.sustain_level << 8) |
		chan->volume.envelope.decay_time);
	emuxki_write(sc, chano, EMU_CHAN_PTRX_PITCHTARGET,
		chan->pitch.target);
	emuxki_write(sc, chano, EMU_CHAN_CPF_PITCH,
		chan->pitch.current);
	emuxki_write(sc, chano, EMU_CHAN_IP, chan->pitch.initial);

	splx(s);
}

void
emuxki_channel_stop(struct emuxki_channel *chan)
{
	int s;
	u_int8_t chano = chan->num;
	struct emuxki_softc *sc = chan->voice->sc;

	s = splaudio();
	emuxki_write(sc, chano, EMU_CHAN_PTRX_PITCHTARGET, 0);
	emuxki_write(sc, chano, EMU_CHAN_CPF_PITCH, 0);
	emuxki_write(sc, chano, EMU_CHAN_IFATN_ATTENUATION, 0xff);
	emuxki_write(sc, chano, EMU_CHAN_VTFT_VOLUMETARGET, 0);
	emuxki_write(sc, chano, EMU_CHAN_CVCF_CURRVOL, 0);
	emuxki_write(sc, chano, EMU_CHAN_IP, 0);
	splx(s);
}

/*
 * Voices managment
 * emuxki_voice_dataloc : use(play or rec) independant dataloc union helpers
 * emuxki_voice_channel_* : play part of dataloc union helpers
 * emuxki_voice_recsrc_* : rec part of dataloc union helpers
 */

/* Allocate channels for voice in case of play voice */
int
emuxki_voice_channel_create(struct emuxki_voice *voice)
{
	struct emuxki_channel **channel = voice->sc->channel;
	u_int8_t i, stereo = voice->stereo;
	int s;

	for (i = 0; i < EMU_NUMCHAN; i += stereo + 1) {
		if ((stereo && (channel[i + 1] != NULL)) ||
		    (channel[i] != NULL))	/* Looking for free channels */
			continue;
		s = splaudio();
		if (stereo) {
			voice->dataloc.chan[1] =
				emuxki_channel_new(voice, i + 1);
			if (voice->dataloc.chan[1] == NULL) {
				splx(s);
				return (ENOMEM);
			}
		}
		voice->dataloc.chan[0] = emuxki_channel_new(voice, i);
		if (voice->dataloc.chan[0] == NULL) {
			if (stereo) {
				emuxki_channel_delete(voice->dataloc.chan[1]);
				voice->dataloc.chan[1] = NULL;
			}
			splx(s);
			return (ENOMEM);
		}
		splx(s);
		return (0);
	}
	return (EAGAIN);
}

/* When calling this function we assume no one can access the voice */
void
emuxki_voice_channel_destroy(struct emuxki_voice *voice)
{
	emuxki_channel_delete(voice->dataloc.chan[0]);
	voice->dataloc.chan[0] = NULL;
	if (voice->stereo)
		emuxki_channel_delete(voice->dataloc.chan[1]);
	voice->dataloc.chan[1] = NULL;
}

/*
 * Will come back when used in voice_dataloc_create
 */
#if 0
int
emuxki_recsrc_reserve(struct emuxki_voice *voice, emuxki_recsrc_t source)
{
	if (voice->emu->recsrc[source] != NULL)
		return (EBUSY);
	voice->emu->recsrc[source] = voice;
	return (0);
}
#endif

/* When calling this function we assume the voice is stopped */
void
emuxki_voice_recsrc_release(struct emuxki_softc *sc, emuxki_recsrc_t source)
{
	sc->recsrc[source] = NULL;
}

int
emuxki_voice_dataloc_create(struct emuxki_voice *voice)
{
	int             error;

	if (voice->use & EMU_VOICE_USE_PLAY) {
		if ((error = emuxki_voice_channel_create(voice)))
			return (error);
	} else {
		/*
		 * Commented out because i don't know how to get the selected
		 * recording source
		 */
#if 0
		if (emuxki_recsrc_reserve(voice, recsrc))
			return (EBUSY);
		printf("Which rec src do i have to create!!!\n");
#endif
		return (EBUSY);	/* just return an error, no real meaning */
	}
	return (0);
}

void
emuxki_voice_dataloc_destroy(struct emuxki_voice *voice)
{
	if (voice->use & EMU_VOICE_USE_PLAY) {
		if (voice->dataloc.chan[0] != NULL)
			emuxki_voice_channel_destroy(voice);
	} else {
		if (voice->dataloc.source != EMU_RECSRC_NOTSET) {
			emuxki_voice_recsrc_release(voice->sc,
						     voice->dataloc.source);
			voice->dataloc.source = EMU_RECSRC_NOTSET;
		}
	}
}

struct emuxki_voice *
emuxki_voice_new(struct emuxki_softc *sc, u_int8_t use)
{
	struct emuxki_voice *voice;
	int             s;

	s = splaudio();
	voice = sc->lvoice;
	sc->lvoice = NULL;
	splx(s);

	if (!voice) {
		if (!(voice = malloc(sizeof(*voice), M_DEVBUF, M_WAITOK)))
			return (NULL);
		voice->sc = sc;
		voice->state = !EMU_VOICE_STATE_STARTED;
		voice->stereo = EMU_VOICE_STEREO_NOTSET;
		voice->b16 = 0;
		voice->sample_rate = 0;
		if (use & EMU_VOICE_USE_PLAY)
			voice->dataloc.chan[0] = voice->dataloc.chan[0] = NULL;
		else
			voice->dataloc.source = EMU_RECSRC_NOTSET;
		voice->buffer = NULL;
		voice->blksize = 0;
		voice->trigblk = 0;
		voice->blkmod = 0;
		voice->inth = NULL;
		voice->inthparam = NULL;
	}
	voice->use = use;

	s = splaudio();
	LIST_INSERT_HEAD((&sc->voices), voice, next);
	splx(s);

	return (voice);
}

void
emuxki_voice_delete(struct emuxki_voice *voice)
{
	struct emuxki_softc *sc = voice->sc;
	struct emuxki_voice *lvoice;
	int s;

	if (voice->state & EMU_VOICE_STATE_STARTED)
		emuxki_voice_halt(voice);

	s = splaudio();
	LIST_REMOVE(voice, next);
	lvoice = sc->lvoice;
	sc->lvoice = voice;
	splx(s);

	if (lvoice) {
		emuxki_voice_dataloc_destroy(lvoice);
		free(lvoice, M_DEVBUF);
	}
}

int
emuxki_voice_set_stereo(struct emuxki_voice *voice, u_int8_t stereo)
{
	int	error;
	struct emuxki_chanparms_fxsend fxsend;

	emuxki_voice_dataloc_destroy(voice);
	voice->stereo = stereo;
	if ((error = emuxki_voice_dataloc_create(voice)))
	  return (error);
	if (voice->use & EMU_VOICE_USE_PLAY) {
		fxsend.a.dest = 0x0;
		fxsend.b.dest = 0x1;
		fxsend.c.dest = 0x2;
		fxsend.d.dest = 0x3;
		if (voice->stereo) {
			fxsend.a.level = fxsend.c.level = 0xc0;
			fxsend.b.level = fxsend.d.level = 0x00;
			emuxki_channel_set_fxsend(voice->dataloc.chan[0],
						   &fxsend);
			fxsend.a.level = fxsend.c.level = 0x00;
			fxsend.b.level = fxsend.d.level = 0xc0;
			emuxki_channel_set_fxsend(voice->dataloc.chan[1],
						   &fxsend);
		} /* No else : default is good for mono */	
	}
	return (0);
}

int
emuxki_voice_set_srate(struct emuxki_voice *voice, u_int32_t srate)
{
	if (voice->use & EMU_VOICE_USE_PLAY) {
		if ((srate < 4000) || (srate > 48000))
			return (EINVAL);
		voice->sample_rate = srate;
		emuxki_channel_set_srate(voice->dataloc.chan[0], srate);
		if (voice->stereo)
			emuxki_channel_set_srate(voice->dataloc.chan[1],
						  srate);
	} else {
#ifdef EMUXKI_DEBUG
		printf("Recording voice set_srate not implemented\n");
#endif
		return (EINVAL);
	}
	return (0);
}

int
emuxki_voice_set_audioparms(struct emuxki_voice *voice, u_int8_t stereo,
			     u_int8_t b16, u_int32_t srate)
{
	int             error;

	/*
	 * Audio driver tried to set recording AND playing params even if
	 * device opened in play or record only mode ==>
	 * modified emuxki_set_params.
	 * Stays here for now just in case ...
	 */
	if (voice == NULL) {
#ifdef EMUXKI_DEBUG
		printf("warning: tried to set unallocated voice params !!\n");
#endif
		return (0);
	}

	if (voice->stereo == stereo && voice->b16 == b16 &&
	    voice->sample_rate == srate)
		return (0);

#ifdef EMUXKI_DEBUG
	printf("Setting %s voice params : %s, %u bits, %u hz\n",
	       (voice->use & EMU_VOICE_USE_PLAY) ? "play" : "record",
	       stereo ? "stereo" : "mono", (b16 + 1) * 8, srate);
#endif
	
	if (voice->stereo != stereo) {
		if ((error = emuxki_voice_set_stereo(voice, stereo)))
			return (error);
	 }
	voice->b16 = b16;
	if (voice->sample_rate != srate)
		emuxki_voice_set_srate(voice, srate);
	return (0);
}

/* voice audio parms (see just before) must be set prior to this */
int
emuxki_voice_set_bufparms(struct emuxki_voice *voice, void *ptr,
			   u_int32_t bufsize, u_int16_t blksize)
{
	struct emuxki_mem *mem;
	struct emuxki_channel **chan;
	u_int32_t start, end;
	u_int8_t sample_size;
	int error = EFAULT;

	LIST_FOREACH(mem, &voice->sc->mem, next) {
		if (KERNADDR(mem->dmamem) != ptr)
			continue;

		voice->buffer = mem;
		sample_size = (voice->b16 + 1) * (voice->stereo + 1);
		voice->blksize = blksize / sample_size;
		voice->trigblk = 0;	/* This shouldn't be needed */
		voice->blkmod = bufsize / blksize;
		if (bufsize % blksize) 	  /* This should not happen */
			voice->blkmod++;
		error = 0;

		if (voice->use & EMU_VOICE_USE_PLAY) {
			chan = voice->dataloc.chan;
			start = mem->ptbidx << 12;
			end = start + bufsize / sample_size;
			emuxki_channel_set_bufparms(chan[0],
						     start, end);
			if (voice->stereo)
				emuxki_channel_set_bufparms(chan[1],
				     start, end);
			voice->timerate = (u_int32_t) 48000 *
			                voice->blksize / voice->sample_rate;
			if (voice->timerate < 5)
				error = EINVAL;
		} else {
#ifdef EMUXKI_DEBUG
			printf("Rec voice set bufparms not implemented\n");
#endif
			error = ENODEV;
		}

		break;
	}

	return (error);
}

void
emuxki_voice_commit_parms(struct emuxki_voice *voice)
{
	if (voice->use & EMU_VOICE_USE_PLAY) {
		emuxki_channel_commit_parms(voice->dataloc.chan[0]);
		if (voice->stereo)
			emuxki_channel_commit_parms(voice->dataloc.chan[1]);
	}
}

u_int32_t
emuxki_voice_curaddr(struct emuxki_voice *voice)
{
	if (voice->use & EMU_VOICE_USE_PLAY)
		return (emuxki_read(voice->sc,
				     voice->dataloc.chan[0]->num,
				     EMU_CHAN_CCCA_CURRADDR) -
			voice->dataloc.chan[0]->loop.start);
	return (0);
}

void
emuxki_resched_timer(struct emuxki_softc *sc)
{
	struct emuxki_voice *voice;
	u_int16_t       timerate = 1024;
	u_int8_t	active = 0;
	int s;

	s = splaudio();
	LIST_FOREACH(voice, &sc->voices, next) {
		if ((voice->use & EMU_VOICE_USE_PLAY) == 0 ||
		    (voice->state & EMU_VOICE_STATE_STARTED) == 0)
			continue;
		active = 1;
		if (voice->timerate < timerate)
			timerate = voice->timerate;
	}

	if (timerate & ~EMU_TIMER_RATE_MASK)
		timerate = 0;
	bus_space_write_4(sc->sc_iot, sc->sc_ioh, EMU_TIMER, timerate);
	if (!active && (sc->timerstate & EMU_TIMER_STATE_ENABLED)) {
		bus_space_write_4(sc->sc_iot, sc->sc_ioh, EMU_INTE,
			bus_space_read_4(sc->sc_iot, sc->sc_ioh, EMU_INTE) &
			~EMU_INTE_INTERTIMERENB);
		sc->timerstate &= ~EMU_TIMER_STATE_ENABLED;
	} else if (active && !(sc->timerstate & EMU_TIMER_STATE_ENABLED)) {
		bus_space_write_4(sc->sc_iot, sc->sc_ioh, EMU_INTE,
			bus_space_read_4(sc->sc_iot, sc->sc_ioh, EMU_INTE) |
			EMU_INTE_INTERTIMERENB);
		sc->timerstate |= EMU_TIMER_STATE_ENABLED;
	}
	splx(s);
}

void
emuxki_voice_start(struct emuxki_voice *voice,
		    void (*inth) (void *), void *inthparam)
{
	voice->inth = inth;
	voice->inthparam = inthparam;
	if (voice->use & EMU_VOICE_USE_PLAY) {
		voice->trigblk = 1;
		emuxki_channel_start(voice->dataloc.chan[0]);
		if (voice->stereo)
			emuxki_channel_start(voice->dataloc.chan[1]);
	}
#ifdef EMUXKI_DEBUG
	else
		printf("Recording voice start not implemented\n");
#endif
	voice->state |= EMU_VOICE_STATE_STARTED;
	if (voice->use & EMU_VOICE_USE_PLAY)
		emuxki_resched_timer(voice->sc);
}

void
emuxki_voice_halt(struct emuxki_voice *voice)
{
	if (voice->use & EMU_VOICE_USE_PLAY) {
		emuxki_channel_stop(voice->dataloc.chan[0]);
		if (voice->stereo)
			emuxki_channel_stop(voice->dataloc.chan[1]);
	}
#ifdef EMUXKI_DEBUG
	else
		printf("Recording voice halt not implemented\n");
#endif
	voice->state &= ~EMU_VOICE_STATE_STARTED;
	if (voice->use & EMU_VOICE_USE_PLAY)
		emuxki_resched_timer(voice->sc);
}

/*
 * The interrupt handler
 */
int
emuxki_intr(void *arg)
{
	struct emuxki_softc *sc = arg;
	u_int32_t       ipr, curblk, us = 0;
	struct emuxki_voice *voice;

	while ((ipr = bus_space_read_4(sc->sc_iot, sc->sc_ioh, EMU_IPR))) {
		if (ipr & EMU_IPR_INTERVALTIMER) {
			LIST_FOREACH(voice, &sc->voices, next) {
				if ((voice->use & EMU_VOICE_USE_PLAY)==0 ||
				    (voice->state &
				      EMU_VOICE_STATE_STARTED) == 0)
					continue;

				curblk = emuxki_voice_curaddr(voice) /
				       voice->blksize;
				if (curblk == voice->trigblk) {
					voice->inth(voice->inthparam);
					voice->trigblk++;
					voice->trigblk %= voice->blkmod;
				}
			}
			us = 1;
		}

		/* Got interrupt */
		bus_space_write_4(sc->sc_iot, sc->sc_ioh, EMU_IPR, ipr);
	}

	return (us);
}


/*
 * Audio Architecture callbacks
 */

int
emuxki_open(void *addr, int flags)
{
	struct emuxki_softc *sc = addr;

#ifdef EMUXKI_DEBUG
	printf("%s: emuxki_open called\n", sc->sc_dev.dv_xname);
#endif

	/*
	 * Multiple voice support would be added as soon as I find a way to
	 * trick the audio arch into supporting multiple voices.
	 * Or I might integrate a modified audio arch supporting
	 * multiple voices.
	 */

	/*
	 * I did this because i have problems identifying the selected
	 * recording source(s) which is necessary when setting recording
	 * params This will be adressed very soon
	 */
	if (flags & AUOPEN_READ)
		return (EOPNOTSUPP);

	if (flags & AUOPEN_WRITE) {
		sc->pvoice = emuxki_voice_new(sc, EMU_VOICE_USE_PLAY);
		if (sc->pvoice == NULL) {
			if (flags & AUOPEN_READ)
				emuxki_voice_delete(sc->rvoice);
			return (EBUSY);
		}
	}

	return (0);
}

void
emuxki_close(void *addr)
{
	struct emuxki_softc *sc = addr;

#ifdef EMUXKI_DEBUG
	printf("%s: emu10K1_close called\n", sc->sc_dev.dv_xname);
#endif

	/* No multiple voice support for now */
	if (sc->rvoice != NULL)
		emuxki_voice_delete(sc->rvoice);
	sc->rvoice = NULL;
	if (sc->pvoice != NULL)
		emuxki_voice_delete(sc->pvoice);
	sc->pvoice = NULL;
}

int
emuxki_query_encoding(void *addr, struct audio_encoding *fp)
{
#ifdef EMUXKI_DEBUG
	struct emuxki_softc *sc = addr;

	printf("%s: emuxki_query_encoding called\n", sc->sc_dev.dv_xname);
#endif

	switch (fp->index) {
	case 0:
		strcpy(fp->name, AudioEulinear);
		fp->encoding = AUDIO_ENCODING_ULINEAR;
		fp->precision = 8;
		fp->flags = 0;
		break;
	case 1:
		strcpy(fp->name, AudioEmulaw);
		fp->encoding = AUDIO_ENCODING_ULAW;
		fp->precision = 8;
		fp->flags = AUDIO_ENCODINGFLAG_EMULATED;
		break;
	case 2:
		strcpy(fp->name, AudioEalaw);
		fp->encoding = AUDIO_ENCODING_ALAW;
		fp->precision = 8;
		fp->flags = AUDIO_ENCODINGFLAG_EMULATED;
		break;
	case 3:
		strcpy(fp->name, AudioEslinear);
		fp->encoding = AUDIO_ENCODING_SLINEAR;
		fp->precision = 8;
		fp->flags = AUDIO_ENCODINGFLAG_EMULATED;
		break;
	case 4:
		strcpy(fp->name, AudioEslinear_le);
		fp->encoding = AUDIO_ENCODING_SLINEAR_LE;
		fp->precision = 16;
		fp->flags = 0;
		break;
	case 5:
		strcpy(fp->name, AudioEulinear_le);
		fp->encoding = AUDIO_ENCODING_ULINEAR_LE;
		fp->precision = 16;
		fp->flags = AUDIO_ENCODINGFLAG_EMULATED;
		break;
	case 6:
		strcpy(fp->name, AudioEslinear_be);
		fp->encoding = AUDIO_ENCODING_SLINEAR_BE;
		fp->precision = 16;
		fp->flags = AUDIO_ENCODINGFLAG_EMULATED;
		break;
	case 7:
		strcpy(fp->name, AudioEulinear_be);
		fp->encoding = AUDIO_ENCODING_ULINEAR_BE;
		fp->precision = 16;
		fp->flags = AUDIO_ENCODINGFLAG_EMULATED;
		break;
	default:
		return (EINVAL);
	}
	return (0);
}

int
emuxki_set_vparms(struct emuxki_voice *voice, struct audio_params *p)
{
	u_int8_t        b16, mode;

	mode = (voice->use & EMU_VOICE_USE_PLAY) ?
		AUMODE_PLAY : AUMODE_RECORD;
	p->factor = 1;
	p->sw_code = NULL;
	if (p->channels != 1 && p->channels != 2)
		return (EINVAL);/* Will change when streams come in use */

	switch (p->encoding) {
	case AUDIO_ENCODING_ULAW:
		if (mode == AUMODE_PLAY) {
			p->factor = 2;
			p->sw_code = mulaw_to_slinear16_le;
			b16 = 1;
		} else {
			p->sw_code = ulinear8_to_mulaw;
			b16 = 0;
		}
		break;

	case AUDIO_ENCODING_ALAW:
		if (mode == AUMODE_PLAY) {
			p->factor = 2;
			p->sw_code = alaw_to_slinear16_le;
			b16 = 1;
		} else {
			p->sw_code = ulinear8_to_alaw;
			b16 = 0;
		}
		break;

	case AUDIO_ENCODING_SLINEAR_LE:
		if (p->precision == 8)
			p->sw_code = change_sign8;
		b16 = (p->precision == 16);
		break;

	case AUDIO_ENCODING_ULINEAR_LE:
		if (p->precision == 16)
			p->sw_code = change_sign16_le;
		b16 = (p->precision == 16);
		break;

	case AUDIO_ENCODING_SLINEAR_BE:
		if (p->precision == 16)
			p->sw_code = swap_bytes;
		else
			p->sw_code = change_sign8;
		b16 = (p->precision == 16);
		break;

	case AUDIO_ENCODING_ULINEAR_BE:
		if (p->precision == 16) {
			if (mode == AUMODE_PLAY)
				p->sw_code = swap_bytes_change_sign16_le;
			else
				p->sw_code = change_sign16_swap_bytes_le;
		}
		b16 = (p->precision == 16);
		break;

	default:
		return (EINVAL);
	}

	return (emuxki_voice_set_audioparms(voice, p->channels == 2,
				     b16, p->sample_rate));
}

int
emuxki_set_params(void *addr, int setmode, int usemode,
		   struct audio_params *play, struct audio_params *rec)
{
	struct emuxki_softc *sc = addr;
	int             mode, error;
	struct audio_params *p;

	for (mode = AUMODE_RECORD; mode != -1;
	     mode = mode == AUMODE_RECORD ? AUMODE_PLAY : -1) {
		if ((usemode & setmode & mode) == 0)
			continue;

		p = (mode == AUMODE_PLAY) ? play : rec;

		/* No multiple voice support for now */
		if ((error = emuxki_set_vparms((mode == AUMODE_PLAY) ?
						sc->pvoice : sc->rvoice, p)))
			return (error);
	}

	return (0);
}

int
emuxki_halt_output(void *addr)
{
	struct emuxki_softc *sc = addr;

	/* No multiple voice support for now */
	if (sc->pvoice == NULL)
		return (ENXIO);

	emuxki_voice_halt(sc->pvoice);
	return (0);
}

int
emuxki_halt_input(void *addr)
{
	struct emuxki_softc *sc = addr;

#ifdef EMUXKI_DEBUG
	printf("%s: emuxki_halt_input called\n", sc->sc_dev.dv_xname);
#endif

	/* No multiple voice support for now */
	if (sc->rvoice == NULL)
		return (ENXIO);
	emuxki_voice_halt(sc->rvoice);
	return (0);
}

int
emuxki_getdev(void *v, struct audio_device *adp)
{
	struct emuxki_softc *sc = v;
	*adp = sc->sc_audv;
	return 0;
}

int
emuxki_set_port(void *addr, mixer_ctrl_t *mctl)
{
	struct emuxki_softc *sc = addr;

	return sc->codecif->vtbl->mixer_set_port(sc->codecif, mctl);
}

int
emuxki_get_port(void *addr, mixer_ctrl_t *mctl)
{
	struct emuxki_softc *sc = addr;

	return sc->codecif->vtbl->mixer_get_port(sc->codecif, mctl);
}

int
emuxki_query_devinfo(void *addr, mixer_devinfo_t *minfo)
{
	struct emuxki_softc *sc = addr;

	return sc->codecif->vtbl->query_devinfo(sc->codecif, minfo);
}

void *
emuxki_allocm(void *addr, int direction, size_t size, int type, int flags)
{
	struct emuxki_softc *sc = addr;

	if (direction == AUMODE_PLAY)
		return emuxki_pmem_alloc(sc, size, type, flags);
	else
		return emuxki_rmem_alloc(sc, size, type, flags);
}

void
emuxki_freem(void *addr, void *ptr, int type)
{
	struct emuxki_softc *sc = addr;
	int             i, s;
	struct emuxki_mem *mem;
	size_t          numblocks;
	u_int32_t      *ptb, silentpage;

	ptb = KERNADDR(sc->ptb);
	silentpage = DMAADDR(sc->silentpage) << 1;
	LIST_FOREACH(mem, &sc->mem, next) {
		if (KERNADDR(mem->dmamem) != ptr)
			continue;

		s = splaudio();
		if (mem->ptbidx != EMU_RMEM) {
			numblocks = DMASIZE(mem->dmamem) / EMU_PTESIZE;
			if (DMASIZE(mem->dmamem) % EMU_PTESIZE)
				numblocks++;
			for (i = 0; i < numblocks; i++)
				ptb[mem->ptbidx + i] =
					silentpage | (mem->ptbidx + i);
		}
		LIST_REMOVE(mem, next);
		splx(s);

		emuxki_mem_delete(mem, type);
		break;
	}
}

size_t
emuxki_round_buffersize(void *addr, int direction, size_t bsize)
{
	static const int recbuf_sz[] = {
		0, 384, 448, 512, 640, 768, 896, 1024, 1280, 1536, 1792,
		2048, 2560, 3072, 3584, 4096, 5120, 6144, 7168, 8192, 10240,
		12288, 14366, 16384, 20480, 24576, 28672, 32768, 40960, 49152,
		57344, 65536
	};

	if (direction == AUMODE_PLAY) {
		if (bsize < EMU_PTESIZE)
			bsize = EMU_PTESIZE;
		else if (bsize > (EMU_PTESIZE * EMU_MAXPTE))
			bsize = EMU_PTESIZE * EMU_MAXPTE;
		/* Would be better if set to max available */
		else if (bsize % EMU_PTESIZE)
			bsize = bsize -
				(bsize % EMU_PTESIZE) +
				EMU_PTESIZE;
	} else {
		int idx;

		/* find nearest lower recbuf size */
		for(idx=32; --idx >= 0; ) {
			if (bsize >= recbuf_sz[idx]) {
				bsize = recbuf_sz[idx];
				break;
			}
		}

		if (bsize == 0)
			bsize = 384;
	}

	return (bsize);
}

paddr_t
emuxki_mappage(void *addr, void *ptr, off_t off, int prot)
{
	struct emuxki_softc *sc = addr;
	struct emuxki_mem *mem;
	u_int32_t      *ptb;

	ptb = KERNADDR(sc->ptb);
	LIST_FOREACH(mem, &sc->mem, next) {
		if (KERNADDR(mem->dmamem) == ptr) {
			struct dmamem *dm = mem->dmamem;

			return bus_dmamem_mmap(dm->dmat, dm->segs, dm->nsegs,
			       off, prot, BUS_DMA_WAITOK);
		}
	}

	return (-1);
}

int
emuxki_get_props(void *addr)
{
	return (AUDIO_PROP_MMAP | AUDIO_PROP_INDEPENDENT |
		AUDIO_PROP_FULLDUPLEX);
}

int
emuxki_trigger_output(void *addr, void *start, void *end, int blksize,
		       void (*inth) (void *), void *inthparam,
		       struct audio_params *params)
{
	struct emuxki_softc *sc = addr;
	/* No multiple voice support for now */
	struct emuxki_voice *voice = sc->pvoice;
	int             error;

	if (voice == NULL)
		return (ENXIO);
	if ((error = emuxki_set_vparms(voice, params)))
		return (error);
	if ((error = emuxki_voice_set_bufparms(voice, start,
				(caddr_t)end - (caddr_t)start, blksize)))
		return (error);
	emuxki_voice_commit_parms(voice);
	emuxki_voice_start(voice, inth, inthparam);

	return (0);
}

int
emuxki_trigger_input(void *addr, void *start, void *end, int blksize,
		      void (*inth) (void *), void *inthparam,
		      struct audio_params *params)
{
	struct emuxki_softc *sc = addr;
	/* No multiple voice support for now */
	struct emuxki_voice *voice = sc->rvoice;
	int             error;

	if (voice == NULL)
		return (ENXIO);
	if ((error = emuxki_set_vparms(voice, params)))
		return (error);
	if ((error = emuxki_voice_set_bufparms(voice, start,
						(caddr_t)end - (caddr_t)start,
						blksize)))
		return (error);
	emuxki_voice_commit_parms(voice); /* Useless for record ? */
	emuxki_voice_start(voice, inth, inthparam);

	return (0);
}


/*
 * AC97 callbacks
 */

int
emuxki_ac97_attach(void *arg, struct ac97_codec_if *codecif)
{
	struct emuxki_softc *sc = arg;

	sc->codecif = codecif;
	return (0);
}

int
emuxki_ac97_read(void *arg, u_int8_t reg, u_int16_t *val)
{
	struct emuxki_softc *sc = arg;
	int s;

	s = splaudio();
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, EMU_AC97ADDR, reg);
	*val = bus_space_read_2(sc->sc_iot, sc->sc_ioh, EMU_AC97DATA);
	splx(s);

	return (0);
}

int
emuxki_ac97_write(void *arg, u_int8_t reg, u_int16_t val)
{
	struct emuxki_softc *sc = arg;
	int s;

	s = splaudio();
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, EMU_AC97ADDR, reg);
	bus_space_write_2(sc->sc_iot, sc->sc_ioh, EMU_AC97DATA, val);
	splx(s);

	return (0);
}

void
emuxki_ac97_reset(void *arg)
{
}