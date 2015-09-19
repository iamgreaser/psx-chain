#include "common.h"
#include "GL/intern.h"

// partial implementation of:
// - GL_EXT_paletted_texture 
// - GL_EXT_shared_texture_palette
//
// supplying mostly custom extensions though

/*
GLvoid glColorTableEXT(GLenum target, GLenum internalFormat,
	GLsizei width, GLenum format, GLenum type, const GLvoid *data)
*/
GLvoid glBindClutPSX(GLuint clut_texture, GLuint xoffs, GLuint yoffs)
{
	// Check if bound
	if(clut_texture >= GLINTERNAL_MAX_TEX)
	{
		gl_internal_set_error(GL_INVALID_VALUE);
		return;
	}

	// Get texture
	GLtex_s *tex = &gl_tex_handle[clut_texture];

	// Check if valid
	if(tex->bits == 127 || tex->w == 0)
	{
		gl_internal_set_error(GL_INVALID_VALUE);
		return;
	}

	// Get base texture
	GLtex_s *btex = &gl_tex_handle[gl_tex_bind2d];

	// Check if valid
	if(btex->bits == 127 || btex->w == 0)
	{
		gl_internal_set_error(GL_INVALID_VALUE);
		return;
	}

	// Bind!
	btex->clut = (((tex->x*2 + xoffs)>>4)&0x3F) | (((tex->y*8+yoffs)<<6) & 0x7FC0);
	gl_begin_texclut = btex->clut<<16;
}
