static void vec3_set(vec3 *v, fixed x, fixed y, fixed z)
{
	(*v)[0] = x;
	(*v)[1] = y;
	(*v)[2] = z;
}

static void vec3_copy(vec3 *v, vec3 *a)
{
	memcpy((*v), (*a), sizeof(vec3));
}

static void vec4_copy_from_vec3(vec4 *v, vec3 *a)
{
	memcpy((*v), (*a), sizeof(vec3));
	(*v)[3] = 0x10000;
}

static void vec4_copy(vec4 *v, vec4 *a)
{
	memcpy((*v), (*a), sizeof(vec4));
}

static fixed vec4_dot_3(vec4 *a, vec4 *b)
{
	int i;
	fixed sum = 0;

	for(i = 0; i < 3; i++)
		sum += fixmul((*a)[i], (*b)[i]);
	
	return sum;
}

static fixed vec4_selfdot_3(vec4 *v)
{
	int i;
	fixed sum = 0;

	for(i = 0; i < 3; i++)
		sum += fixmul((*v)[i], (*v)[i]);
	
	return sum;
}

static void vec4_cross(vec4 *v, vec4 *a, vec4 *b)
{
	(*v)[0] = fixmulf((*a)[1], (*b)[2]) - fixmulf((*a)[2], (*b)[1]);
	(*v)[1] = fixmulf((*a)[2], (*b)[0]) - fixmulf((*a)[0], (*b)[2]);
	(*v)[2] = fixmulf((*a)[0], (*b)[1]) - fixmulf((*a)[1], (*b)[0]);
	(*v)[3] = 0;
}

static void vec4_cross_origin(vec4 *v, vec4 *a, vec4 *o, vec4 *b)
{
	int i;

	vec4 ao, bo;
	for(i = 0; i < 3; i++)
	{
		ao[i] = (*a)[i] - (*o)[i];
		bo[i] = (*b)[i] - (*o)[i];
	}

	vec4_cross(v, &ao, &bo);
}

static void vec4_normalize_3(vec4 *v)
{
	int i;

	fixed sum2 = vec4_selfdot_3(v);
	fixed rcp = fixisqrt(sum2);

	for(i = 0; i < 3; i++)
		(*v)[i] = fixmulf((*v)[i], rcp);
}

static void mat4_load_identity(mat4 *M)
{
	int i, j;

	for(i = 0; i < 4; i++)
	for(j = 0; j < 4; j++)
		(*M)[i][j] = (i==j ? (1<<16) : (0<<16));
}

static void mat4_copy(mat4 *M, mat4 *A)
{
	memcpy((*M), (*A), sizeof(mat4));
}

static void mat4_mul_mat4_mat4(mat4 *M, mat4 *A, mat4 *B)
{
	int i,j,k;

	for(i = 0; i < 4; i++)
	for(j = 0; j < 3; j++)
	{
		fixed sum = 0;
		for(k = 0; k < 4; k++)
			sum += fixmul((*A)[i][k], (*B)[k][j]);

		(*M)[i][j] = sum;
	}

	// this actually does speed it up a bit
	for(j = 0; j < 3; j++)
		(*M)[j][3] = 0;
	(*M)[3][3] = 0x10000;
}

static void mat4_mul_mat4_post(mat4 *M, mat4 *A)
{
	mat4 T;
	mat4_mul_mat4_mat4(&T, M, A);
	mat4_copy(M, &T);
}

static void mat4_mul_mat4_pre(mat4 *M, mat4 *A)
{
	mat4 T;
	mat4_mul_mat4_mat4(&T, A, M);
	mat4_copy(M, &T);
}

static void mat4_translate_imm3(mat4 *M, fixed x, fixed y, fixed z)
{
	(*M)[3][0] += x;
	(*M)[3][1] += y;
	(*M)[3][2] += z;
}

static void mat4_translate_vec3(mat4 *M, vec3 *v)
{
	(*M)[3][0] += (*v)[0];
	(*M)[3][1] += (*v)[1];
	(*M)[3][2] += (*v)[2];
}

static void mat4_translate_vec4(mat4 *M, vec4 *v)
{
	(*M)[3][0] += (*v)[0];
	(*M)[3][1] += (*v)[1];
	(*M)[3][2] += (*v)[2];
}

static void mat4_translate_vec3_neg(mat4 *M, vec3 *v)
{
	(*M)[3][0] -= (*v)[0];
	(*M)[3][1] -= (*v)[1];
	(*M)[3][2] -= (*v)[2];
}

static void mat4_translate_vec4_neg(mat4 *M, vec4 *v)
{
	(*M)[3][0] -= (*v)[0];
	(*M)[3][1] -= (*v)[1];
	(*M)[3][2] -= (*v)[2];
}

static void mat4_rotate_y(mat4 *M, fixed ang)
{
	int i;

	fixed vs = fixsin(ang);
	fixed vc = fixcos(ang);

	for(i = 0; i < 4; i++)
	{
		vec4 *sv = &(*M)[i];

		fixed tx = (*sv)[0];
		fixed tz = (*sv)[2];

		(*sv)[0] = fixmul(tx, vc) - fixmul(tz, vs);
		(*sv)[2] = fixmul(tx, vs) + fixmul(tz, vc);
	}
}

static void mat4_rotate_x(mat4 *M, fixed ang)
{
	int i;

	fixed vs = fixsin(ang);
	fixed vc = fixcos(ang);

	for(i = 0; i < 4; i++)
	{
		vec4 *sv = &(*M)[i];

		fixed ty = (*sv)[1];
		fixed tz = (*sv)[2];

		(*sv)[1] = fixmul(ty, vc) - fixmul(tz, vs);
		(*sv)[2] = fixmul(ty, vs) + fixmul(tz, vc);
	}
}

static void mat4_rotate_z(mat4 *M, fixed ang)
{
	int i;

	fixed vs = fixsin(ang);
	fixed vc = fixcos(ang);

	for(i = 0; i < 4; i++)
	{
		vec4 *sv = &(*M)[i];

		fixed tx = (*sv)[0];
		fixed ty = (*sv)[1];

		(*sv)[0] = fixmul(tx, vc) - fixmul(ty, vs);
		(*sv)[1] = fixmul(tx, vs) + fixmul(ty, vc);
	}
}

static void mat4_scale(mat4 *M, fixed s)
{
	int i, j;

	for(i = 0; i < 3; i++)
	for(j = 0; j < 3; j++)
		(*M)[i][j] = fixmul((*M)[i][j], s);
}

static void mat4_apply_vec4(vec4 *v, mat4 *A)
{
	int i, j;
	vec4 sv;
	vec4_copy(&sv, v);

	for(i = 0; i < 4; i++)
	{
		fixed sum = 0;

		sum += (((*A)[0][i])>>8)*(sv[0]>>8);
		sum += (((*A)[1][i])>>8)*(sv[1]>>8);
		sum += (((*A)[2][i])>>8)*(sv[2]>>8);
		sum += (((*A)[3][i])>>8)*(sv[3]>>8);

		(*v)[i] = sum;
	}

}

