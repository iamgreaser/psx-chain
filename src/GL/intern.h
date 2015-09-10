// begin
// note, we don't use alpha at all
extern GLenum gl_begin_mode;
extern GLint gl_begin_idx;
extern GLint gl_begin_gourcount;
extern uint32_t gl_begin_colcur;
extern uint32_t gl_begin_texcur;
extern uint32_t gl_begin_vtxbeg[2];
extern uint32_t gl_begin_colbeg;
extern uint32_t gl_begin_texbeg;
extern uint32_t gl_begin_vtxbuf[4][2];
extern uint32_t gl_begin_colbuf[4];
extern uint32_t gl_begin_texbuf[4];

// clear
extern GLubyte gl_clear_color_ub[4];

// draw
GLvoid gl_internal_push_triangle(GLuint i0, GLuint i1, GLuint i2);
GLvoid gl_internal_push_triangle_fromlist(uint32_t count, uint32_t *data, uint32_t *zdata);

// enable
extern GLboolean gl_enable_cull_face;
extern GLboolean gl_enable_depth_test;

// error
extern GLenum gl_error;
GLvoid gl_internal_set_error(GLenum error);

// list
#define GLINTERNAL_MAX_LIST 1024
typedef struct GLlist
{
	GLsizei size;
	GLsizei space;
	uint32_t data[];
} GLlist_s;
extern GLlist_s *gl_list_alloc[GLINTERNAL_MAX_LIST];
extern GLuint gl_list_cur;
extern GLenum gl_list_mode;
GLvoid gl_internal_list_add(GLsizei count, GLsizei zcount, uint32_t *data);

// matrix
#define GLINTERNAL_MAX_MATRIX_STACK 16
extern GLfixed gl_mat_rot[3][GLINTERNAL_MAX_MATRIX_STACK][9];
extern GLfixed gl_mat_trn[3][GLINTERNAL_MAX_MATRIX_STACK][3];
extern GLint gl_mat_stack[3];
extern GLint gl_mat_cur;
extern GLboolean gl_mat_gte_isdirty;
extern GLvoid gl_internal_flush_matrix(GLvoid);

// tex
#define GLINTERNAL_MAX_TEX 1024
typedef struct GLtex
{
	GLushort x, y;
	GLushort w, h;
	GLushort bits;
	GLushort clut;
	GLushort pad1[2];
} GLtex_s;
extern GLuint gl_tex_bind2d;
extern GLtex_s gl_tex_handle[GLINTERNAL_MAX_TEX];
extern GLshort gl_tex_map[64][512];

// viewport
extern GLint   gl_vp_x, gl_vp_y;
extern GLsizei gl_vp_w, gl_vp_h;


