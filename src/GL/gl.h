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

// enum: 'C' colour types for texture formats
// taken from the GL 1.2 man pages
#define GL_RED 0x4301
#define GL_GREEN 0x4302
#define GL_BLUE 0x4304
#define GL_ALPHA 0x4308
#define GL_LUMINANCE 0x4311
#define GL_LUMINANCE4 0x4321
#define GL_LUMINANCE8 0x4331
#define GL_LUMINANCE12 0x4341
#define GL_LUMINANCE16 0x4351
#define GL_LUMINANCE_ALPHA 0x4319
#define GL_LUMINANCE4_ALPHA4 0x4329
#define GL_LUMINANCE6_ALPHA2 0x4339
#define GL_LUMINANCE8_ALPHA8 0x4349
#define GL_LUMINANCE12_ALPHA4 0x4359
#define GL_LUMINANCE12_ALPHA12 0x4369
#define GL_LUMINANCE16_ALPHA16 0x4379
#define GL_INTENSITY 0x4312
#define GL_INTENSITY4 0x4322
#define GL_INTENSITY8 0x4332
#define GL_INTENSITY12 0x4342
#define GL_INTENSITY16 0x4352
#define GL_RG 0x4313
#define GL_RGB 0x4317
#define GL_RGBA 0x431F
#define GL_R3_G3_B2 0x4327
#define GL_RGB4 0x4337
#define GL_RGB5 0x4347
#define GL_RGB8 0x4357
#define GL_RGB10 0x4367
#define GL_RGB12 0x4377
#define GL_RGB16 0x4387
#define GL_RGBA2 0x432F
#define GL_RGBA4 0x433F
#define GL_RGB5_A1 0x434F
#define GL_RGBA8 0x435F
#define GL_RGB10_A2 0x436F
#define GL_RGBA12 0x437F
#define GL_RGBA16 0x438F
#define GL_COLOR_INDEX 0x4304

// extension: GL_EXT_paletted_texture
#define GL_COLOR_INDEX1_EXT 0x4314
#define GL_COLOR_INDEX2_EXT 0x4324
#define GL_COLOR_INDEX4_EXT 0x4334
#define GL_COLOR_INDEX8_EXT 0x4344
#define GL_COLOR_INDEX16_EXT 0x4354

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

// enum: 'I' integer storage stuff (also includes floats)
#define GL_BYTE 0x6908
#define GL_UNSIGNED_BYTE 0x6988
#define GL_SHORT 0x6910
#define GL_UNSIGNED_SHORT 0x6990
#define GL_INT 0x6920
#define GL_UNSIGNED_INT 0x69A0
#define GL_FLOAT 0x695F
#define GL_DOUBLE 0x697F
#define GL_BITMAP 0x6901
// extension
#define GL_NYBBLE_PSX 0x6904
#define GL_UNSIGNED_NYBBLE_PSX 0x6984
#define GL_FIXED_PSX 0x69DF

// enum: 'L' list modes
// p134, 5.4 Display Lists:
#define GL_COMPILE 0x4C11
#define GL_COMPILE_AND_EXECUTE 0x4C12

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

// enum: 'T' targets
#define GL_TEXTURE_2D 0x5402

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

// list
GLvoid glNewList(GLuint n, GLenum mode); // p134 5.4
GLvoid glEndList(GLvoid); // p134 5.4
GLvoid glCallList(GLuint n); // p134 5.4
GLuint glGenLists(GLsizei s); // p137 5.4

// matrix
GLvoid glLoadIdentity(GLvoid); // p35 2.9.2
GLvoid glMatrixMode(GLenum mode); // p34 2.9.2
GLvoid glRotatex(GLfixed theta, GLfixed x, GLfixed y, GLfixed z); // p35 2.9.2
GLvoid glTranslatex(GLfixed x, GLfixed y, GLfixed z); // p36 2.9.2
GLvoid glPushMatrix(GLvoid); // p37 2.9.2
GLvoid glPopMatrix(GLvoid); // p37 2.9.2

// tex
GLvoid glTexStealRangePSX(GLuint x, GLuint y, GLuint w, GLuint h);
GLvoid glTexImage2D(GLenum target, GLint level, GLint internalFormat,
	GLsizei width, GLsizei height, GLint border,
	GLenum format, GLenum type, const GLvoid *pixels);

// viewport
GLvoid glViewport(GLint x, GLint y, GLsizei w, GLsizei h); // p34 2.9.1

#endif

