/*
 * FIXME
 */

#ifndef _HFC_FIFO_H
#define _HFC_FIFO_H

static inline u16 Z_inc(struct hfc_chan_simplex *chan, u16 z, u16 inc)
{
	// declared as u32 in order to manage overflows
	u32 newz = z + inc;
	if (newz > chan->z_max)
		newz -= chan->fifo_size;

	return newz;
}

static inline u8 F_inc(struct hfc_chan_simplex *chan, u8 f, u8 inc)
{
	// declared as u16 in order to manage overflows
	u16 newf = f + inc;
	if (newf > chan->f_max)
		newf -= chan->f_num;

	return newf;
}

static inline u16 hfc_fifo_used_rx(struct hfc_chan_simplex *chan)
{
	return (chan->z1 - chan->z2 + chan->fifo_size) % chan->fifo_size;
}

static inline u16 hfc_fifo_get_frame_size(struct hfc_chan_simplex *chan)
{
 // This +1 is needed because in frame mode the available bytes are Z2-Z1+1
 // while in transparent mode I wouldn't consider the byte pointed by Z2 to
 // be available, otherwise, the FIFO would always contain one byte, even
 // when Z1==Z2

	return hfc_fifo_used_rx(chan) + 1;
}

/*
static inline u8 hfc_fifo_u8(struct hfc_chan_simplex *chan, u16 z)
{
	return *((u8 *)(chan->z_base + z));
}
*/
static inline u16 hfc_fifo_used_tx(struct hfc_chan_simplex *chan)
{
	return (chan->z1 - chan->z2 + chan->fifo_size) % chan->fifo_size;
}

static inline u16 hfc_fifo_free_rx(struct hfc_chan_simplex *chan)
{
	u16 free_bytes=chan->z2 - chan->z1;

	if (free_bytes > 0)
		return free_bytes;
	else
		return free_bytes + chan->fifo_size;
}

static inline u16 hfc_fifo_free_tx(struct hfc_chan_simplex *chan)
{
	u16 free_bytes=chan->z2 - chan->z1;

	if (free_bytes > 0)
		return free_bytes;
	else
		return free_bytes + chan->fifo_size;
}

static inline int hfc_fifo_has_frames(struct hfc_chan_simplex *chan)
{
	return chan->f1 != chan->f2;
}

static inline u8 hfc_fifo_used_frames(struct hfc_chan_simplex *chan)
{
	return (chan->f1 - chan->f2 + chan->f_num) % chan->f_num;
}

static inline u8 hfc_fifo_free_frames(struct hfc_chan_simplex *chan)
{
	u8 free_frames = chan->f2 - chan->f1;
	if (free_frames > 0)
		return free_frames;
	else
		return free_frames + chan->f_num;
}

static inline void hfc_fifo_reset(struct hfc_card *card)
{
	hfc_outb(card, hfc_A_INC_RES_FIFO,
		hfc_A_INC_RES_FIFO_V_RES_F);

	hfc_wait_busy(card);
}


static inline void hfc_fifo_refresh_fz_cache(struct hfc_chan_simplex *chan)
{
	struct hfc_card *card = chan->chan->port->card;

	// Se hfc-8s-4s.pdf par 4.4.7 for an explanation of this:
	u16 prev_f1f2 = hfc_inw(card, hfc_A_F12);;
	do {
		chan->f1f2 = hfc_inw(card, hfc_A_F12);
	} while(chan->f1f2 != prev_f1f2);

	chan->z1z2 = hfc_inl(card, hfc_A_Z12);
}

// This function and all subsequent accesses to the selected FIFO must be done
// in interrupt handler or inside a spin_lock_irq* protected section
static inline void hfc_fifo_select(struct hfc_chan_simplex *chan)
{
//	WARN_ON(!irqs_disabled() || !in_interrupt());

	hfc_outb(chan->chan->port->card, hfc_R_FIFO,
		hfc_R_FIFO_V_FIFO_ID(chan->fifo_id));
//		hfc_R_FIFO_V_REV);

	hfc_wait_busy(chan->chan->port->card);
	hfc_fifo_refresh_fz_cache(chan);
}

void hfc_fifo_clear_rx(struct hfc_chan_simplex *chan);
void hfc_fifo_clear_tx(struct hfc_chan_simplex *chan);
int hfc_fifo_get(struct hfc_chan_simplex *chan, void *data, int size);
void hfc_fifo_put(struct hfc_chan_simplex *chan, void *data, int size);
void hfc_fifo_drop(struct hfc_chan_simplex *chan, int size);
int hfc_fifo_get_frame(struct hfc_chan_simplex *chan, void *data, int max_size);
void hfc_fifo_drop_frame(struct hfc_chan_simplex *chan);
void hfc_fifo_put_frame(struct hfc_chan_simplex *chan, void *data, int size);

#endif