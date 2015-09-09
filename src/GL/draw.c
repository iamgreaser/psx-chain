#include "common.h"
#include "GL/intern.h"

GLboolean gl_internal_calc_triangle(int32_t *xyz0, int32_t *xyz1, int32_t *xyz2)
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
		"r"(xyz0[0]),
		"r"(xyz0[1]),
		"r"(xyz1[0]),
		"r"(xyz1[1]),
		"r"(xyz2[0]),
		"r"(xyz2[1])
		:);

	
	if(gl_list_cur == 0)
	{
		// RTPT
		asm volatile ("\tcop2 0x0280030\n");

		// Test for culling
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
				return GL_FALSE;
		}

		// Get results
		// TODO: clip, etc
		asm volatile(
			"\tmfc2 %0, $12\n"
			"\tmfc2 %1, $13\n"
			"\tmfc2 %2, $14\n"
			:
			"=r"(xyz0[0]),
			"=r"(xyz1[0]),
			"=r"(xyz2[0])
			::);

		return GL_TRUE;

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
		xyz0[0] = (mac00&0xFFFF)|(mac01<<16);
		xyz0[1] = mac02;

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
		xyz1[0] = (mac10&0xFFFF)|(mac11<<16);
		xyz1[1] = mac12;

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
		xyz2[0] = (mac20&0xFFFF)|(mac21<<16);
		xyz2[1] = mac22;

		return GL_TRUE;
	}

}

GLvoid gl_internal_push_triangle(GLuint i0, GLuint i1, GLuint i2)
{
	int32_t xyz0[2], xyz1[2], xyz2[2];
	int32_t z0, z1, z2;

	// Fill buffer
	xyz0[0] = gl_begin_vtxbuf[i0][0];
	xyz0[1] = gl_begin_vtxbuf[i0][1];
	xyz1[0] = gl_begin_vtxbuf[i1][0];
	xyz1[1] = gl_begin_vtxbuf[i1][1];
	xyz2[0] = gl_begin_vtxbuf[i2][0];
	xyz2[1] = gl_begin_vtxbuf[i2][1];

	// Calculate triangle
	if(!gl_internal_calc_triangle(xyz0, xyz1, xyz2))
		return;

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
			((0x30<<24)|gl_begin_colbuf[i0]), (xyz0[0]),
			((0x00<<24)|gl_begin_colbuf[i1]), (xyz1[0]),
			((0x00<<24)|gl_begin_colbuf[i2]), (xyz2[0]),
			(xyz0[1]<<8)+1,
			(xyz1[1]<<8)+3,
			(xyz2[1]<<8)+5,
		};

		if(otz == -2)
			gl_internal_list_add(6, 3, data);
		else
			dma_send_prim(6, data, otz);

	} else {
		uint32_t data[] = {
			((0x20<<24)|gl_begin_colbuf[i0]),
			(xyz0[0]),
			(xyz1[0]),
			(xyz2[0]),
			(xyz0[1]<<8)+1,
			(xyz1[1]<<8)+2,
			(xyz2[1]<<8)+3,
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


