#include "common.h"

volatile int screen_buffer = 0;
volatile int vblank_triggered = 0;

extern uint8_t fsys_rawcga[];

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
	//while((GP1 & 0x10000000) != 0)
		//;

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

void gpu_init(void)
{
	int i;
	int x, y;
	static volatile int k = 0;

	// Reset GPU 
	gpu_send_control_gp1(0x00000000);

	// Fix up DMA 
	gpu_send_control_gp1(0x04000002);
	gpu_send_control_gp1(0x01000000);

	// Set display area 
	//gpu_crtc_range(0x260, 0x88-(240/2), 320*8, 240); // NTSC
	gpu_crtc_range(0x260, 0xA3-(240/2), 320*8, 239); // PAL
	gpu_display_start(0, 0);

	// Set display mode 
	//gpu_send_control_gp1(0x08000001); // NTSC
	gpu_send_control_gp1(0x08000009); // PAL

	// Set draw mode 
	gpu_send_control_gp0(0xE6000000); gpu_send_control_gp0(0xE1000618); // Texpage
	gpu_draw_texmask(32, 32, 0, 0);
	gpu_draw_range(0, 0, 319, 239);

	// Copy CLUT to GPU
	gpu_send_control_gp1(0x01000000);
	gpu_send_control_gp0(0xA0000000);
	//gpu_send_data(0x01F70000);
	//gpu_send_data(0x000001C0);
	gpu_push_vertex(448, 0);
	gpu_push_vertex(2, 1);
	//gpu_send_data(0x7FFF0001);
	gpu_send_data(0x7FFF0000);

	// Copy font to GPU
	gpu_send_control_gp1(0x01000000);
	gpu_send_control_gp0(0xA0000000);
	gpu_push_vertex(512, 0);
	gpu_push_vertex(512, 8);

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
	//gpu_send_control_gp0(0x027D7D7D);
	gpu_send_control_gp0(0x02000000);
	gpu_send_data(0x00000000);
	gpu_send_data((320) | ((240)<<16));
	screen_buffer = 0;
	gpu_display_start(0, screen_buffer + 0);
}

