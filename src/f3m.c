// made in 2015 by GreaseMonkey - Public Domain
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#ifdef TARGET_GBA
#define TARGET_EMBEDDED
#endif
#ifdef TARGET_PSX
#define TARGET_EMBEDDED
#endif

#ifdef TARGET_EMBEDDED
#define assert(...)
#else
#include <assert.h>
#endif

#ifndef F3M_FREQ
#define F3M_FREQ 32768
//define F3M_FREQ 16384
#endif
#ifndef F3M_BUFLEN
#define F3M_BUFLEN 546
//define F3M_BUFLEN 273
#endif
#ifndef F3M_CHNS
#define F3M_CHNS 1
#endif
#ifndef F3M_VCHNS
#define F3M_VCHNS 20
#endif
#define F3M_PRIO_NORMAL 50
#define F3M_PRIO_MUSIC_OFF 100
#define F3M_PRIO_MUSIC 0x7FFF

#ifdef TARGET_EMBEDDED

#if 0
void __assert_func(const char *a, int b, const char *c, const char *d)
{
	for(;;);
}
#endif

// for -O3
#if 0
void *memset(void *b, int c, size_t len)
{
	size_t i;
	unsigned char rc = (unsigned char)c;
	uint8_t *bb = (uint8_t *)b;
	uint32_t cw = ((uint32_t)rc) * 0x01010101;

	for(i = 0; i < len-3; i += 4)
		*(uint32_t *)(bb+i) = cw;
	for(; i < len; i++)
		bb[i] = rc;

	return b;
}
#endif
#endif

typedef struct ins
{
	uint8_t typ;
	uint8_t fname[12];
	uint8_t dat_para_h;
	uint16_t dat_para;
	uint32_t len, lpbeg, lpend;
	uint8_t vol, rsv1, pack, flags;
	uint32_t c4freq;
	uint8_t rsv2[12];
	uint8_t name[28];
	uint8_t magic[4];
} __attribute__((__packed__)) ins_s;

typedef struct mod
{
	uint8_t name[28];
	uint8_t magic[4];
	uint16_t ord_num, ins_num, pat_num;
	uint16_t flags, ver, smptyp;
	uint8_t magic_scrm[4];
	uint8_t gvol, ispeed, itempo, mvol;
	uint8_t uclick, defpanFC;
	uint8_t unused1[8];
	uint16_t special;
	uint8_t cset[32];
	uint8_t extra[];
}__attribute__((__packed__)) mod_s;
extern mod_s fsys_s3m_test[];

typedef struct vchn
{
#ifdef TARGET_PSX
	uint16_t spu_data;
	uint16_t spu_data_lpbeg;
#else
	const uint8_t *data;
#endif
	int32_t len;
	int32_t len_loop;

	int32_t period;
	int32_t gxx_period;

	int32_t freq;
	int32_t offs;
	uint16_t suboffs;
	int16_t priority;

	int8_t vol;
	uint8_t pan;

	uint8_t vib_offs;
	uint8_t rtg_count;

	uint8_t eft, efp, lefp, last_note;
	uint8_t lins;
	uint8_t mem_gxx, mem_hxx, mem_oxx;
} vchn_s;

typedef struct player
{
	const mod_s *mod;
	const void *modbase;
	const uint16_t *ins_para;
	const uint16_t *pat_para;
	const uint8_t *ord_list;

	int32_t speed, tempo;
	int32_t ctick, tempo_samples, tempo_wait;
	int32_t cord, cpat, crow;
	const uint8_t *patptr;

	int sfxoffs;
	int ccount;

#ifdef TARGET_PSX
	uint16_t psx_spu_offset[99];
	uint16_t psx_spu_offset_lpbeg[99];
#endif

	vchn_s vchn[F3M_VCHNS];
} player_s;

const uint32_t period_amiclk = 8363*1712-400;
const uint16_t period_table[12] = {1712,1616,1524,1440,1356,1280,1208,1140,1076,1016,960,907};

// from ITTECH.TXT
static const int8_t f3m_sintab[64] = {
	  0,  6, 12, 19, 24, 30, 36, 41,
	 45, 49, 53, 56, 59, 61, 63, 64,
	 64, 64, 63, 61, 59, 56, 53, 49,
	 45, 41, 36, 30, 24, 19, 12,  6,
};

#ifdef F3M_ENABLE_DYNALOAD
mod_s *f3m_mod_dynaload_filename(const char *fname)
{
	int buf_max = 256;
	int buf_len = 0;

	FILE *fp = fopen(fname, "rb");
	assert(fp != NULL);
	char *buf = malloc(buf_max);

	for(;;)
	{
		if(buf_len >= buf_max)
		{
			buf_max = (buf_max*5/4+2);

			if(buf_len >= buf_max)
				buf_max = buf_len + 10;

			buf = realloc(buf, buf_max);
		}

		int v = fgetc(fp);
		if(v < 0) break;
		buf[buf_len++] = v;
	}

	fclose(fp);
	buf = realloc(buf, buf_len);
	return (mod_s *)buf;
}

void f3m_mod_free(mod_s *mod)
{
	free(mod);
}
#endif

static uint16_t f3m_get_para(const uint16_t *p)
{
	const uint8_t *p2 = (const uint8_t *)p;
	uint16_t v0 = p2[0];
	uint16_t v1 = p2[1];

	return (v1<<8)|v0;
}

static int32_t f3m_calc_tempo_samples(int32_t tempo)
{
	return (F3M_FREQ*10)/(tempo*4);
}

static int32_t f3m_calc_freq(int32_t freq)
{
#if F3M_FREQ == 32768
	freq <<= 1;
#else
#if F3M_FREQ == 16384
	freq <<= 2;
#else
	freq = (freq << 10) / (F3M_FREQ >> 6);
#endif
#endif
	return freq;

}

static int32_t f3m_calc_period_freq(int32_t period)
{
	int32_t freq = period_amiclk / period;
	return f3m_calc_freq(freq);
}

void f3m_player_init(player_s *player, mod_s *mod)
{
	int i;

	update_music_status(0, mod->ins_num);

	player->mod = mod;
	player->modbase = (const void *)mod;
	player->ord_list = (const uint8_t *)(((const char *)(mod+1)));
	player->ins_para = (const uint16_t *)(((const char *)(mod+1)) + mod->ord_num);
	player->pat_para = (const uint16_t *)(((const char *)(mod+1)) + mod->ord_num + mod->ins_num*2);

	player->speed = mod->ispeed;
	player->tempo = mod->itempo;
	player->ctick = player->speed;
	player->tempo_samples = f3m_calc_tempo_samples(player->tempo);
	player->tempo_wait = 0;

	player->cord = 0-1;
	player->cpat = 0;
	player->crow = 64;
	player->patptr = NULL;
	player->sfxoffs = 0;
	player->ccount = 16;

	for(i = 0; i < F3M_VCHNS; i++)
	{
		vchn_s *vchn = &(player->vchn[i]);

#ifdef TARGET_PSX
		vchn->spu_data = 0;
#else
		vchn->data = NULL;
#endif
		vchn->len = 0;
		vchn->len_loop = 0;

		vchn->freq = 0;
		vchn->offs = 0;
		vchn->suboffs = 0;
		vchn->priority = (i < player->ccount ? F3M_PRIO_MUSIC_OFF : 0);

		vchn->gxx_period = 0;
		vchn->period = 0;
		vchn->vol = 0;
		vchn->pan = ((mod->mvol&0x80)==0)
			? 0x8
			: (mod->defpanFC == 0xFC
				? ((const uint8_t *)(player->pat_para + mod->pat_num))[i] & 0xF
				: ((i&1)?0xC:0x3));

		vchn->vib_offs = 0;
		vchn->rtg_count = 0;

		vchn->eft = 0;
		vchn->efp = 0;
		vchn->lefp = 0;
		vchn->last_note = 0;
		vchn->lins = 0;

		vchn->mem_gxx = 0;
		vchn->mem_hxx = 0;
		vchn->mem_oxx = 0;
	}

#ifdef TARGET_PSX
	int j, k;
	volatile int lag;

	//volatile uint16_t *butt = (volatile uint16_t *)0x80100000;
	//*(butt++) = 0x1234;

	// load samples
	for(i = 0; i < 99; i++)
	{
		player->psx_spu_offset[i] = 0;
		player->psx_spu_offset_lpbeg[i] = 0;
	}

	uint16_t spu_offs = 0x01000>>3;
	uint16_t smp_data_buf[8];
	int smp_src_buf[28];
	int smp_data_last = 0;

	// FIXME: mednafen will murder you if write your SPU loading code like this and from what I gather so will a real PSX - pcsxr laps it up w/o any issues for some reason
	SPU_MEM_CNT = 0x0004;
	for(lag = 0; lag < 0x1000; lag++) ;
	SPU_CNT = 0x0000;
	//while((SPU_STAT & 0x3F) != 0x00) ;
	for(lag = 0; lag < 0x1000; lag++) ;
	SPU_MEM_ADDR = spu_offs;
	for(lag = 0; lag < 0x1000; lag++) ;

	for(i = 0; i < 99 && i < mod->ins_num; i++)
	{
		update_music_status(i, mod->ins_num);
		// TODO: subtly adjust samples so loops work properly

		// Get instrument + check if valid
		const ins_s *ins = ((void *)mod) + (((uint32_t)(f3m_get_para(&player->ins_para[i])))*16);
		uint32_t para = (((uint32_t)(ins->dat_para_h))<<16)|((uint32_t)(ins->dat_para));
		if(ins->len == 0 || para == 0)
			continue;

		int lpbeg = (((ins->flags & 0x01) != 0) ? ins->lpbeg : ins->len + 64);
		int lpend = (((ins->flags & 0x01) != 0) ? ins->lpend+1 : ins->len + 64);
		// Ensure the loop actually fires
		// Not sure if the assurance is really that good here!
		if((ins->flags & 0x01) != 0 && lpend > (int)ins->len-14)
			lpend = ins->len-14;

		const uint8_t *data = ((void *)mod) + (para*16);
		player->psx_spu_offset[i] = spu_offs;
		player->psx_spu_offset_lpbeg[i] = spu_offs;
		for(j = 0; j < 64000 && j < (int)ins->len; j += 28, data += 28, spu_offs += (0x10>>3))
		{
			// Load data
			int src_min = smp_data_last;
			int src_max = smp_data_last;

			for(k = 0; k < 28; k++)
			{
				int v = (j+k >= (int)ins->len ? 0 : (((int)(data[k]))-0x80)<<8);
				if(v < src_min) src_min = v;
				if(v > src_max) src_max = v;
				smp_src_buf[k] = v;
			}

			// Calculate shift
			int src_range = src_max - src_min;
			int shift = 0;
			while(src_range >= 16 && shift < 12)
			{
				shift++;
				src_range >>= 1;
			}

			// Clear old buffer
			for(k = 0; k < 8; k++)
				smp_data_buf[k] = 0;

			// Set header
			// applying filter 1 so we get a delta + soft LPF
			smp_data_buf[0] = (12-shift) | (1<<4) | ((0x00)<<8);
			if(j+14 >= lpbeg && j-14 < lpbeg)
			{
				smp_data_buf[0] |= 0x0400;
				player->psx_spu_offset_lpbeg[i] = spu_offs;
			}
			if(j+14 >= lpend && j-14 < lpend)
				smp_data_buf[0] |= 0x0300;

			// Add data
			for(k = 0; k < 28; k++)
			{
				int v = smp_src_buf[k];
				v -= smp_data_last;
				v = (v + (1<<(shift-1)))>>shift;
				if(v < -8) v = -8;
				if(v >  7) v =  7;

				smp_data_buf[1+(k>>2)] |= ((v&15)<<((k&3)<<2));
				smp_data_last += (v<<shift);
			}

			// Upload data
			//for(k = 0; k < 8; k++) *(butt++) = smp_data_buf[k];
			for(k = 0; k < 8; k++)
				SPU_MEM_DATA = smp_data_buf[k];
			SPU_CNT = 0x0010;
			//while((SPU_STAT & 0x3F) != 0x10) ;
			while((SPU_STAT & 0x0400) != 0) ;
		}

		// Upload silence
		SPU_MEM_DATA = 0x0500;
		for(k = 1; k < 8; k++)
			SPU_MEM_DATA = 0x0000;
		SPU_CNT = 0x0010;
		//while((SPU_STAT & 0x3F) != 0x10) ;
		while((SPU_STAT & 0x0400) != 0) ;
		spu_offs += 0x10>>3;
	}
#endif
}

static void f3m_player_eff_slide_vol(player_s *player, vchn_s *vchn, int isfirst)
{
	(void)player; // "player" is only there to check the fast slide flag (TODO!)

	uint8_t lefp = vchn->lefp;
	int samt = 0;

	if((lefp & 0xF0) == 0x00)
	{
		if((!isfirst) || lefp == 0x0F) samt = -(lefp & 0x0F);
	} else if((lefp & 0x0F) == 0x00) {
		if((!isfirst) || lefp == 0xF0) samt = lefp >> 4;
	} else if((lefp & 0x0F) == 0x0F) {
		if(isfirst) samt = lefp >> 4;
	} else if((lefp & 0xF0) == 0xF0) {
		if(isfirst) samt = -(lefp & 0x0F);
	} else {
		// default: slide down on nonzero ticks
		// SATELL.s3m relies on this
		if(!isfirst) samt = -(lefp & 0x0F);
	}

	if(samt > 0)
	{
		vchn->vol += samt;
		if(vchn->vol > 63) vchn->vol = 63;
	} else if(samt < 0) {
		if(vchn->vol < (uint8_t)-samt) vchn->vol = 0;
		else vchn->vol += samt;
	}
}

static void f3m_player_eff_slide_period(vchn_s *vchn, int amt)
{
	vchn->period += amt;
	vchn->freq = f3m_calc_period_freq(vchn->period);
}

static void f3m_player_eff_vibrato(vchn_s *vchn, int lefp, int shift)
{
	vchn->freq = f3m_calc_period_freq(vchn->period);

	int vspeed = (lefp>>4);
	int vdepth = (lefp&15)<<shift;

	// TODO: support other waveforms

	// TODO: find rounding + direction
	int vval = f3m_sintab[vchn->vib_offs&31];
	if(vchn->vib_offs & 32) vval = -vval;
	vval *= vdepth;
	vval += (1<<(5-1));
	vval >>= 5;

	vchn->freq = f3m_calc_period_freq(vchn->period + vval);
	vchn->vib_offs += vspeed;
}

static void f3m_note_retrig(player_s *player, vchn_s *vchn)
{
	int iidx = vchn->lins;
	const ins_s *ins = player->modbase + (((uint32_t)(f3m_get_para(&player->ins_para[iidx-1])))*16);
	uint32_t para = (((uint32_t)(ins->dat_para_h))<<16)|((uint32_t)(ins->dat_para));

	int note = vchn->last_note;
	vchn->gxx_period = ((8363 * 16 * period_table[note&15]) / ins->c4freq)
		>> (note>>4);

#ifdef TARGET_PSX
	vchn->spu_data = player->psx_spu_offset[iidx-1];
	vchn->spu_data_lpbeg = player->psx_spu_offset_lpbeg[iidx-1];
#else
	vchn->data = player->modbase + para*16;
#endif
	vchn->priority = F3M_PRIO_MUSIC;
	vchn->len = (((ins->flags & 0x01) != 0) && ins->lpend < ins->len
		? ins->lpend
		: ins->len);
	vchn->len_loop = (((ins->flags & 0x01) != 0) && ins->lpbeg < ins->len
		? vchn->len - ins->lpbeg
		: 0);

	// TODO: verify if this is the case wrt note-end
#ifdef TARGET_PSX
	if(vchn->spu_data == 0 || (vchn->eft != ('G'-'A'+1) && vchn->eft != ('L'-'A'+1)))
#else
	if(vchn->data == NULL || (vchn->eft != ('G'-'A'+1) && vchn->eft != ('L'-'A'+1)))
#endif
	{
		vchn->period = vchn->gxx_period;
		vchn->freq = f3m_calc_period_freq(vchn->period);
		vchn->offs = 0;
		if(vchn->eft == ('O'-'A'+1))
		{
			vchn->eft = 0;
			int lefp = (vchn->efp != 0 ? vchn->efp : vchn->mem_oxx);
			vchn->mem_oxx = lefp;
			lefp <<= 8;
			if(lefp < vchn->len)
				vchn->offs = lefp;
		}
		vchn->vib_offs = 0; // TODO: find correct retrig point
	}
}

void f3m_effect_nop(player_s *player, vchn_s *vchn, int tick, int pefp, int lefp)
{
	(void)player; (void)vchn; (void)tick; (void)pefp; (void)lefp;
}

void f3m_effect_Axx(player_s *player, vchn_s *vchn, int tick, int pefp, int lefp)
{
	(void)player; (void)vchn; (void)tick; (void)pefp; (void)lefp;

	if(tick == 0 && pefp >= 1)
		player->speed = pefp;
}

void f3m_effect_Bxx(player_s *player, vchn_s *vchn, int tick, int pefp, int lefp)
{
	(void)player; (void)vchn; (void)tick; (void)pefp; (void)lefp;

	if(tick == 0)
	{
		// TODO: handle Bxx/Cxx combined
		player->cord = pefp-1;
		player->crow = 64; 
	}
}

void f3m_effect_Cxx(player_s *player, vchn_s *vchn, int tick, int pefp, int lefp)
{
	(void)player; (void)vchn; (void)tick; (void)pefp; (void)lefp;

	if(tick == 0)
	{
		// TODO: actually look up the jump value
		player->crow = 64; 
	}
}

void f3m_effect_Dxx(player_s *player, vchn_s *vchn, int tick, int pefp, int lefp)
{
	(void)player; (void)vchn; (void)tick; (void)pefp; (void)lefp;

	f3m_player_eff_slide_vol(player, vchn, tick == 0);
}

void f3m_effect_Exx(player_s *player, vchn_s *vchn, int tick, int pefp, int lefp)
{
	(void)player; (void)vchn; (void)tick; (void)pefp; (void)lefp;

	if(tick == 0)
	{
		if(lefp >= 0xF0)
		{
			f3m_player_eff_slide_period(vchn, ((lefp & 0x0F)<<2));
		} else if(lefp >= 0xE0) {
			f3m_player_eff_slide_period(vchn, (lefp & 0x0F));
		}
	} else {
		if(lefp < 0xE0)
		{
			f3m_player_eff_slide_period(vchn, (lefp<<2));
		}
	}
}

void f3m_effect_Fxx(player_s *player, vchn_s *vchn, int tick, int pefp, int lefp)
{
	(void)player; (void)vchn; (void)tick; (void)pefp; (void)lefp;

	if(tick == 0)
	{
		if(lefp >= 0xF0)
		{
			f3m_player_eff_slide_period(vchn, -((lefp & 0x0F)<<2));
		} else if(lefp >= 0xE0) {
			f3m_player_eff_slide_period(vchn, -(lefp & 0x0F));
		}
	} else {
		if(lefp < 0xE0)
		{
			f3m_player_eff_slide_period(vchn, -(lefp<<2));
		}
	}
}

void f3m_effect_Gxx(player_s *player, vchn_s *vchn, int tick, int pefp, int lefp)
{
	(void)player; (void)vchn; (void)tick; (void)pefp; (void)lefp;

	if(tick == 0)
	{
		lefp = (pefp != 0 ? pefp : vchn->mem_gxx);
		vchn->mem_gxx = lefp;
	} else {
		lefp = vchn->mem_gxx;

		if(vchn->period < vchn->gxx_period)
		{
			vchn->period += lefp<<2;
			if(vchn->period > vchn->gxx_period)
				vchn->period = vchn->gxx_period;
			vchn->freq = f3m_calc_period_freq(vchn->period);

		} else if(vchn->period > vchn->gxx_period) {
			vchn->period -= lefp<<2;
			if(vchn->period < vchn->gxx_period)
				vchn->period = vchn->gxx_period;
			vchn->freq = f3m_calc_period_freq(vchn->period);
		}
	}
}

void f3m_effect_Hxx(player_s *player, vchn_s *vchn, int tick, int pefp, int lefp)
{
	(void)player; (void)vchn; (void)tick; (void)pefp; (void)lefp;

	if(tick == 0)
	{
		lefp = pefp;
		if((lefp&0x0F) == 0) lefp |= vchn->mem_hxx&0x0F;
		if((lefp&0xF0) == 0) lefp |= vchn->mem_hxx&0xF0;
		vchn->mem_hxx = lefp;
	} else {
		lefp = vchn->mem_hxx;

		f3m_player_eff_vibrato(vchn, lefp, 2);
	}
}

void f3m_effect_Kxx(player_s *player, vchn_s *vchn, int tick, int pefp, int lefp)
{
	(void)player; (void)vchn; (void)tick; (void)pefp; (void)lefp;

	if(tick != 0)
	{
		f3m_effect_Hxx(player, vchn, tick, 0, 0);
		f3m_effect_Dxx(player, vchn, tick, pefp, lefp);
	}
}

void f3m_effect_Lxx(player_s *player, vchn_s *vchn, int tick, int pefp, int lefp)
{
	(void)player; (void)vchn; (void)tick; (void)pefp; (void)lefp;

	if(tick != 0)
	{
		f3m_effect_Gxx(player, vchn, tick, 0, 0);
		f3m_effect_Dxx(player, vchn, tick, pefp, lefp);
	}
}

void f3m_effect_Qxx(player_s *player, vchn_s *vchn, int tick, int pefp, int lefp)
{
	(void)player; (void)vchn; (void)tick; (void)pefp; (void)lefp;

	// Notes:
	// 1. When effect is not Qxy, rtg_count is reset.
	// 2. Current y (from lefp, not special mem) is used as a threshold.
	// 3. When y is exceeded, change vol according to current x.

	int voldrop = (lefp>>4);
	int rtick = (lefp&15);

	if(rtick != 0 && vchn->rtg_count >= rtick)
	{
		// Retrigger
		// TODO: work out what happens when we've already done a period or vol slide
		// TODO: 
		f3m_note_retrig(player, vchn);

		if(voldrop < 8)
		{
			if(voldrop < 6)
			{
				vchn->vol -= (1<<voldrop);
				if(vchn->vol < 0) vchn->vol = 0;
			} else if(voldrop == 6) {
				// *2/3, which according to FC is exactly the same as 5/8
				vchn->vol = (vchn->vol*5)>>3;
			} else {
				// *1/2
				vchn->vol = vchn->vol>>1;
			}

		} else {
			voldrop -= 8;
			if(voldrop < 6)
			{
				vchn->vol += (1<<voldrop);
			} else if(voldrop == 6) {
				// *3/2
				vchn->vol = (vchn->vol*3)>>1;
			} else {
				// *2
				vchn->vol = vchn->vol<<1;
			}

			// XXX: do we deal with the case where vol > 63 before doubling?
			if(vchn->vol > 63) vchn->vol = 63;
		}

		vchn->rtg_count = 0;
	}

	vchn->rtg_count++;
}

void f3m_effect_Sxx(player_s *player, vchn_s *vchn, int tick, int pefp, int lefp)
{
	(void)player; (void)vchn; (void)tick; (void)pefp; (void)lefp;

	switch(lefp>>4)
	{
		case 0x8:
			if(tick == 0)
			if((player->mod->mvol&0x80)!=0)
			{
				vchn->pan = lefp & 0x0F;
			}
			break;

		case 0xC:
			// TODO confirm SC0 behaviour
			if(tick != 0 && (lefp&0x0F) == tick)
			{
#ifdef TARGET_PSX
				vchn->spu_data = 0;
#else
				vchn->data = NULL;
#endif
				vchn->priority = F3M_PRIO_MUSIC_OFF;
				vchn->vol = 0;
			}
			break;

		case 0xD:
			// TODO confirm SD0 behaviour
			if(tick != 0 && (lefp&0x0F) == tick)
			{
				f3m_note_retrig(player, vchn);
			}
			break;
	}
}

void f3m_effect_Txx(player_s *player, vchn_s *vchn, int tick, int pefp, int lefp)
{
	(void)player; (void)vchn; (void)tick; (void)pefp; (void)lefp;

	if(pefp >= 33)
	{
		player->tempo = pefp;
		player->tempo_samples = f3m_calc_tempo_samples(player->tempo);
	}

}

void f3m_effect_Uxx(player_s *player, vchn_s *vchn, int tick, int pefp, int lefp)
{
	(void)player; (void)vchn; (void)tick; (void)pefp; (void)lefp;

	if(tick == 0)
	{
		lefp = pefp;
		if((lefp&0x0F) == 0) lefp |= vchn->mem_hxx&0x0F;
		if((lefp&0xF0) == 0) lefp |= vchn->mem_hxx&0xF0;
		vchn->mem_hxx = lefp;
	} else {
		lefp = vchn->mem_hxx;

		f3m_player_eff_vibrato(vchn, lefp, 0);
	}
}


void (*(f3m_effect_tab[32]))(player_s *player, vchn_s *vchn, int tick, int pefp, int lefp) = {
	f3m_effect_nop, f3m_effect_Axx, f3m_effect_Bxx, f3m_effect_Cxx,
	f3m_effect_Dxx, f3m_effect_Exx, f3m_effect_Fxx, f3m_effect_Gxx,
	f3m_effect_Hxx, f3m_effect_nop, f3m_effect_nop, f3m_effect_Kxx,
	f3m_effect_Lxx, f3m_effect_nop, f3m_effect_nop, f3m_effect_nop,

	f3m_effect_nop, f3m_effect_Qxx, f3m_effect_nop, f3m_effect_Sxx,
	f3m_effect_Txx, f3m_effect_Uxx, f3m_effect_nop, f3m_effect_nop,
	f3m_effect_nop, f3m_effect_nop, f3m_effect_nop, f3m_effect_nop,
	f3m_effect_nop, f3m_effect_nop, f3m_effect_nop, f3m_effect_nop,
};

static void f3m_player_play_newnote(player_s *player)
{
	int i;

	// Advance row
	player->crow++;
	if(player->crow >= 64)
	{
		player->crow = 0;

		// Advance order
		player->cord++;
		while(player->cord < player->mod->ord_num && player->ord_list[player->cord] == 0xFE)
			player->cord++;
		if(player->cord >= player->mod->ord_num || player->ord_list[player->cord] == 0xFF)
			player->cord = 0;
		while(player->cord < player->mod->ord_num && player->ord_list[player->cord] == 0xFE)
			player->cord++;

		player->cpat = player->ord_list[player->cord];
		assert(player->cpat < 200);
		assert(player->cpat < player->mod->pat_num);

		// Get new pattern pointer
		player->patptr = player->modbase + (((uint32_t)(f3m_get_para(&player->pat_para[player->cpat])))*16);
		player->patptr += 2;
	}

	// Clear vchn pattern data
	for(i = 0; i < F3M_VCHNS; i++)
	{
		vchn_s *vchn = &(player->vchn[i]);
		vchn->eft = 0x00;
		vchn->efp = 0x00;
	}

	// Read pattern data
	if(player->patptr == NULL)
		return;

	const uint8_t *p = player->patptr;

	for(;;)
	{
		uint8_t cv = *(p++);
		if(cv == 0) break;
		vchn_s *vchn = &(player->vchn[cv&15]); // TODO proper channel map check?

		uint8_t pnote = 0xFF;
		uint8_t pins = 0x00;
		uint8_t pvol = 0xFF;
		uint8_t peft = 0x00;
		uint8_t pefp = 0x00;

		if((cv & 0x20) != 0)
		{
			pnote = *(p++);
			pins = *(p++);
		}

		if((cv & 0x40) != 0)
		{
			pvol = *(p++);
		}

		if((cv & 0x80) != 0)
		{
			peft = *(p++);
			pefp = *(p++);
			peft &= 0x1F;
		}

		vchn->eft = peft;
		vchn->efp = pefp;
		if(pefp != 0) vchn->lefp = pefp;
		uint8_t lefp = vchn->lefp;

		// TODO: DO THIS PROPERLY
		if(pnote == 0xFE)
		{
#ifdef TARGET_PSX
			vchn->spu_data = 0;
#else
			vchn->data = NULL;
#endif
			vchn->priority = F3M_PRIO_MUSIC_OFF;

		} else if((pnote < 0x80 && (pins != 0 || vchn->lins != 0))
				|| (pnote == 0xFF && pins != 0)) {
			int iidx = (pins == 0 ? vchn->lins : pins);
			vchn->lins = iidx;
			const ins_s *ins = player->modbase + (((uint32_t)(f3m_get_para(&player->ins_para[iidx-1])))*16);

			vchn->vol = (pvol != 0xFF ? pvol
				: pins != 0 ? ins->vol
				: vchn->vol);
			if(vchn->vol > 63) vchn->vol = 63; // lesser-known quirk

			// TODO: work out what happens on note end when ins but no note
#ifdef TARGET_PSX
			if(vchn->spu_data == 0 || pnote < 0x80)
#else
			if(vchn->data == NULL || pnote < 0x80)
#endif
			{
				int note = (pnote < 0x80 ? pnote : vchn->last_note);
				vchn->last_note = note;

				if(peft != ('S'-'A'+1) || (lefp&0xF0) != 0xD0)
					f3m_note_retrig(player, vchn);
			}
		}

		if((peft == ('S'-'A'+1) && (lefp&0xF0) == 0xD0) && pnote >= 0x80)
		{
			// Cancel effect if no note to trigger (e.g. CLICK.S3M)
			// TODO: Check if vol column has any effect
			vchn->eft = 0;
		}

		if(pvol < 0x80)
		{
			vchn->vol = pvol;
			if(vchn->vol > 63) vchn->vol = 63;
		}

		if(peft != ('Q'-'A'+1))
		{
			vchn->rtg_count = 0;
		}

		f3m_effect_tab[peft](player, vchn, 0, pefp, lefp);
	}

	player->patptr = p;
}

void f3m_player_play_newtick(player_s *player)
{
	int i;

	player->ctick++;
	if(player->ctick >= player->speed)
	{
		player->ctick = 0;
		f3m_player_play_newnote(player);
	} else {
		for(i = 0; i < F3M_VCHNS; i++)
		{
			vchn_s *vchn = &(player->vchn[i]);

			uint8_t peft = vchn->eft;
			uint8_t pefp = vchn->efp;
			uint8_t lefp = vchn->lefp;

			f3m_effect_tab[peft&31](player, vchn, player->ctick, pefp, lefp);
		}

	}
}

void f3m_sfx_play(player_s *player, int priority, const uint8_t *data, int len, int len_loop, int freq, int vol)
{
	int i;
	vchn_s *vchn;
	int got_vchn = 0;

	// Scan for free channels
	for(i = 0; i < F3M_VCHNS; i++)
	{
		vchn = &player->vchn[player->sfxoffs];
		player->sfxoffs++;
		if(player->sfxoffs >= F3M_VCHNS)
			player->sfxoffs = 0;

		if(vchn->priority == 0)
		{
			got_vchn = 1;
			break;
		}
	}

	// Scan for channels we can override
	if(!got_vchn) for(i = 0; i < F3M_VCHNS; i++)
	{
		vchn = &player->vchn[player->sfxoffs];
		player->sfxoffs++;
		if(player->sfxoffs >= F3M_VCHNS)
			player->sfxoffs = 0;

		if(vchn->priority <= priority)
		{
			got_vchn = 1;
			break;
		}
	}

	if(!got_vchn) return;

	// Set channel up to do our bidding
	vchn->freq = f3m_calc_freq(freq);
#ifdef TARGET_PSX
	vchn->spu_data = (int)data;
#else
	vchn->data = data;
#endif
	vchn->len = len;
	vchn->len_loop = len_loop;
	vchn->vol = vol;
	vchn->priority = priority;
	vchn->offs = 0;
}

void f3m_player_play(player_s *player, int32_t *mbuf, uint8_t *obuf)
{
	(void)mbuf;
	(void)obuf;
	int i, j;

	const int blen = F3M_BUFLEN;

#ifdef TARGET_GBA_DEBUG
	VPAL0[0] = 0x7FFF;
#endif

	// Check if we have a new tick
	while(player->tempo_wait < 0)
	{
		f3m_player_play_newtick(player);
		player->tempo_wait += player->tempo_samples;
	}

	player->tempo_wait -= blen;

#ifndef TARGET_PSX
	// Clear mixing buffer
	for(i = 0; i < blen*F3M_CHNS; i++)
		mbuf[i] = 0;
	
	// Do each channel
	for(i = 0; i < F3M_VCHNS; i++)
	{
		vchn_s *vchn = &(player->vchn[i]);

		// Channel enabled?
		if(vchn->data == NULL)
			continue;

		// Output sample
		int32_t *out = mbuf;
		const uint8_t *d = vchn->data;
		int32_t offs = vchn->offs;
		int32_t suboffs = vchn->suboffs;
		int32_t len = vchn->len;
#if F3M_CHNS == 2
		int32_t lvol = vchn->vol;
		int32_t rvol = vchn->vol;
		if(vchn->pan < 0x8)
		{
			lvol = (lvol*vchn->pan*2)/15;
		} else {
			rvol = (lvol*(15-vchn->pan)*2)/15;
		}
#else
		int32_t vol = vchn->vol;
#endif
		const int32_t freq = vchn->freq;
		for(j = 0; j < blen;)
		{
			// Work out the time to the end of the sample
			int32_t rem_len = (len - offs);
			int32_t endsmp = blen;

			// Quick calculation to determine if we need to do a slow calculation
			if(rem_len < (((blen-j)*freq + 0x20000)>>16))
			{
				// We can't just sprint through,
				// so cut it off at the right point.

				// NOTE: ARM7TDMI (and anything pre-ARMv7)
				// lacks a divide instruction!
				// Thus, it calls a function instead.
				//
				// With that said, it's probably cheaper
				// than doing the length check every sample.
				// (Plus we don't do it every update.)

				int32_t rem_sublen = (rem_len<<16) - suboffs;
				endsmp = (rem_sublen+freq-1)/freq;
				endsmp += j;
			}

			// TODO: find optimal threshold
			if(freq >= 0x8000)
			{
				for(; j < endsmp; j++)
				{
#if F3M_CHNS == 2
					out[j*2+0] += (int32_t)(lvol*((int32_t)(d[offs])-0x80));
					out[j*2+1] += (int32_t)(rvol*((int32_t)(d[offs])-0x80));
#else
#if F3M_CHNS == 1
					out[j] += (int32_t)(vol*(((int32_t)d[offs])-0x80));
#else
#error "F3M_CHNS must be 1 or 2"
#endif
#endif
					suboffs += freq;
					offs += suboffs>>16;
					suboffs &= 0xFFFF;
				}
			} else { 
				int32_t loffs = offs-1;
				int32_t lval = 0;
				for(; j < endsmp; j++)
				{
#if F3M_CHNS == 2
					// TODO: port optimisations to stereo
					out[j*2+0] += (int32_t)(lvol*((int32_t)(d[offs])-0x80));
					out[j*2+1] += (int32_t)(rvol*((int32_t)(d[offs])-0x80));
#else
#if F3M_CHNS == 1
					if(loffs != offs)
					{
						lval = (int32_t)(vol*(((int32_t)d[offs])-0x80));
						loffs = offs;

					}
					out[j] += lval;
#else
#error "F3M_CHNS must be 1 or 2"
#endif
#endif
					suboffs += freq;
					offs += suboffs>>16;
					suboffs &= 0xFFFF;
				}
			}

			// Know of any clever ways to speed this up?
			// Memory timings (16/32):
			// - 1/1 Fast RAM
			// - 1/2 VRAM
			// - 3/6 Slow RAM
			// - 3/6 GamePak ROM - SEQUENTIAL
			// - 5/8 GamePak ROM - RANDOM ACCESS
			// Memory timings according to GBATEK:
			if(offs >= len)
			{
				if(vchn->len_loop == 0)
				{
					vchn->data = NULL;
					vchn->priority = (i < player->ccount ? F3M_PRIO_MUSIC_OFF : 0);
					break;
				}

				offs -= vchn->len_loop;
			}
		}

		vchn->offs = offs;
		vchn->suboffs = suboffs;

	}

	for(i = 0; i < blen*F3M_CHNS; i++)
	{
		int32_t base = ((mbuf[i]+0x80)>>8)+0x80;
		if(base < 0x00) base = 0x00;
		if(base > 0xFF) base = 0xFF;
#ifdef TARGET_GBA
		obuf[i] = base ^ 0x80;
#else
		obuf[i] = base;
#endif
	}

#ifdef TARGET_GBA_DEBUG
	VPAL0[0] = 0x0000;
#endif
#else
	// TARGET_PSX.
	// We need to use hardware channels for this.
	uint32_t kon_mask = 0;

	for(i = 0; i < F3M_VCHNS; i++)
	{
		vchn_s *vchn = &(player->vchn[i]);

		// Channel enabled?
		// TODO: handle note offs properly
		if(vchn->spu_data == 0)
		{
			if((vchn->offs & 1) != 0)
			{
				vchn->offs &= ~1;
				SPU_KOFF = (1<<i);
			}

			continue;
		}

		// Output sample
		uint16_t spu_offs = vchn->spu_data;
		uint16_t spu_offs_lpbeg = vchn->spu_data_lpbeg;
		int32_t offs = vchn->offs;
		int32_t suboffs = vchn->suboffs;
		int32_t len = vchn->len;
		int32_t lvol = vchn->vol<<7;
		int32_t rvol = vchn->vol<<7;
		if(vchn->pan < 0x8)
		{
			lvol = (lvol*(vchn->pan*2+1))/15;
		} else {
			rvol = (rvol*((15-vchn->pan)*2+1))/15;
		}
		const int32_t freq = vchn->freq;

		if((vchn->offs & 1) == 0)
		{
			SPU_n_START(i) = spu_offs + (((offs+14)/28)<<1);
			SPU_n_REPEAT(i) = spu_offs_lpbeg;
			SPU_n_ADSR(i) = 0x9FC083FF;
			//SPU_KON = (1<<i);
			kon_mask |= (1<<i);
			vchn->offs |= 1;
		}

		SPU_n_MVOL_L(i) = lvol;
		SPU_n_MVOL_R(i) = rvol;
		SPU_n_PITCH(i) = freq>>4;

	}

	SPU_KON = kon_mask;

#endif
}

