#include "common.h"
#include "GL/intern.h"

// width is measured in nybbles
GLvoid glTexStealRangePSX(GLuint x, GLuint y, GLuint w, GLuint h)
{
	GLuint px, py;

	// TODO: check if already taken
	// TODO: check if width/height valid
	w >>= 3;
	h >>= 3;
	x >>= 3;
	y >>= 3;

	for(py = 0; py < h; py++)
	for(px = 0; px < w; px++)
		gl_tex_map[py+y][px+x] = -1;
}

// using GL 1.1 man page specification
GLvoid glTexImage2D(GLenum target, GLint level, GLint internalFormat,
	GLsizei width, GLsizei height, GLint border,
	GLenum format, GLenum type, const GLvoid *pixels)
{
	GLsizei bx, by;
	GLsizei x, y;
	GLsizei i;

	// We do not support borders
	if(border != 0)
	{
		gl_internal_set_error(GL_INVALID_VALUE);
		return;
	}

	// We do not support mipmapping (yet)
	if(level != 0)
	{
		gl_internal_set_error(GL_INVALID_VALUE);
		return;
	}

	// Width and height must be a power of two
	if(width == 0 || (width & (width - 1)) != 0)
	{
		gl_internal_set_error(GL_INVALID_VALUE);
		return;
	}
	if(height == 0 || (height & (height - 1)) != 0)
	{
		gl_internal_set_error(GL_INVALID_VALUE);
		return;
	}

	// Minimum texture size: 8x8
	if(width < 8 || height < 8)
	{
		gl_internal_set_error(GL_INVALID_VALUE);
		return;
	}

	// Check pixel format
	// (we are very strict here)
	if(GL_TRUE
		&& internalFormat != GL_RGB
		&& internalFormat != GL_RGBA
		&& internalFormat != GL_RGB5
		&& internalFormat != GL_RGB5_A1
		&& internalFormat != GL_COLOR_INDEX)
	{
		gl_internal_set_error(GL_INVALID_ENUM);
		return;
	}

	// Now check if we have any valid formats
	int gpubits;
	int gpuwbits;
	if(internalFormat == GL_COLOR_INDEX4_EXT
		&& format == GL_COLOR_INDEX
		&& type == GL_UNSIGNED_NYBBLE_PSX)
	{
		gpubits = 4;
		gpuwbits = 3;
	}
	else if(internalFormat == GL_COLOR_INDEX8_EXT
		&& format == GL_COLOR_INDEX
		&& type == GL_UNSIGNED_BYTE)
	{
		gpubits = 8;
		gpuwbits = 2;
	}
	else if(internalFormat != GL_COLOR_INDEX
		&& format == GL_RGBA
		&& type == GL_UNSIGNED_SHORT)
	{
		// XXX: type is probably not valid
		gpubits = 16;
		gpuwbits = 1;
	}
	else
	{
		gl_internal_set_error(GL_INVALID_ENUM);
		return;
	}

	// Make sure our target is GL_TEXTURE_2D
	if(target != GL_TEXTURE_2D)
	{
		gl_internal_set_error(GL_INVALID_ENUM);
		return;
	}

	// Work out the size of our region in 32-bit words
	int bitcw = width>>gpuwbits;
	int bitch = height>>3;

	// Check if we need to reallocate
	// (w,h are in nybble tiles)
	GLtex_s *tex = &gl_tex_handle[gl_tex_bind2d];
	if(bitcw != tex->w && bitch != tex->h)
	{
		// Deallocate
		// XXX: do we reallocate if we run out of memory?
		for(y = 0; y < tex->w; y++)
		for(x = 0; x < tex->h; x++)
		{
			gl_tex_map[tex->y+y][tex->x+x] = 0;
		}

		tex->w = 0;
		tex->h = 0;
		tex->x = 0;
		tex->y = 0;
		tex->bits = 0;

		// Find a range
		// TODO: faster algorithm
		GLboolean pass = GL_FALSE;
		for(by = 0; by < (GLuint)( 64-bitch) && !pass; by++)
		for(bx = 0; bx < (GLuint)(512-bitcw) && !pass; bx++)
		{
			pass = GL_TRUE;

			for(y = 0; y < (GLuint)bitch && pass; y++)
			for(x = 0; x < (GLuint)bitcw; x++)
			{
				if(gl_tex_map[by+y][bx+x] != 0)
				{
					pass = GL_FALSE;
					break;
				}
			}

			if(pass)
			{
				// Fill the range
				for(y = 0; y < (GLuint)bitch && pass; y++)
				for(x = 0; x < (GLuint)bitcw; x++)
				{
					gl_tex_map[by+y][bx+x] = 1;
				}

				// Set the texture parameters
				tex->w = bitcw;
				tex->h = bitch;
				tex->x = bx;
				tex->y = by;
				tex->clut = 0; // TODO
				tex->bits = gpubits;
			}
		}

		if(!pass)
		{
			// Failed to allocate
			// OUT OF MEMORY
			gl_internal_set_error(GL_OUT_OF_MEMORY);
			return;
		}
	}

	// Upload
	dma_wait();
	gpu_send_control_gp0(0xA0000000);
	gpu_push_vertex(tex->x*2, tex->y*8);
	gpu_push_vertex( bitcw*2,  bitch*8);

	for(y = 0; y < (GLuint)bitch*8; y++)
	for(x = 0; x < (GLuint)bitcw*2; x++)
	{
		gpu_send_data(*(uint32_t *)pixels);
		pixels += 4;
	}
}

// using GL 1.1 man page specification - THIS IS A 1.1 FEATURE
GLvoid glBindTexture(GLenum target, GLuint texture)
{
	// "GL_INVALID_OPERATION is generated if glBindTexture is executed  between
	//  the execution of glBegin and the corresponding execution of glEnd."
	//
	// We are ignoring that sentence because this feature is useful
	// and has a low overhead.

	// no 1D (or 3D for that matter) texture support
	if(target != GL_TEXTURE_2D)
	{
		gl_internal_set_error(GL_INVALID_ENUM);
		return;
	}

	if(texture >= GLINTERNAL_MAX_TEX)
	{
		gl_internal_set_error(GL_INVALID_VALUE);
		return;
	}

	gl_tex_bind2d = texture;
}

// using GL 1.1 man page specification - THIS IS A 1.1 FEATURE
GLvoid glGenTextures(GLsizei n, GLuint *textures)
{
	GLsizei i;

	// Make sure we aren't in a glBegin/glEnd block
	if(gl_begin_mode != 0)
	{
		gl_internal_set_error(GL_INVALID_OPERATION);
		return;
	}

	// n cannot be < 0
	// But we've defined sizei as unsigned
	// So by definition n cannot be < 0

	for(i = 0; i < GLINTERNAL_MAX_TEX; i++)
	{
		if(gl_tex_handle[i].bits == 0)
		{
			// special marker to indicate we reserved this
			// but not a valid value
			gl_tex_handle[i].bits = 127;

			// Report allocation
			*textures = i;
			n -= 1;
			textures += 1;

			// If we've allocated everything, all good!
			if(n == 0)
				return;
		}

	}

	// OUT OF MEMORY
	gl_internal_set_error(GL_OUT_OF_MEMORY);
	return;
}

