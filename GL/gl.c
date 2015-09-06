#include "GL/gl.h"

// begin
// note, we don't use alpha at all
static GLenum gl_begin_mode = 0;
static GLint gl_begin_idx = 0;
static uint32_t gl_begin_colcur = 0x00FFFFFF;
static uint32_t gl_begin_texcur = 0x00000000;
static uint32_t gl_begin_vtxbeg[2];
static uint32_t gl_begin_colbeg;
static uint32_t gl_begin_texbeg;
static uint32_t gl_begin_vtxbuf[4][2];
static uint32_t gl_begin_colbuf[4];
static uint32_t gl_begin_texbuf[4];

// clear
static GLubyte gl_clear_color_ub[4] = {0, 0, 0, 0};

// error
static GLenum gl_error = GL_NO_ERROR;
GLvoid gl_internal_set_error(GLenum error);

// matrix
#define GLINTERNAL_MAX_MATRIX_STACK 10
static GLfixed gl_mat_rot[3][GLINTERNAL_MAX_MATRIX_STACK][9];
static GLfixed gl_mat_trn[3][GLINTERNAL_MAX_MATRIX_STACK][3];
static GLint gl_mat_stack[3] = {0, 0, 0};
static GLint gl_mat_cur = 0;
static GLboolean gl_mat_gte_isdirty = GLtrue;
GLvoid gl_internal_flush_matrix(GLvoid);

// viewport
static GLint   gl_vp_x = 0, gl_vp_y = 0;
static GLsizei gl_vp_w = 0, gl_vp_h = 0;

#include "GL/begin.c"
#include "GL/clear.c"
#include "GL/error.c"
#include "GL/matrix.c"
#include "GL/viewport.c"


