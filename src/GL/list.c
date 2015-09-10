#include "common.h"
#include "GL/intern.h"

// XXX: ASSUMES gl_list_cur IS VALID AND NONZERO
GLvoid gl_internal_list_add(GLsizei count, GLsizei zcount, uint32_t *data)
{
	// Get list
	GLlist_s **dl_ptr = &gl_list_alloc[gl_list_cur-1];
	GLlist_s *dl = *dl_ptr;

	// Ensure enough space
	GLsizei sz = count+zcount+1;
	if(dl->space - dl->size < sz)
	{
		// Calculate more space
		GLsizei newsz = dl->space+50;
		if(newsz - dl->size < sz)
			newsz = sz + dl->size + 5;

		// Reallocate
		dl = realloc(dl, sizeof(GLlist_s)
			+ sizeof(GLuint)*newsz);
		if(dl == NULL)
		{
			// OUT OF MEMORY
			// (keep intact!)
			gl_internal_set_error(GL_OUT_OF_MEMORY);
			return;
		}

		// Update space + pointer
		dl->space = newsz;
		*dl_ptr = dl;
	}

	// Set up header
	dl->data[dl->size] = zcount | (count<<8);

	// Copy data
	memcpy(&dl->data[dl->size+1], data, (sz-1)*sizeof(uint32_t));

	// Advance
	dl->size += sz;
}

GLvoid glNewList(GLuint n, GLenum mode) // p134 5.4
{
	if(gl_list_cur != 0)
	{
		// cannot nest lists, p134 5.4
		gl_internal_set_error(GL_INVALID_OPERATION);
		return;
	}

	if(n == 0)
	{
		// n cannot be 0, p134 5.4
		gl_internal_set_error(GL_INVALID_VALUE);
		return;
	}

	if(mode != GL_COMPILE && mode != GL_COMPILE_AND_EXECUTE)
	{
		// not a valid mode
		gl_internal_set_error(GL_INVALID_ENUM);
		return;
	}

	if(gl_begin_mode != 0)
	{
		// cannot start list during Begin/End, assumed
		gl_internal_set_error(GL_INVALID_OPERATION);
		return;
	}

	if(n > GLINTERNAL_MAX_LIST)
	{
		// don't overflow n!
		gl_internal_set_error(GL_OUT_OF_MEMORY);
		return;
	}

	// Allocate display list
	GLlist_s *dl = gl_list_alloc[n-1];
	const GLsizei space_init = 30;
	gl_list_alloc[n-1] = realloc(dl,
		sizeof(GLlist_s) + sizeof(uint32_t)*space_init);

	if(gl_list_alloc[n-1] == NULL)
	{
		// OUT OF MEMORY
		gl_internal_set_error(GL_OUT_OF_MEMORY);
		free(dl);
		return;
	}

	// Set up display list
	dl = gl_list_alloc[n-1];
	dl->size = 0;
	dl->space = space_init;

	// Stash new matrix
	// TODO: protect against underflow and imbalance
	// (and use the correct damn modes!)
	glPushMatrix();
	glLoadIdentity();

	// Set state
	gl_list_cur = n;
	gl_list_mode = mode;
}

GLvoid glEndList(GLvoid) // p134 5.4
{
	if(gl_list_cur == 0)
	{
		// cannot end w/o beginning, p134 5.4
		gl_internal_set_error(GL_INVALID_OPERATION);
		return;
	}

	// Reduce list if possible
	GLlist_s *newdl = realloc(gl_list_alloc[gl_list_cur-1],
		sizeof(GLlist_s) + 
		gl_list_alloc[gl_list_cur-1]->size*sizeof(uint32_t));
	if(newdl != NULL)
	{
		gl_list_alloc[gl_list_cur-1] = newdl;
		newdl->space = newdl->size;
	}

	// Backup index
	GLuint n = gl_list_cur;
	
	// Restore matrix
	glPopMatrix();

	// Clear list gen state
	gl_list_cur = 0;
	gl_list_mode = 0;

	// Call list if we used GL_COMPILE_AND_EXECUTE
	if(gl_list_mode == GL_COMPILE_AND_EXECUTE)
		glCallList(gl_list_cur);
}

GLvoid glCallList(GLuint n) // p134 5.4
{
	GLsizei i;

	// spec does say that you can glCallList recursively
	//
	// however, it places the glCallList command into the list
	// rather than calling the list and adding its commands
	//
	// considering adding a glCallList list command w/ a supplied matrix.
	// resulting list size would be smaller,
	// potentially a bit slower than precalcing though.
	//
	// glCallList command encoding proposal:
	// 6 words (12 hwords) for matrix
	// 1 word for header w/ count=0, zcount=7, upper 16 bits = list ID
	// 7 words total

	/*
	if(gl_list_cur != 0)
	{
		gl_internal_set_error(GL_INVALID_OPERATION);
		return;
	}
	*/

	if(n == 0)
	{
		// n cannot be 0, p136 5.4
		gl_internal_set_error(GL_INVALID_VALUE);
		return;
	}

	if(n > GLINTERNAL_MAX_LIST
		|| gl_list_alloc[n-1] == NULL
		|| gl_list_alloc[n-1]->size == 0
		|| gl_list_alloc[n-1]->space == 0)
	{
		// list has to be allocated, assumed
		// (also assuming it actually has stuff in it!)
		gl_internal_set_error(GL_INVALID_VALUE);
		return;
	}

	// Flush matrix
	gl_internal_flush_matrix();

	// Get display list
	GLlist_s *dl = gl_list_alloc[n-1];

	// Walk display list
	for(i = 0; i < dl->size; )
	{
		// Read header
		uint32_t hdr = dl->data[i];
		uint32_t count = hdr >> 8;
		uint32_t zcount = hdr & 0xFF;
		uint32_t *plist = &dl->data[i+1];
		uint32_t *zlist = &dl->data[i+1+count];

		// Advance
		i += count+zcount+1;

#if 0
		// Draw
		// FIXME: SLOW
		gl_internal_push_triangle_fromlist(count, plist, zlist);
#else
		// Skip if we're going to overflow
		uint32_t pdata[14];
		if(count >= 14)
			return;

		// Get values
		int i0 = zlist[0]&0xFF;
		int i1 = zlist[1]&0xFF;
		int i2 = zlist[2]&0xFF;
		const uint32_t *xy0_srcptr = &plist[i0];
		const uint32_t *xy1_srcptr = &plist[i1];
		const uint32_t *xy2_srcptr = &plist[i2];
		uint32_t xy0 = *xy0_srcptr;
		uint32_t xy1 = *xy1_srcptr;
		uint32_t xy2 = *xy2_srcptr;
		uint32_t *xy0_ptr = &pdata[i0];
		uint32_t *xy1_ptr = &pdata[i1];
		uint32_t *xy2_ptr = &pdata[i2];
		uint32_t z0 = (uint32_t)(((int32_t)(zlist[0]))>>8);
		uint32_t z1 = (uint32_t)(((int32_t)(zlist[1]))>>8);
		uint32_t z2 = (uint32_t)(((int32_t)(zlist[2]))>>8);

		// XXX: assumes triangle prims (zcount == 3)
		// Load triangle
		asm volatile (
			"\tmtc2 %0, $0\n"
			"\tmtc2 %1, $1\n"
			"\tmtc2 %2, $2\n"
			"\tmtc2 %3, $3\n"
			"\tmtc2 %4, $4\n"
			"\tmtc2 %5, $5\n"
			: :
			"r"(xy0),
			"r"(z0),
			"r"(xy1),
			"r"(z1),
			"r"(xy2),
			"r"(z2)
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
				continue;
		}

		// Copy
		memcpy(pdata, plist, sizeof(uint32_t)*count);

		// Get results
		asm volatile(
			"\tmfc2 %0, $12\n"
			"\tmfc2 %1, $13\n"
			"\tmfc2 %2, $14\n"
			:
			"=r"(*xy0_ptr),
			"=r"(*xy1_ptr),
			"=r"(*xy2_ptr)
			::);

		// Get Z order
		int32_t otz = -1;
		if(gl_list_cur == 0 && gl_enable_depth_test)
		{
			// TODO: z-clip
			// Get average Z
			asm volatile (
				"\tcop2 0x158002D\n" // AVSZ3
				"\tmfc2 %0, $7\n"
				:
				"=r"(otz)
				::);
		}

		// Post
		if(gl_list_cur != 0)
		{
			memcpy(pdata+count, zlist, sizeof(uint32_t)*3);
			gl_internal_list_add(count, 3, pdata);
		} else {
			dma_send_prim(count, pdata, otz);
		}
#endif
	}
}

GLuint glGenLists(GLsizei s) // p137 5.4
{
	GLuint base, fol;

	// Return 0 if s == 0, p134
	if(s == 0) return 0;

	// We cannot allocate more than we have available
	if(s > GLINTERNAL_MAX_LIST)
	{
		// XXX: not sure if GL_OUT_OF_MEMORY is appropriate
		// command is actually ignored
		// "undefined" just means it doesn't have to ignore it
		gl_internal_set_error(GL_OUT_OF_MEMORY);
		return 0;
	}

	// Scan list
	for(base = 1; base <= GLINTERNAL_MAX_LIST; base++)
	{
		GLboolean list_pass = GL_TRUE;

		for(fol = 0; fol < s; fol++)
		{
			if(gl_list_alloc[base+fol] != NULL)
			{
				list_pass = GL_FALSE;
				base = base+fol;
				break;
			}
		}

		if(list_pass)
		{
			for(fol = 0; fol < s; fol++)
			{
				gl_list_alloc[base+fol] =
					malloc(sizeof(GLlist_s));

				if(gl_list_alloc[base+fol] == NULL)
				{
					// OUT OF MEMORY
					gl_internal_set_error(GL_OUT_OF_MEMORY);
					return 0;
				}

				gl_list_alloc[base+fol]->size = 0;
				gl_list_alloc[base+fol]->space = 0;
			}

			return base;
		}

	}

	gl_internal_set_error(GL_OUT_OF_MEMORY);
	return 0;
}

