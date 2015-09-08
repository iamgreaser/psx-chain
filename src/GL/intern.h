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

// enable
extern GLboolean gl_enable_cull_face;
extern GLboolean gl_enable_depth_test;

// error
extern GLenum gl_error;
GLvoid gl_internal_set_error(GLenum error);

// list
#define GLINTERNAL_MAX_LIST 1024
extern GLuint *gl_list_alloc[GLINTERNAL_MAX_LIST];

// matrix
#define GLINTERNAL_MAX_MATRIX_STACK 10
extern GLfixed gl_mat_rot[3][GLINTERNAL_MAX_MATRIX_STACK][9];
extern GLfixed gl_mat_trn[3][GLINTERNAL_MAX_MATRIX_STACK][3];
extern GLint gl_mat_stack[3];
extern GLint gl_mat_cur;
extern GLboolean gl_mat_gte_isdirty;
extern GLvoid gl_internal_flush_matrix(GLvoid);

// viewport
extern GLint   gl_vp_x, gl_vp_y;
extern GLsizei gl_vp_w, gl_vp_h;


