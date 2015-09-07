#include "common.h"

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
extern uint8_t int_handler_stub[];
extern uint8_t int_handler_stub_end[];

volatile int got_vblank = 0;
int waiting_for_vblank = 0;
volatile int playing_music = 0;
void isr_handler_c(uint32_t cop0_sr, uint32_t cop0_cause, uint32_t cop0_epc)
{
	(void)cop0_sr;
	(void)cop0_cause;
	(void)cop0_epc;

	//*(uint32_t *)0x801FFFFC = cop0_cause;
	//*(uint32_t *)0x801FFFF8 = cop0_sr;

	if((I_STAT & 0x0080) != 0)
	{
		while((JOY_STAT & 0x0080) != 0) {}
		joy_update();

		I_STAT &= ~0x0080;
		I_MASK |= 0x0080;
		JOY_CTRL |= 0x0010;
		// TODO handle other joypad interrupts
	}

	if((I_STAT & 0x0001) != 0)
	{
		TMR_n_MODE(1) = 0;
		TMR_n_COUNT(1) = 0;
		TMR_n_MODE(1) = 0x0300;
		got_vblank++;
		if(playing_music) f3m_player_play(&s3mplayer, NULL, NULL);
		I_STAT &= ~0x0001;
		I_MASK |= 0x0001;
	}
}

void aaa_start(void)
{
	volatile int fencer = 0;
	(void)fencer;
	int i;

	I_MASK = 0;

	memset((void *)(_BSS_START), 0, _BSS_END - _BSS_START);
	memcpy((void *)0x80000080, int_handler_stub,
		int_handler_stub_end - int_handler_stub);

	fencer = 1;

	I_STAT = 0xFFFF;
	I_MASK = 0x0001;
	asm volatile (
		"\tmfc0 $t0, $12\n"
		"\tori $t0, 0x7F01\n"
		"\tmtc0 $t0, $12\n"
		/*
		"\tmfc0 $t0, $13\n"
		"\tori $t0, 0x0300\n"
		"\tmtc0 $t0, $13\n"
		*/
		/*
		"\tsyscall\n"
		"\tnop\n"
		"\tsyscall\n"
		"\tnop\n"
		*/
		:::"t0"
	);

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

static void update_frame(void)
{
	volatile int lag;
	int i;

	int tmr_frame = TMR_n_COUNT(1);

	// Enable things
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	// Clear screen
	glClearColorx(0x0000, 0x1D00, 0x1D00, 0x0000);
	glClear(1);

	// Set up camera matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatex(0, 0, 0x100);
	glRotatex(tri_ang*360, 0, 0, 0x1000);
	glRotatex(tri_ang*60, 0, 0x1000, 0);
	//glRotatex(0, 0, 0x10000, 0);

	// Draw spinny triangle
	//gpu_send_control_gp1(0x01000000);
	int q = 60;
	for(i = 0; i < q; i++)
	{
		glPushMatrix();
		glRotatex(((360*i)<<16)/q, 0, 0, 0x1000);
		glTranslatex(0x80, 0, 0);
		glRotatex(-tri_ang*360*3, 0, 0, 0x10000);
		glRotatex(tri_ang*60, 0, 0x1000, 0);
		glRotatex(tri_ang*200, 0, 0, 0x1000);
		glBegin(GL_TRIANGLES);
			glColor3ub(0x7F, 0x00, 0x00);
			glVertex3x(-50, -50,  0);
			glColor3ub(0x7F, 0x7F, 0x00);
			glVertex3x( 50, -50,  0);
			glColor3ub(0x7F, 0x00, 0x7F);
			glVertex3x(  0,  50,  0);

			glColor3ub(0x00, 0x7F, 0x00);
			glVertex3x(  0,   0, 70);
			glColor3ub(0x7F, 0x7F, 0x00);
			glVertex3x( 50, -50,  0);
			glColor3ub(0x00, 0x7F, 0x7F);
			glVertex3x(-50, -50,  0);

			glColor3ub(0x00, 0x00, 0x7F);
			glVertex3x(  0,   0, 70);
			glVertex3x(-50, -50,  0);
			glColor3ub(0x00, 0x7F, 0x7F);
			glVertex3x(  0,  50,  0);

			glColor3ub(0x7F, 0x7F, 0x7F);
			glVertex3x(  0,   0, 70);
			glVertex3x(  0,  50,  0);
			glColor3ub(0x00, 0x00, 0x00);
			glVertex3x( 50, -50,  0);
		glEnd();
		glPopMatrix();
	}

	tri_ang += FM_PI*2/180/2;

	// Draw string
	//gpu_send_control_gp1(0x01000000);
	uint8_t update_str_buf[64];
	sprintf(update_str_buf, "ord=%02i row=%02i speed=%03i/%i"
		, (int)s3mplayer.cord
		, (int)s3mplayer.crow
		, (int)s3mplayer.tempo
		, (int)s3mplayer.speed
		);
	screen_print(16, 16+8*0, 0x007F7F7F, update_str_buf);
	screen_print(16, 16+8*1, 0x007F7F7F, s3mplayer.mod->name);

	// Read joypad
	pad_id   =  pad_id_now  ;
	pad_data = ~pad_data_now;
	joy_poll();

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

	sprintf(update_str_buf, "joypad=%04X (%04X: %s)", (unsigned)pad_data, (unsigned)pad_id, pad_id_str);
	screen_print(16, 16+8*3, 0x007F7F7F, update_str_buf);
	sprintf(update_str_buf, "glGetError() = %X", (unsigned)glGetError());
	screen_print(16, 16+8*5, 0x007F7F7F, update_str_buf);
	sprintf(update_str_buf, "dma beg = %i", (int)TMR_n_COUNT(1));
	screen_print(16, 16+8*8, 0x007F7F7F, update_str_buf);

	// Flush DMA
	glFinish();

	// Get actual render time
	sprintf(update_str_buf, "dma end = %i", (int)TMR_n_COUNT(1));
	screen_print(16, 16+8*9, 0x007F7F7F, update_str_buf);
	sprintf(update_str_buf, "fra beg = %i", (int)tmr_frame);
	screen_print(16, 16+8*7, 0x007F7F7F, update_str_buf);
	glFinish();

	//while((GP1 & 0x10000000) == 0) {}

	// Flip pages
	gpu_display_start(0, screen_buffer);
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
	static const uint8_t test_str[] = "Converting samples...";
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
	gpu_display_start(0, screen_buffer);
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
	gpu_init();
	dma_init();

	// Set up joypad
	joy_init();

	// Prep module
	f3m_player_init(&s3mplayer, fsys_s3m_test);

	SPU_CNT = 0xC000;
	SPU_MVOL_L = 0x2000;
	SPU_MVOL_R = 0x2000;

	waiting_for_vblank = got_vblank;
	playing_music = 1;
	for(;;)
	{
		while(waiting_for_vblank >= got_vblank) {}
		waiting_for_vblank++;
		//f3m_player_play(&s3mplayer, NULL, NULL);
		update_frame();
	}

	for(;;)
		yield();

	return 0;
}

