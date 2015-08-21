#include <string.h>
#include <stdint.h>

void update_music_status(int ins, int ins_num);

#define PSX_IOBASE 0x00000000

#define JOY_TX_DATA (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801040))
#define JOY_RX_DATA (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801040))
#define JOY_STAT (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801044))
#define JOY_MODE (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801048))
#define JOY_CTRL (*(volatile uint16_t *)(PSX_IOBASE + 0x1F80104A))
#define JOY_BAUD (*(volatile uint16_t *)(PSX_IOBASE + 0x1F80104E))

#define I_STAT (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801070))
#define I_MASK (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801074))

#define GP0 (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801810))
#define GP1 (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801814))

#define SPU_n_MVOL_L(n) (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801C00 + (n)*16))
#define SPU_n_MVOL_R(n) (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801C02 + (n)*16))
#define SPU_n_PITCH(n) (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801C04 + (n)*16))
#define SPU_n_START(n) (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801C06 + (n)*16))
#define SPU_n_ADSR(n) (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801C08 + (n)*16))
#define SPU_n_REPEAT(n) (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801C0E + (n)*16))

#define TMR_n_COUNT(n) (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801100 + (n)*16))
#define TMR_n_MODE(n) (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801104 + (n)*16))
#define TMR_n_TARGET(n) (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801108 + (n)*16))

#define SPU_MVOL_L (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801D80))
#define SPU_MVOL_R (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801D82))
#define SPU_KON (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801D88))
#define SPU_KOFF (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801D8C))
#define SPU_PMON (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801D90))
#define SPU_ENDX (*(volatile uint32_t *)(PSX_IOBASE + 0x1F801D9C))
#define SPU_MEM_ADDR (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801DA6))
#define SPU_MEM_DATA (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801DA8))
#define SPU_CNT (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801DAA))
#define SPU_MEM_CNT (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801DAC))
#define SPU_STAT (*(volatile uint16_t *)(PSX_IOBASE + 0x1F801DAE))

#define busywait()

#define TARGET_PSX
#define F3M_FREQ 44100
#define F3M_BUFLEN 882
#define F3M_CHNS 2
#include "f3m.c"
player_s s3mplayer;

extern uint8_t fsys_rawcga[];
//extern mod_s fsys_s3m_test[];

#define InitHeap(a0, a1) ((void (*)(int, int, int))0xA0)(0x39, a0, a1);
#define malloc(a0) ((void *(*)(int, int))0xA0)(0x33, a0);
#define free(a0) ((void (*)(int, int))0xA0)(0x34, a0);

typedef int fixed;
#define M_PI ((fixed)0x0003243F)

int main(void);
void yield(void);

extern uint8_t _BSS_START[];
extern uint8_t _BSS_END[];

int screen_buffer = 0;

void aaa_start(void)
{
	int i;

	//for(i = 0; i < _BSS_START - _BSS_END; i++)
	//	_BSS_START[i] = 0;

	//InitHeap(0x100000, 0x0F0000);
	I_MASK = 0;
	main();

	for(;;)
		yield();
}

void yield(void)
{
	// TODO: halt 
}

static int fixtoi(fixed v)
{
	return v>>16;
}

static fixed itofix(int v)
{
	return v<<16;
}

static fixed fixmul(fixed a, fixed b)
{
	int sign = (a^b)&0x80000000;

	if((a&0x80000000) != 0) a = -a;
	if((b&0x80000000) != 0) b = -b;

	fixed al = (a & 0xFFFF);
	fixed ah = (a >> 16) & 0x7FFF;
	fixed bl = (b & 0xFFFF);
	fixed bh = (b >> 16) & 0x7FFF;

	// TODO: Handle overflow
	fixed r = ((bl*al + 0x8000)>>16)
		+ bl*ah + bh*al
		+ ((ah*bh) << 16);

	if(sign != 0) r = -r;

	return r;
}

static fixed fixsin(fixed ang)
{
	int i;

	// Clamp angle to 0 <= ang < 2 * M_PI
	if(ang < 0) ang = ((M_PI*2) - (-ang) % (M_PI*2)) % (M_PI*2);
	else ang = ang % (M_PI*2);

	// Now wrap to -M_PI <= ang < M_PI
	if(ang >= M_PI) ang -= M_PI*2;

	// Get sign bit and deal with corner
	int sign = ang & 0x80000000;
	if(sign != 0) ang = -ang;

	// Calculate
	fixed acc1 = ang;
	fixed acc2 = fixmul(ang, ang);
	fixed acc3 = fixmul(acc2, acc1)/6;
	fixed acc5 = fixmul(acc2, acc3)/20;
	fixed acc7 = fixmul(acc2, acc5)/42;
	fixed ret = acc1 - acc3 + acc5 - acc7;

	// Invert if originally negative
	//if(sign != 0) ret = -ret;

	// Return!
	return ret;
}

static fixed fixcos(fixed ang)
{
	return fixsin(ang + (M_PI/2));
}

void gpu_send_control_gp0(int v)
{
	int k;

	// Wait until no longer busy 
	// Or if we time out, just send the damn thing anyway
	//for(k = 0; k < 200; k++)
	for(;;)
		if((GP1 & 0x04000000) != 0)
			break;

	// Send to GPU 
	GP0 = v;
}

void gpu_send_control_gp1(int v)
{
	// Wait until no longer busy 
	//while((GP1 & 0x04000000) == 0)
		;

	// Send to GPU 
	GP1 = v;
}

void gpu_send_data(int v)
{
	// Wait until space available in buffer 
	//while((GP1 & 0x02000000) == 0)
	//	;

	// Send to GPU 
	GP0 = v;
}

void gpu_display_start(int x1, int y1)
{
	gpu_send_control_gp1(0x05000000 | (x1 & 0x3FF) | ((y1 & 0x1FF)<<10));
}

void gpu_crtc_range(int x1, int y1, int w, int h)
{
	gpu_send_control_gp1(0x06000000 | (x1 & 0xFFF) | (((x1+w) & 0xFFF)<<12));
	gpu_send_control_gp1(0x07000000 | (y1 & 0x3FF) | (((y1+h) & 0x3FF)<<10));
}

void gpu_draw_offset(int x, int y)
{
	gpu_send_control_gp0(0xE5000000 | (x & 0x7FF) | ((y & 0x7FF)<<11));
}

void gpu_draw_texmask(int w, int h, int ox, int oy)
{
	if((w&~7) != 0) w = ~((w-1)>>3);
	if((h&~7) != 0) h = ~((h-1)>>3);
	ox >>= 3;
	oy >>= 3;

	gpu_send_control_gp0(0xE2000000
		| ((w & 0x1F) << 0)
		| ((h & 0x1F) << 5)
		| ((ox & 0x1F) << 10)
		| ((oy & 0x1F) << 15)
		);
}

void gpu_draw_range(int x1, int y1, int x2, int y2)
{
	gpu_send_control_gp0(0xE3000000 | (x1 & 0x3FF) | ((y1 & 0x3FF)<<10));
	gpu_send_control_gp0(0xE4000000 | (x2 & 0x3FF) | ((y2 & 0x3FF)<<10));
}

void gpu_push_vertex(int x, int y)
{
	gpu_send_data((y<<16) | (x&0xFFFF));
}

static void update_frame(void)
{
	int i;

	// Clear screen
	//gpu_send_control_gp0((screen_buffer == 0 ? 0x027D7D7D : 0x024D4D4D));
	gpu_send_control_gp0(0x02001D00);
	gpu_send_data(0x00000000 + (screen_buffer<<16));
	gpu_send_data((320) | ((240)<<16));

	// Draw string
	gpu_send_control_gp1(0x01000000);
	uint8_t update_str_buf[64];
	sprintf(update_str_buf, "ord=%02i row=%02i speed=%03i/%i"
		, s3mplayer.cord
		, s3mplayer.crow
		, s3mplayer.tempo
		, s3mplayer.speed
		);
	for(i = 0; update_str_buf[i] != '\x00'; i++)
	{
		uint32_t test_char = update_str_buf[i];
		gpu_send_control_gp0(0xE1080208 | ((test_char>>5)));
		gpu_draw_texmask(8, 8, (test_char&31)<<3, 0);
		gpu_send_control_gp0(0x757F7F7F);
		gpu_push_vertex(i*8+16-160, 16-120);
		gpu_send_data(0x001C0000);
	}
	for(i = 0; i < 27; i++)
	{
		uint32_t test_char = s3mplayer.mod->name[i];
		gpu_send_control_gp0(0xE1080208 | ((test_char>>5)));
		gpu_draw_texmask(8, 8, (test_char&31)<<3, 0);
		gpu_send_control_gp0(0x757F7F7F);
		gpu_push_vertex(i*8+16-160, 16+8*1-120);
		gpu_send_data(0x001C0000);
	}

	while((GP1 & 0x10000000) == 0)
		{}

	// Flip pages
	gpu_display_start(0, screen_buffer + 8);
	screen_buffer = (screen_buffer == 0 ? 240 : 0);
	gpu_draw_range(0, screen_buffer, 320, 240 + screen_buffer);
	gpu_draw_offset(0 + 160, screen_buffer + 120);
}

void update_music_status(int ins, int ins_num)
{
	int i;
	int scroll_len = ((320-20*2)*ins)/ins_num;

	// Clear screen
	gpu_send_control_gp0(0x024D0000);
	gpu_send_data(0x00000000 + (screen_buffer<<16));
	gpu_send_data((320) | ((240)<<16));

	// Draw status bar
	if(scroll_len < 320)
	{
		gpu_send_control_gp1(0x01000000);
		gpu_send_control_gp0(0x02000000);
		gpu_send_data(0x00000000 + 20 + scroll_len + ((screen_buffer+120-5)<<16));
		gpu_send_data((320-20*2-scroll_len) | ((10)<<16));
	}

	if(scroll_len > 0)
	{
		gpu_send_control_gp1(0x01000000);
		gpu_send_control_gp0(0x02007F00);
		gpu_send_data(0x00000000 + 20 + ((screen_buffer+120-5)<<16));
		gpu_send_data((scroll_len) | ((10)<<16));
	}

	// Draw string
	gpu_send_control_gp1(0x01000000);
	const static uint8_t test_str[] = "Converting samples...";
	for(i = 0; test_str[i] != '\x00'; i++)
	{
		uint32_t test_char = test_str[i];
		gpu_send_control_gp0(0xE1080208 | ((test_char>>5)));
		gpu_draw_texmask(8, 8, (test_char&31)<<3, 0);
		gpu_send_control_gp0(0x757F7F7F);
		gpu_push_vertex(i*8+40-160, 120-5-8-4-120);
		gpu_send_data(0x001C0000);
	}

	while((GP1 & 0x10000000) == 0)
		{}

	// Flip pages
	gpu_display_start(0, screen_buffer + 8);
	screen_buffer = (screen_buffer == 0 ? 240 : 0);
	gpu_draw_range(0, screen_buffer, 320, 240 + screen_buffer);
	gpu_draw_offset(0 + 160, screen_buffer + 120);
}

int main(void)
{
	int i;
	volatile int k = 0;
	int x, y, xc, yc;

	// Reset GPU 
	gpu_send_control_gp1(0x00000000);

	// Fix up DMA 
	gpu_send_control_gp1(0x04000001);
	gpu_send_control_gp1(0x01000000);

	// Set display area 
	gpu_crtc_range(0x260, 0x88-(224/2), 320*8, 224);
	gpu_display_start(0, 8);

	// Set display mode 
	gpu_send_control_gp1(0x08000001);

	// Set draw mode 
	gpu_send_control_gp0(0xE6000000); gpu_send_control_gp0(0xE1000618); // Texpage
	gpu_draw_texmask(32, 32, 0, 0);
	gpu_draw_range(0, 0, 320, 240);

	// Copy CLUT to GPU
	gpu_send_control_gp1(0x01000000);
	gpu_send_control_gp0(0xA0000000);
	//gpu_send_data(0x01F70000);
	gpu_send_data(0x000001C0);
	gpu_send_data(0x00010002);
	//gpu_send_data(0x7FFF0001);
	gpu_send_data(0x7FFF0000);

	// Copy font to GPU
	gpu_send_control_gp1(0x01000000);
	gpu_send_control_gp0(0xA0000000);
	gpu_send_data(0x00000200);
	gpu_send_data(0x00080200);

	for(y = 0; y < 8; y++)
	for(x = 0; x < 256; x++)
	{
		uint32_t wdata = 0;
		for(i = 0; i < 8; i++, wdata <<= 4)
		if((fsys_rawcga[y+x*8]&(1<<i)) != 0)
			wdata |= 0x1;

		gpu_send_data(wdata);
	}

	// Enable display 
	gpu_send_control_gp1(0x03000000);

	// Clear screen 
	gpu_send_control_gp0(0x027D7D7D);
	gpu_send_data(0x00000000);
	gpu_send_data((320) | ((240)<<16));
	screen_buffer = 0;
	gpu_display_start(0, screen_buffer + 8);

	// Prep module
	f3m_player_init(&s3mplayer, fsys_s3m_test);

	SPU_CNT = 0xC000;
	SPU_MVOL_L = 0x3FFF;
	SPU_MVOL_R = 0x3FFF;

	// Set up timer
	//TMR_n_TARGET(2) = 42300;//42336;
	// pcsxr tends to get the timing wrong
	TMR_n_TARGET(2) = 42336;
	TMR_n_COUNT(2) = 0x0000;
	TMR_n_MODE(2) = 0x0608;
	k = TMR_n_MODE(2);

	for(;;)
	{
		while((TMR_n_MODE(2) & 0x0800) == 0) {}
		while((TMR_n_MODE(2) & 0x0800) != 0) {}
		while((TMR_n_MODE(2) & 0x0800) == 0) {}
		while((TMR_n_MODE(2) & 0x0800) != 0) {}
		f3m_player_play(&s3mplayer, NULL, NULL);
		update_frame();
	}

	// Set up joypad
	JOY_CTRL = 0x0000;
	JOY_MODE = 0x000D;
	JOY_BAUD = 0x0088;

	for(;;)
		yield();

	return 0;
}

