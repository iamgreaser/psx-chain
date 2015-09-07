#include "common.h"
#include "GL/intern.h"

GLvoid gl_internal_flush_matrix(GLvoid)
{
	// Check if GTE matrix is clean
	if(!gl_mat_gte_isdirty)
		return;

	// Combine matrices
	// TODO!

	// Upload matrix to GTE
	gte_loadmat_gl(
		gl_mat_rot[0][gl_mat_stack[0]],
		gl_mat_trn[0][gl_mat_stack[0]]);

	// Mark as clean
	gl_mat_gte_isdirty = GL_FALSE;
}

GLvoid glLoadIdentity(GLvoid) // p35 2.9.2
{
	GLsizei i, j;

	gl_mat_gte_isdirty = GL_TRUE;

	GLint stackidx = gl_mat_stack[gl_mat_cur];
	GLfixed *rot = gl_mat_rot[gl_mat_cur][stackidx];
	GLfixed *trn = gl_mat_trn[gl_mat_cur][stackidx];

	for(i = 0; i < 3; i++)
	for(j = 0; j < 3; j++)
		rot[i*3+j] = (i==j ? 0x10000 : 0x00000);
	
	for(i = 0; i < 3; i++)
		trn[i] = 0x00000;
}

GLvoid glMatrixMode(GLenum mode) // p34 2.9.2
{
	if(mode >= GL_MODELVIEW && mode <= GL_PROJECTION)
		gl_mat_cur = mode - GL_MODELVIEW;
	else
		gl_internal_set_error(GL_INVALID_ENUM);
}

GLvoid glRotatex(GLfixed theta, GLfixed x, GLfixed y, GLfixed z) // p35 2.9.2
{
	int i, j, k;

	// Ensure we have an axis of rotation!
	if(x == 0 && y == 0 && z == 0)
		return;

	// Mark dirty
	gl_mat_gte_isdirty = GL_TRUE;

	// OPTIMISATION: If on a single axis we don't need to tweak as much
	int zeros = 0;
	if(x == 0) zeros++;
	if(y == 0) zeros++;
	if(z == 0) zeros++;

	//if(0)
	if(zeros == 2)
	{
		// Rotate around an axis
		// FIXME: get correct rotation orders
		int a, b;
		if(x != 0) { a = 1; b = 2; }
		else if(y != 0) { a = 2; b = 0; }
		else { a = 0; b = 1; }

		// Get matrix pointer
		GLint stackidx = gl_mat_stack[gl_mat_cur];
		GLfixed *rot = gl_mat_rot[gl_mat_cur][stackidx];

		// Get sin/cos
		theta /= 360;
		GLfixed tsin = fixsin(theta);
		GLfixed tcos = fixcos(theta);

		// Get old values
		GLfixed ta0 = rot[a*3 + 0];
		GLfixed ta1 = rot[a*3 + 1];
		GLfixed ta2 = rot[a*3 + 2];
		GLfixed tb0 = rot[b*3 + 0];
		GLfixed tb1 = rot[b*3 + 1];
		GLfixed tb2 = rot[b*3 + 2];

		// Write new values
		// TODO: get this to behave
		rot[a*3 + 0] = fixmulf(ta0, tcos) - fixmulf(tb0, tsin);
		rot[a*3 + 1] = fixmulf(ta1, tcos) - fixmulf(tb1, tsin);
		rot[a*3 + 2] = fixmulf(ta2, tcos) - fixmulf(tb2, tsin);
		rot[b*3 + 0] = fixmulf(ta0, tsin) + fixmulf(tb0, tcos);
		rot[b*3 + 1] = fixmulf(ta1, tsin) + fixmulf(tb1, tcos);
		rot[b*3 + 2] = fixmulf(ta2, tsin) + fixmulf(tb2, tcos);

		return;
	}

	//
	// GENERIC OPENGL ROTATION
	//

	// Normalise x,y,z
	GLfixed vlen2 = fixmul(x,x) + fixmul(y,y) + fixmul(z,z);
	if(vlen2 >= 0x100)
	{
		GLfixed vlen = fixsqrt(vlen2);
		x = fixdiv(x, vlen);
		y = fixdiv(y, vlen);
		z = fixdiv(z, vlen);
	} else {
		// Length is too small to get a sane result
		// Use int multiplies instead and shift later

		// FIXME get this working correctly
		vlen2 = x*x + y*y + z*z;

		// Skip if we get 0
		if(vlen2 == 0)
			return;

		GLfixed vlen = fixsqrt(vlen2)<<8;
		x = fixdiv(x, vlen);
		y = fixdiv(y, vlen);
		z = fixdiv(z, vlen);

	}

	// Get sin/cos
	theta /= 360;
	GLfixed tsin = fixsin(theta);
	GLfixed tcos = fixcos(theta);

	// Generate rotation matrix
	GLfixed itcos = 0x10000-tcos;
	GLfixed base_rot[3][3] = {
		{
			fixmulf(itcos, fixmulf(x, x))+tcos,
			fixmulf(itcos, fixmulf(x, y))-fixmulf(tsin, z),
			fixmulf(itcos, fixmulf(x, z))+fixmulf(tsin, y),
		},
		{
			fixmulf(itcos, fixmulf(y, x))+fixmulf(tsin, z),
			fixmulf(itcos, fixmulf(y, y))+tcos,
			fixmulf(itcos, fixmulf(y, z))-fixmulf(tsin, x),
		},
		{
			fixmulf(itcos, fixmulf(z, x))-fixmulf(tsin, y),
			fixmulf(itcos, fixmulf(z, y))+fixmulf(tsin, x),
			fixmulf(itcos, fixmulf(z, z))+tcos,
		},
	};

	// Apply rotation matrix
	GLint stackidx = gl_mat_stack[gl_mat_cur];
	GLfixed *rot = gl_mat_rot[gl_mat_cur][stackidx];
	//GLfixed *trn = gl_mat_trn[gl_mat_cur][stackidx];
	GLfixed oldrot[9];
	//GLfixed oldtrn[3];
	memcpy(oldrot, rot, sizeof(GLfixed)*9);
	//memcpy(oldtrn, trn, sizeof(GLfixed)*3);

	// FIXME: translate properly
	// FIXME: ensure correct order
	for(i = 0; i < 3; i++)
	for(j = 0; j < 3; j++)
	{
		GLfixed sum = 0;
		for(k = 0; k < 3; k++)
			//sum += fixmulf(oldrot[3*i + k], base_rot[k][j]);
			sum += fixmulf(oldrot[3*k + j], base_rot[i][k]);

		rot[3*i + j] = sum;
	}

	/*
	for(i = 0; i < 3; i++)
	{
		GLfixed sum = 0;
		for(k = 0; k < 3; k++)
			sum += fixmulf(oldtrn[k], rot[k*3 + i]);
			//sum += fixmulf(oldtrn[k], rot[i*3 + k]);
			//sum += fixmulf(oldtrn[k], base_rot[k][i]);
			//sum += fixmulf(oldtrn[k], base_rot[i][k]);

		//trn[i] = sum;
	}
	*/
}

GLvoid glTranslatex(GLfixed x, GLfixed y, GLfixed z) // p36 2.9.2
{
	int i, j;

	gl_mat_gte_isdirty = GL_TRUE;

	GLint stackidx = gl_mat_stack[gl_mat_cur];
	GLfixed *rot = gl_mat_rot[gl_mat_cur][stackidx];
	GLfixed *trn = gl_mat_trn[gl_mat_cur][stackidx];

	GLfixed xyz[3] = {x, y, z};

	for(i = 0; i < 3; i++)
	for(j = 0; j < 3; j++)
		//trn[i] += fixmul(rot[i*3 + j], xyz[j]);
		trn[j] += fixmul(rot[i*3 + j], xyz[i]);

	/*
	trn[0] += x;
	trn[1] += y;
	trn[2] += z;
	*/
}

GLvoid glPushMatrix(GLvoid) // p37 2.9.2
{
	// Prevent stack overflow
	if(gl_mat_stack[gl_mat_cur]+1 >= GLINTERNAL_MAX_MATRIX_STACK)
	{
		gl_internal_set_error(GL_STACK_OVERFLOW);
		return;
	}

	// Copy to new cell
	GLint stackidx = gl_mat_stack[gl_mat_cur];
	memcpy( gl_mat_rot[gl_mat_cur][stackidx+1],
		gl_mat_rot[gl_mat_cur][stackidx+0],
		sizeof(GLfixed)*9);
	memcpy( gl_mat_trn[gl_mat_cur][stackidx+1],
		gl_mat_trn[gl_mat_cur][stackidx+0],
		sizeof(GLfixed)*3);

	// Push
	gl_mat_stack[gl_mat_cur]++;
}

GLvoid glPopMatrix(GLvoid) // p37 2.9.2
{
	// Prevent stack underflow
	if(gl_mat_stack[gl_mat_cur]-1 < 0)
	{
		gl_internal_set_error(GL_STACK_UNDERFLOW);
		return;
	}

	gl_mat_gte_isdirty = GL_TRUE;

	// Pop
	gl_mat_stack[gl_mat_cur]--;
}

