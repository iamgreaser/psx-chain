#include "common.h"
#include "GL/intern.h"

GLvoid gl_internal_push_triangle(GLuint i0, GLuint i1, GLuint i2)
{
	// Flush matrix
	gl_internal_flush_matrix();

	// Load triangle
	asm volatile (
		"\tmtc2 %0, $0\n"
		"\tmtc2 %1, $1\n"
		"\tmtc2 %2, $2\n"
		"\tmtc2 %3, $3\n"
		"\tmtc2 %4, $4\n"
		"\tmtc2 %5, $5\n"
		: :
		"r"(gl_begin_vtxbuf[i0][0]),
		"r"(gl_begin_vtxbuf[i0][1]),
		"r"(gl_begin_vtxbuf[i1][0]),
		"r"(gl_begin_vtxbuf[i1][1]),
		"r"(gl_begin_vtxbuf[i2][0]),
		"r"(gl_begin_vtxbuf[i2][1])
		:);
	
	int32_t xy0, xy1, xy2;
	int32_t z0 = 0;
	int32_t z1 = 0;
	int32_t z2 = 0;

	if(gl_list_cur == 0)
	{
		// RTPT
		asm volatile ("\tcop2 0x0280030\n");

	} else {
		// ACTUALLY LOADS IR123
		uint32_t mac00, mac01, mac02;
		uint32_t mac10, mac11, mac12;
		uint32_t mac20, mac21, mac22;

		// Apply matrix transform to all 3 points
		asm volatile ("\tcop2 0x0480012\n"); // MVMVA V0
		asm volatile (
			"\tnop\n"
			"\tnop\n"
			"\tnop\n"
			"\tnop\n"
			"\tnop\n"
			"\tnop\n"
			"\tnop\n"
			"\tmfc2 %0, $9\n"
			"\tmfc2 %1, $10\n"
			"\tmfc2 %2, $11\n"
			:
			"=r"(mac00),
			"=r"(mac01),
			"=r"(mac02)
			::);
		xy0 = (mac00&0xFFFF)|(mac01<<16);
		z0 = mac02;

		asm volatile ("\tcop2 0x0488012\n"); // MVMVA V1
		asm volatile (
			"\tnop\n"
			"\tnop\n"
			"\tnop\n"
			"\tnop\n"
			"\tnop\n"
			"\tnop\n"
			"\tnop\n"
			"\tmfc2 %0, $9\n"
			"\tmfc2 %1, $10\n"
			"\tmfc2 %2, $11\n"
			:
			"=r"(mac10),
			"=r"(mac11),
			"=r"(mac12)
			::);
		xy1 = (mac10&0xFFFF)|(mac11<<16);
		z1 = mac12;

		asm volatile ("\tcop2 0x0490012\n"); // MVMVA V2
		asm volatile (
			"\tnop\n"
			"\tnop\n"
			"\tnop\n"
			"\tnop\n"
			"\tnop\n"
			"\tnop\n"
			"\tnop\n"
			"\tmfc2 %0, $9\n"
			"\tmfc2 %1, $10\n"
			"\tmfc2 %2, $11\n"
			:
			"=r"(mac20),
			"=r"(mac21),
			"=r"(mac22)
			::);
		xy2 = (mac20&0xFFFF)|(mac21<<16);
		z2 = mac22;

		// TODO: fix things
		/*
		xy0 = gl_begin_vtxbuf[i0][0];
		z0 = gl_begin_vtxbuf[i0][1];
		xy1 = gl_begin_vtxbuf[i1][0];
		z1 = gl_begin_vtxbuf[i1][1];
		xy2 = gl_begin_vtxbuf[i2][0];
		z2 = gl_begin_vtxbuf[i2][1];
		*/
	}

	if(gl_enable_cull_face && gl_list_cur != 0)
	{
		int32_t mac0;

		// Get side
		asm volatile (
			"\tcop2 0x1400006\n" // NCLIP
			"\tmfc2 %0, $24\n"
			:
			"=r"(mac0)
			::);

		// Apply back/frontface culling
		if(mac0 < 0)
			return;
	}

	// Get results
	// TODO: clip, etc
	if(gl_list_cur == 0)
	{
		asm volatile(
			"\tmfc2 %0, $12\n"
			"\tmfc2 %1, $13\n"
			"\tmfc2 %2, $14\n"
			:
			"=r"(xy0),
			"=r"(xy1),
			"=r"(xy2)
			::);
	}

	// Get Z order
	int32_t otz = -1;
	if(gl_list_cur != 0)
	{
		// Z order irrelevant at this point
		// use a special marker for this
		otz = -2;

	} else if(gl_enable_depth_test) {
		// TODO: z-clip
		// Get average Z
		asm volatile (
			"\tcop2 0x158002D\n" // AVSZ3
			"\tmfc2 %0, $7\n"
			:
			"=r"(otz)
			::);
	}

	// Split into subtypes
	if(gl_begin_gourcount != 0)
	{
		uint32_t data[] = {
			((0x30<<24)|gl_begin_colbuf[i0]), (xy0),
			((0x00<<24)|gl_begin_colbuf[i1]), (xy1),
			((0x00<<24)|gl_begin_colbuf[i2]), (xy2),
			(z0<<8)+1,
			(z1<<8)+3,
			(z2<<8)+5,
		};

		if(otz == -2)
			gl_internal_list_add(6, 3, data);
		else
			dma_send_prim(6, data, otz);

	} else {
		uint32_t data[] = {
			((0x20<<24)|gl_begin_colbuf[i0]),
			(xy0),
			(xy1),
			(xy2),
			(z0<<8)+1,
			(z1<<8)+2,
			(z2<<8)+3,
		};

		if(otz == -2)
			gl_internal_list_add(4, 3, data);
		else
			dma_send_prim(4, data, otz);
	}

	// TODO: colours and textures and whatnot
}

GLvoid glFlush(GLvoid) // p138, 5.5
{
	dma_flush();
}

GLvoid glFinish(GLvoid) // p138, 5.5
{
	glFlush();
	dma_wait();
}


