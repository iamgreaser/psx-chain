#include "common.h"
#include "GL/intern.h"

// begin
// note, we don't use alpha at all
GLenum gl_begin_mode = 0;
GLint gl_begin_idx = 0;
GLint gl_begin_gourcount = 0;
uint32_t gl_begin_colcur = 0x00FFFFFF;
uint32_t gl_begin_texcur = 0x00000000;
uint32_t gl_begin_vtxbeg[2];
uint32_t gl_begin_colbeg;
uint32_t gl_begin_texbeg;
uint32_t gl_begin_vtxbuf[4][2];
uint32_t gl_begin_colbuf[4];
uint32_t gl_begin_texbuf[4];
uint32_t gl_begin_texclut;
uint32_t gl_begin_texpage;
uint32_t gl_begin_texmask;

// clear
GLubyte gl_clear_color_ub[4] = {0, 0, 0, 0};

// draw
GLvoid gl_internal_push_triangle(GLuint i0, GLuint i1, GLuint i2);

// enable
GLboolean gl_enable_cull_face = GL_FALSE;
GLboolean gl_enable_depth_test = GL_FALSE;
GLboolean gl_enable_texture_2d = GL_FALSE;

// error
GLenum gl_error = GL_NO_ERROR;
GLvoid gl_internal_set_error(GLenum error);

// list
GLlist_s *gl_list_alloc[GLINTERNAL_MAX_LIST];
GLuint gl_list_cur = 0;
GLenum gl_list_mode = 0;

// matrix
GLfixed gl_mat_rot[3][GLINTERNAL_MAX_MATRIX_STACK][9];
GLfixed gl_mat_trn[3][GLINTERNAL_MAX_MATRIX_STACK][3];
GLint gl_mat_stack[3] = {0, 0, 0};
GLint gl_mat_cur = 0;
GLboolean gl_mat_gte_isdirty = GL_TRUE;
GLvoid gl_internal_flush_matrix(GLvoid);

// tex
GLuint gl_tex_bind2d = 0;
GLtex_s gl_tex_handle[GLINTERNAL_MAX_TEX];
GLbyte gl_tex_map[64][512];

// viewport
GLint   gl_vp_x = 0, gl_vp_y = 0;
GLsizei gl_vp_w = 0, gl_vp_h = 0;

