#include "common.h"

static const int gte_loss_invec = 0;

uint32_t gte_get_flag(void)
{
	uint32_t ret;
	asm volatile("\tcfc2 %0, $31\n" : "=r"(ret) : : );
	return ret;
}

void gte_init(int ofx, int ofy, int h, int otzmax)
{
	asm volatile (
		"\tctc2 %0, $24\n"
		"\tctc2 %1, $25\n"
		"\tctc2 %2, $26\n"
		"\tctc2 %3, $29\n"
		"\tctc2 %4, $30\n"
		: :
		"r"(ofx), // OFX
		"r"(ofy), // OFY
		"r"(h), // H
		"r"(otzmax/0x30), // ZSF3
		"r"(otzmax/0x40) // ZSF4
		: );
}

void gte_loadmat_rot_full(mat4 *M)
{
	asm volatile (
		"\tctc2 %0, $0\n"
		"\tctc2 %1, $1\n"
		"\tctc2 %2, $2\n"
		"\tctc2 %3, $3\n"
		"\tctc2 %4, $4\n"
		: :
		"r"((((*M)[0][0]>>4)&0xFFFF)|(((*M)[1][0]>>4)<<16)),
		"r"((((*M)[2][0]>>4)&0xFFFF)|(((*M)[0][1]>>4)<<16)),
		"r"((((*M)[1][1]>>4)&0xFFFF)|(((*M)[2][1]>>4)<<16)),
		"r"((((*M)[0][2]>>4)&0xFFFF)|(((*M)[1][2]>>4)<<16)),
		"r"((*M)[2][2]>>4)
		: );

	fixed tx_prep = (*M)[3][0]>>gte_loss_invec;
	fixed ty_prep = (*M)[3][1]>>gte_loss_invec;
	fixed tz_prep = (*M)[3][2]>>gte_loss_invec;
	asm volatile (
		"\tctc2 %0, $5\n"
		"\tctc2 %1, $6\n"
		"\tctc2 %2, $7\n"
		: :
		"r"(tx_prep),
		"r"(ty_prep),
		"r"(tz_prep)
		: );
}

void gte_loadmat_gl(GLfixed *MR, GLfixed *MT)
{
	asm volatile (
		"\tctc2 %0, $0\n"
		"\tctc2 %1, $1\n"
		"\tctc2 %2, $2\n"
		"\tctc2 %3, $3\n"
		"\tctc2 %4, $4\n"
		: :
		"r"(((MR[0*3 + 0]>>4)&0xFFFF)|((MR[1*3 + 0]>>4)<<16)),
		"r"(((MR[2*3 + 0]>>4)&0xFFFF)|((MR[0*3 + 1]>>4)<<16)),
		"r"(((MR[1*3 + 1]>>4)&0xFFFF)|((MR[2*3 + 1]>>4)<<16)),
		"r"(((MR[0*3 + 2]>>4)&0xFFFF)|((MR[1*3 + 2]>>4)<<16)),
		"r"(MR[2*3 + 2]>>4)
		: );

	fixed tx_prep = MT[0]>>gte_loss_invec;
	fixed ty_prep = MT[1]>>gte_loss_invec;
	fixed tz_prep = MT[2]>>gte_loss_invec;
	asm volatile (
		"\tctc2 %0, $5\n"
		"\tctc2 %1, $6\n"
		"\tctc2 %2, $7\n"
		: :
		"r"(tx_prep),
		"r"(ty_prep),
		"r"(tz_prep)
		: );
}

void gte_load_v0_vec3(const vec3 *v)
{
	asm volatile (
		"\tmtc2 %0, $0\n"
		"\tmtc2 %1, $1\n"
		: :
		"r"((((*v)[0]>>gte_loss_invec)&0xFFFF)
		|(((*v)[1]>>gte_loss_invec)<<16)),
		"r"((*v)[2]>>gte_loss_invec)
	);
}

void gte_load_v012_vec3(const vec3 *v0, const vec3 *v1, const vec3 *v2)
{
	asm volatile (
		"\tmtc2 %0, $0\n"
		"\tmtc2 %1, $1\n"
		"\tmtc2 %2, $2\n"
		"\tmtc2 %3, $3\n"
		"\tmtc2 %4, $4\n"
		"\tmtc2 %5, $5\n"
		: :
		"r"((((*v0)[0]>>gte_loss_invec)&0xFFFF)
		|(((*v0)[1]>>gte_loss_invec)<<16)),
		"r"((*v0)[2]>>gte_loss_invec),
		"r"((((*v1)[0]>>gte_loss_invec)&0xFFFF)
		|(((*v1)[1]>>gte_loss_invec)<<16)),
		"r"((*v1)[2]>>gte_loss_invec),
		"r"((((*v2)[0]>>gte_loss_invec)&0xFFFF)
		|(((*v2)[1]>>gte_loss_invec)<<16)),
		"r"((*v2)[2]>>gte_loss_invec)
	);
}

void gte_load_v012_gl(const uint32_t *v0, const uint32_t *v1, const uint32_t *v2)
{
	asm volatile (
		"\tmtc2 %0, $0\n"
		"\tmtc2 %1, $1\n"
		"\tmtc2 %2, $2\n"
		"\tmtc2 %3, $3\n"
		"\tmtc2 %4, $4\n"
		"\tmtc2 %5, $5\n"
		: :
		"r"(v0[0]),
		"r"(v0[1]),
		"r"(v1[0]),
		"r"(v1[1]),
		"r"(v2[0]),
		"r"(v2[1])
	);
}

void gte_save_ir123_ptr3(fixed *ir1, fixed *ir2, fixed *ir3)
{
	asm volatile(
		"\tmfc2 %0, $9\n"
		"\tmfc2 %1, $10\n"
		"\tmfc2 %2, $11\n"
		:
		"=r"(*ir1),
		"=r"(*ir2),
		"=r"(*ir3)
		::);
}

void gte_save_s0_vec3(vec3 *v)
{
	int32_t res0_xy;
	int32_t res0_z;

	asm volatile(
		"\tmfc2 %0, $14\n"
		"\tmfc2 %1, $19\n"
		:
		"=r"(res0_xy),
		"=r"(res0_z)
		::);
	(*v)[0] = (fixed)((res0_xy<<16)>>16);
	(*v)[1] = (fixed)((res0_xy)>>16);
	(*v)[2] = (fixed)res0_z;
}

void gte_save_s012_vec3(vec3 *v0, vec3 *v1, vec3 *v2)
{
	int32_t res0_xy;
	int32_t res0_z;
	int32_t res1_xy;
	int32_t res1_z;
	int32_t res2_xy;
	int32_t res2_z;

	asm volatile(
		"\tmfc2 %0, $12\n"
		"\tmfc2 %1, $17\n"
		"\tmfc2 %2, $13\n"
		"\tmfc2 %3, $18\n"
		"\tmfc2 %4, $14\n"
		"\tmfc2 %5, $19\n"
		:
		"=r"(res0_xy),
		"=r"(res0_z),
		"=r"(res1_xy),
		"=r"(res1_z),
		"=r"(res2_xy),
		"=r"(res2_z)
		::);
	(*v0)[0] = (fixed)((res0_xy<<16)>>16);
	(*v0)[1] = (fixed)((res0_xy)>>16);
	(*v0)[2] = (fixed)res0_z;
	(*v1)[0] = (fixed)((res1_xy<<16)>>16);
	(*v1)[1] = (fixed)((res1_xy)>>16);
	(*v1)[2] = (fixed)res1_z;
	(*v2)[0] = (fixed)((res2_xy<<16)>>16);
	(*v2)[1] = (fixed)((res2_xy)>>16);
	(*v2)[2] = (fixed)res2_z;
}

void gte_save_s012xy_ui32_t(uint32_t *xy0, uint32_t *xy1, uint32_t *xy2)
{
	asm volatile(
		"\tmfc2 %0, $12\n"
		"\tmfc2 %1, $13\n"
		"\tmfc2 %2, $14\n"
		:
		"=r"(*xy0),
		"=r"(*xy1),
		"=r"(*xy2)
		::);
}

int gte_get_side(void)
{
	int32_t mac0;

	asm volatile (
		"\tcop2 0x1400006\n" // NCLIP
		"\tmfc2 %0, $24\n"
		:
		"=r"(mac0)
		::);
	
	return mac0;
}

void gte_cmd_rtps(void)
{
	asm volatile ("\tcop2 0x0180001\n");
}

void gte_cmd_rtpt(void)
{
	asm volatile ("\tcop2 0x0280030\n");
}

