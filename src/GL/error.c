#include "common.h"
#include "GL/intern.h"

GLvoid gl_internal_set_error(GLenum error)
{
	if(error != GL_NO_ERROR) // safety
		gl_error = error;
}

GLenum glGetError(GLvoid) // p20 2.5
{
	GLenum ret = gl_error;
	gl_error = 0;
	return ret;
}

