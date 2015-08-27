#include <string.h>
#include <stdint.h>
#include <math.h>

#include "psx.h"

typedef int fixed;
typedef fixed vec3[3];
typedef fixed vec4[4];
typedef vec4 mat4[4];

extern uint8_t fsys_rawcga[];

#include "fix.c"
#include "vec.c"
#include "gpu.c"
#include "gte.c"

void update_music_status(int ins, int ins_num);

#define TARGET_PSX
#define F3M_FREQ 44100
#define F3M_BUFLEN 882
#define F3M_CHNS 2
#include "f3m.c"
player_s s3mplayer;

//extern mod_s fsys_s3m_test[];

int main(void);
void yield(void);

extern volatile uint8_t _BSS_START[];
extern volatile uint8_t _BSS_END[];

void aaa_start(void)
{
	int i;

	I_MASK = 0;

	//memset(_BSS_START, 0, _BSS_END - _BSS_START);
	//for(i = 0; i < _BSS_END - _BSS_START; i++) _BSS_START[i] = 0;

	//InitHeap(0x100000, 0x0F0000);
	/*
	// this code does work, no guarantees that it won't clobber things though
	asm volatile (
		"\tlui $4, 0x0010\n"
		"\tlui $5, 0x000F\n"
		"\tori $9, $zero, 0x39\n"
		"\taddiu $sp, $sp, 0x10\n"
		"\tori $2, $zero, 0x00A0\n"
		"\tjalr $2\n"
		"\tnop\n"
		"\taddiu $sp, $sp, -0x10\n"
	);
	*/

	main();

	for(;;)
		yield();
}

void yield(void)
{
	// TODO: halt 
}

float tri_ang = 0;
uint16_t pad_old_data = 0xFFFF;

static void update_frame(void)
{
	volatile int lag;
	int i;

	// Clear screen
	//gpu_send_control_gp0((screen_buffer == 0 ? 0x027D7D7D : 0x024D4D4D));
	gpu_send_control_gp0(0x02001D00);
	gpu_send_data(0x00000000 + (screen_buffer<<16));
	gpu_send_data((320) | ((240)<<16));

	// Set up camera matrix
	mat4 mat_cam;
	mat4_load_identity(&mat_cam);
	mat4_rotate_z(&mat_cam, tri_ang);
	mat4_translate_imm3(&mat_cam, 0, 0, 0x100);
	gte_init_offset(0, 0, 120);
	gte_loadmat_rot_full(&mat_cam);

	// Draw spinny triangle
	gpu_send_control_gp1(0x01000000);
	gpu_send_control_gp0(0x2000007F);
	uint32_t xy0, xy1, xy2;
	const vec3 tri_data[3] = {
		{-50, -50, 0},
		{ 50, -50, 0},
		{  0,  50, 0},
	};
	gte_load_v012_vec3(&tri_data[0], &tri_data[1], &tri_data[2]);
	gte_cmd_rtpt();
	gte_save_s012xy_ui32_t(&xy0, &xy1, &xy2);
	gpu_send_data(xy0);
	gpu_send_data(xy1);
	gpu_send_data(xy2);
	tri_ang += FM_PI*2/180/2;
	for(lag = 0; lag < 0x300; lag++) {}

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

	// Read joypad
	JOY_CTRL = 0x0003;
	for(lag = 0; lag < 300; lag++) {}
	uint8_t v0 = joy_swap(0x01);
	uint8_t v1 = joy_swap(0x42);
	uint8_t v2 = joy_swap(0x00);
	uint8_t v3 = joy_swap(0x00);
	uint8_t v4 = joy_swap(0x00);
	JOY_CTRL = 0x0000;

	uint16_t pad_id   = (((uint16_t)v2)<<8)|v1;
	uint16_t pad_data = (((uint16_t)v4)<<8)|v3;
	pad_data = ~pad_data;

	if(((pad_data&~pad_old_data) & PAD_RIGHT) != 0)
	{
		if(s3mplayer.cord > s3mplayer.mod->ord_num-2)
			s3mplayer.cord = s3mplayer.mod->ord_num-2;
		s3mplayer.crow=64;
		s3mplayer.ctick = s3mplayer.speed;
	}

	if(((pad_data&~pad_old_data) & PAD_LEFT) != 0)
	{
		s3mplayer.cord -= 2;
		if(s3mplayer.cord < -1)
			s3mplayer.cord = -1;
		s3mplayer.crow=64;
		s3mplayer.ctick = s3mplayer.speed;
	}

	pad_old_data = pad_data;

	const char *pad_id_str = "Unknown";
	switch(pad_id)
	{
		case 0x5A12:
			// I've heard this one's rather buggy
			pad_id_str = "Mouse - FUCK YOU";
			break;
		case 0x5A41:
			pad_id_str = "Digital Pad";
			break;
		case 0x5A53:
			pad_id_str = "Analogue Stick";
			break;
		case 0x5A73:
			pad_id_str = "Analogue Pad";
			break;
		case 0xFFFF:
			pad_id_str = "NOTHING";
			break;
	}

	sprintf(update_str_buf, "joypad=%04X (%04X: %s)", pad_data, pad_id, pad_id_str);
	for(i = 0; update_str_buf[i] != '\x00'; i++)
	{
		uint32_t test_char = update_str_buf[i];
		gpu_send_control_gp0(0xE1080208 | ((test_char>>5)));
		gpu_draw_texmask(8, 8, (test_char&31)<<3, 0);
		gpu_send_control_gp0(0x757F7F7F);
		gpu_push_vertex(i*8+16-160, 16+8*3-120);
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
	scroll_len &= ~0xF;

	// Clear screen
	gpu_send_control_gp0(0x024D0000);
	gpu_send_data(0x00000000 + (screen_buffer<<16));
	gpu_send_data((320) | ((240)<<16));

	// Draw status bar
	if(scroll_len < 320-16*2)
	{
		gpu_send_control_gp1(0x01000000);
		gpu_send_control_gp0(0x02000000);
		gpu_send_data(0x00000000 + 16 + scroll_len + ((screen_buffer+120-5)<<16));
		gpu_send_data((320-16*2-scroll_len) | ((10)<<16));
	}

	if(scroll_len > 0)
	{
		gpu_send_control_gp1(0x01000000);
		gpu_send_control_gp0(0x02007F00);
		gpu_send_data(0x00000000 + 16 + ((screen_buffer+120-5)<<16));
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
	//gpu_crtc_range(0x260, 0x88-(224/2), 320*8, 224); // NTSC
	gpu_crtc_range(0x260, 0xA3-(224/2), 320*8, 224); // PAL
	gpu_display_start(0, 8);

	// Set display mode 
	//gpu_send_control_gp1(0x08000001); // NTSC
	gpu_send_control_gp1(0x08000009); // PAL

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

	// Set up joypad
	JOY_CTRL = 0x0000;
	JOY_MODE = 0x000D;
	JOY_BAUD = 0x0088;

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

	for(;;)
		yield();

	return 0;
}

