#include "common.h"
#include "GL/intern.h"

GLvoid glNewList(GLuint n, GLenum mode) // p134 5.4
{
	// TODO
}

GLvoid glEndList(GLvoid) // p134 5.4
{
	// TODO
}

GLvoid glCallList(GLuint n) // p134 5.4
{
	// TODO
}

GLuint glGenLists(GLsizei s) // p134 5.4
{
	GLint base, fol;

	// Return 0 if s == 0
	if(s == 0) return 0;

	// We cannot allocate more than we have available
	if(s > GLINTERNAL_MAX_LIST) return 0;

	// Scan list
	for(base = 1; base <= GLINTERNAL_MAX_LIST; base++)
	{
		//for(fol = 0; fol ; base++)

	}

	return 0;
}

