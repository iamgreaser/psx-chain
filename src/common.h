#include <string.h>
#include <stdint.h>
#include <math.h>

#include "psx.h"

typedef int32_t fixed;
typedef int64_t fixed64;
typedef fixed vec3[3];
typedef fixed vec4[4];
typedef vec4 mat4[4];

#define FM_PI ((fixed)0x00008000)

#include "GL/gl.h"

// dma
void dma_init_block(void);
void dma_init(void);
void dma_wait(void);
void dma_flush(void);
void dma_send_prim(uint32_t count, uint32_t *data, int32_t otz);
void screen_print(int x, int y, uint32_t c, const char *str);

// fix
int fixtoi(fixed v);
fixed itofix(int v);
fixed fixmul(fixed a, fixed b);
fixed fixmulf(fixed a, fixed b);
fixed fixdiv(fixed a, fixed b);
fixed fixsqrt(fixed v);
int32_t intsqrt(int32_t v);
fixed fixisqrt(fixed v);
fixed fixsin(fixed ang);
fixed fixcos(fixed ang);
fixed fixrand1s(void);

// gpu
void gpu_send_control_gp0(int v);
void gpu_send_control_gp1(int v);
void gpu_send_data(int v);
void gpu_display_start(int x1, int y1);
void gpu_crtc_range(int x1, int y1, int w, int h);
void gpu_draw_offset(int x, int y);
void gpu_draw_texmask(int w, int h, int ox, int oy);
void gpu_draw_range(int x1, int y1, int x2, int y2);
void gpu_push_vertex(int x, int y);
void gpu_init(void);

// gte
uint32_t gte_get_flag(void);
void gte_init(int ofx, int ofy, int h, int otzmax);
void gte_loadmat_rot_full(mat4 *M);
void gte_loadmat_gl(GLfixed *MR, GLfixed *MT);
void gte_load_v0_vec3(const vec3 *v);
void gte_load_v012_vec3(const vec3 *v0, const vec3 *v1, const vec3 *v2);
void gte_load_v012_gl(const uint32_t *v0, const uint32_t *v1, const uint32_t *v2);
void gte_save_ir123_ptr3(fixed *ir1, fixed *ir2, fixed *ir3);
void gte_save_s0_vec3(vec3 *v);
void gte_save_s012_vec3(vec3 *v0, vec3 *v1, vec3 *v2);
void gte_save_s012xy_ui32_t(uint32_t *xy0, uint32_t *xy1, uint32_t *xy2);
int gte_get_side(void);
void gte_cmd_rtps(void);
void gte_cmd_rtpt(void);

extern volatile int screen_buffer;

