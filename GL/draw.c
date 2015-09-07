GLvoid gl_internal_push_triangle(GLuint i0, GLuint i1, GLuint i2)
{
	// Flush matrix
	gl_internal_flush_matrix();

	// Load triangle
	gte_load_v012_gl(
		gl_begin_vtxbuf[i0],
		gl_begin_vtxbuf[i1],
		gl_begin_vtxbuf[i2]);
	
	// RTPT
	gte_cmd_rtpt();

	if(gl_enable_cull_face)
	{
		// Apply back/frontface culling
		if(gte_get_side() < 0)
			return;
	}

	// Get results
	// TODO: cull face, clip, etc
	int16_t xy0[2], xy1[2], xy2[2];
	gte_save_s012xy_ui32_t((uint32_t *)xy0, (uint32_t *)xy1, (uint32_t *)xy2);

	// TODO: nonflats
	gpu_send_control_gp0((0x20<<24)|gl_begin_colbuf[i0]);
	gpu_send_data(*(uint32_t *)xy0);
	gpu_send_data(*(uint32_t *)xy1);
	gpu_send_data(*(uint32_t *)xy2);

	// TODO: colours and textures and whatnot
}

