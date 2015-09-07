static GLvoid gl_internal_set_enabled(GLenum cap, GLboolean val)
{
	switch(cap)
	{
		case GL_CULL_FACE:
			gl_enable_cull_face = val;
			break;

		default:
			gl_internal_set_error(GL_INVALID_ENUM);
			break;
	}
}

GLvoid glEnable(GLenum cap) // p38 2.9.3
{
	gl_internal_set_enabled(cap, GL_TRUE);
}

GLvoid glDisable(GLenum cap) // p38 2.9.3
{
	gl_internal_set_enabled(cap, GL_FALSE);
}

