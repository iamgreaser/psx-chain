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
	
	// RTPT
	asm volatile ("\tcop2 0x0280030\n");

	if(gl_enable_cull_face)
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
	// TODO: clip, z-order, etc
	int16_t xy0[2], xy1[2], xy2[2];
	asm volatile(
		"\tmfc2 %0, $12\n"
		"\tmfc2 %1, $13\n"
		"\tmfc2 %2, $14\n"
		:
		"=r"(*(uint32_t *)xy0),
		"=r"(*(uint32_t *)xy1),
		"=r"(*(uint32_t *)xy2)
		::);

	// TODO: z-order
	int32_t otz = -1;

	// Split into subtypes
	if(gl_begin_gourcount != 0)
	{
		uint32_t data[] = {
			((0x30<<24)|gl_begin_colbuf[i0]), (*(uint32_t *)xy0),
			((0x00<<24)|gl_begin_colbuf[i1]), (*(uint32_t *)xy1),
			((0x00<<24)|gl_begin_colbuf[i2]), (*(uint32_t *)xy2),
		};

		dma_send_prim(6, data, otz);

	} else {
		uint32_t data[] = {
			((0x20<<24)|gl_begin_colbuf[i0]),
			(*(uint32_t *)xy0),
			(*(uint32_t *)xy1),
			(*(uint32_t *)xy2),
		};

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


