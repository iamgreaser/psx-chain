#ifndef _GL_GL_H_
#define _GL_GL_H_
// NOTE: page references are PDF pages in glspec.pdf, not the printed page numbers!

// standard types (p19, 2.4 Basic GL Operation)
// (fixed point types are from the GLES 1.1.12 spec)
typedef void GLvoid;
typedef uint8_t GLboolean;
typedef int8_t GLbyte;
typedef uint8_t GLubyte;
typedef int16_t GLshort;
typedef uint16_t GLushort;
typedef int32_t GLint;
typedef uint32_t GLuint;
typedef int32_t GLenum;
typedef size_t GLsizei;
typedef uint32_t GLbitfield;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef fixed GLfixed;
typedef fixed GLclampx;

#define GLtrue  1
#define GLfalse 0
#define GL_TRUE  1
#define GL_FALSE 0

// enum: 'E' errors (p22, 2.5 GL Errors, table 2.3)
#define GL_NO_ERROR 0
#define GL_INVALID_ENUM 0x4501
#define GL_INVALID_VALUE 0x4502
#define GL_INVALID_OPERATION 0x4503
#define GL_STACK_OVERFLOW 0x4504
#define GL_STACK_UNDERFLOW 0x4505
#define GL_OUT_OF_MEMORY 0x4506

// enum: 'e' enableables (added as we find them)
#define GL_NORMALIZE 0x6501 /* p38, 2.9.3 Normal Transformation */
#define GL_CULL_FACE 0x6502 /* p72, 3.5.1 Basic Polygon Rasterization */
#define GL_DEPTH_TEST 0x6503 /* p106, 4.15 Depth Buffer Test */

// enum: 'M' matrices (p34, 2.9 Coordinate Transformations)
// we are probably going to ignore GL_PROJECTION.
#define GL_MODELVIEW 0x4D01
#define GL_TEXTURE 0x4D02
#define GL_PROJECTION 0x4D03

// enum: 'P' primitives (p25-27, 2.6 Begin/End Paradigm)
// we're keeping GL_POLYGON separate from GL_TRIANGLE_FAN
// so we can subdiv it nicely later if we feel like adding that support
// even though the latter is perfectly implementation-correct for GL_POLYGON
// "Only convex polygons are guaranteed to be drawn correctly by the GL." p25
#define GL_POINTS 0x5010
#define GL_LINES 0x5020
#define GL_LINE_STRIP 0x5021
#define GL_LINE_LOOP 0x5023
#define GL_TRIANGLES 0x5030
#define GL_TRIANGLE_STRIP 0x5031
#define GL_TRIANGLE_FAN 0x5033
#define GL_QUADS 0x5040
#define GL_QUAD_STRIP 0x5041
#define GL_POLYGON 0x5070

// begin
GLvoid glBegin(GLenum mode); // p24 2.6.1
GLvoid glEnd(GLvoid); // p24 2.6.1
GLvoid glColor3ub(GLubyte r, GLubyte g, GLubyte b);
GLvoid glVertex3x(GLfixed x, GLfixed y, GLfixed z);

// clear
GLvoid glClearColorx(GLfixed r, GLfixed g, GLfixed b, GLfixed a);
GLvoid glClear(GLbitfield mask);

// draw
GLvoid glFlush(GLvoid); // p138, 5.5
GLvoid glFinish(GLvoid); // p138, 5.5

// enable
GLvoid glEnable(GLenum cap); // p38 2.9.3
GLvoid glDisable(GLenum cap); // p38 2.9.3

// error
GLenum glGetError(GLvoid); // p20 2.5

// matrix
GLvoid glLoadIdentity(GLvoid); // p35 2.9.2
GLvoid glMatrixMode(GLenum mode); // p34 2.9.2
GLvoid glRotatex(GLfixed theta, GLfixed x, GLfixed y, GLfixed z); // p35 2.9.2
GLvoid glTranslatex(GLfixed x, GLfixed y, GLfixed z); // p36 2.9.2
GLvoid glPushMatrix(GLvoid); // p37 2.9.2
GLvoid glPopMatrix(GLvoid); // p37 2.9.2

// viewport
GLvoid glViewport(GLint x, GLint y, GLsizei w, GLsizei h); // p34 2.9.1

#endif

