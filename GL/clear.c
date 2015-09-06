GLvoid glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	gl_clear_color_ub[0] = (r + (1<<7))>>8;
	gl_clear_color_ub[1] = (g + (1<<7))>>8;
	gl_clear_color_ub[2] = (b + (1<<7))>>8;
	gl_clear_color_ub[3] = (a + (1<<7))>>8;
}

GLvoid glClear(GLbitfield mask)
{
	// TODO: read mask

	uint32_t r = gl_clear_color_ub[0];
	uint32_t g = gl_clear_color_ub[1];
	uint32_t b = gl_clear_color_ub[2];

	// TODO: DMA this
	// TODO: respect viewport set by glViewport
	gpu_send_control_gp0((0x02<<24)|(b<<16)|(g<<8)|r);
	gpu_push_vertex(0, screen_buffer);
	gpu_push_vertex(320, 240);
}


