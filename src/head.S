	/*.org 0x10800*/
	.section ".boot"

	.ascii "PS-X EXE"
	.word 0, 0
	.word aaa_very_start, 0
	.word load_base, _LOADABLE_SIZE
	.word 0, 0
	.word 0, 0
	.word 0x801FFF00, 0
	.word 0, 0

	.word 0, 0
	.word 0
	/*.ascii "Sony Computer Entertainment Inc. for North America area"*/
	.ascii "Sony Computer Entertainment Inc. for Europe area"
	.align 11
load_base:

	.section ".text"
aaa_very_start:
	/* Load stack pointer */
	/*lui $sp, 0x801F
	ori $sp, $sp, 0xFF00*/

	/* Load global pointer */
	lui $gp, %hi(_gp)
	ori $gp, $gp, %lo(_gp)

	/* Call .init */
	lui $t0, %hi(_INIT_SECTION)
	ori $t0, $t0, %lo(_INIT_SECTION)
	jalr $t0

	/* Jump to start */
	jal aaa_start
	nop
	j aaa_very_start
	nop

	.global int_handler_stub
	.global int_handler_stub_end
int_handler_stub:
	lui $k0, %hi(int_handler)
	ori $k0, %lo(int_handler)
	jr $k0
int_handler_stub_end:

int_handler:
	/* Push all the things */
	addiu $sp, $sp, -19*4-64
	.set noat
	sw $1, 4*1($sp)
	.set at
	sw $ra, 4*0($sp)
	sw $2, 4*2($sp)
	sw $3, 4*3($sp)
	sw $4, 4*4($sp)
	sw $5, 4*5($sp)
	sw $6, 4*6($sp)
	sw $7, 4*7($sp)
	sw $8, 4*8($sp)
	sw $9, 4*9($sp)
	sw $10, 4*10($sp)
	sw $11, 4*11($sp)
	sw $12, 4*12($sp)
	sw $13, 4*13($sp)
	sw $14, 4*14($sp)
	sw $15, 4*15($sp)
	sw $t8, 4*17($sp)
	sw $t9, 4*18($sp)

	mfc0 $t0, $14
	sw $t0, 4*16($sp)

	.extern isr_handler_c
	mfc0 $a0, $12
	mfc0 $a1, $13
	mfc0 $a2, $14
	addiu $sp, $sp, -32*4
	jal isr_handler_c
	addiu $sp, $sp, 32*4

	/* Pop EPC */
	lw $k0, 4*16($sp)

	/* Offset - needed in some cases*/
	/* Check for COP1 */
	lw $t2, 0($k0)
	lui $t0, 0xFE00
	lui $t1, 0x4A00
	and $t0, $t0, $t2
	beq $t0, $t1, isr_handler_do_offset
	nop
	/* Check for SYSCALL */
	ori $t1, $zero, 0x000C
	bne $t2, $t1, isr_handler_skip_offset
	nop

	/* Apply offset */
isr_handler_do_offset:
	addi $k0, $k0, 4
isr_handler_skip_offset:

	/* Pop all the things */
	lw $ra, 4*0($sp)
	lw $2, 4*2($sp)
	lw $3, 4*3($sp)
	lw $4, 4*4($sp)
	lw $5, 4*5($sp)
	lw $6, 4*6($sp)
	lw $7, 4*7($sp)
	lw $8, 4*8($sp)
	lw $9, 4*9($sp)
	lw $10, 4*10($sp)
	lw $11, 4*11($sp)
	lw $12, 4*12($sp)
	lw $13, 4*13($sp)
	lw $14, 4*14($sp)
	lw $15, 4*15($sp)
	lw $t8, 4*17($sp)
	lw $t9, 4*18($sp)
	.set noat
	lw $1, 4*1($sp)
	.set at
	addiu $sp, $sp, 19*4+64
	rfe /* assembler is currently set to reorder stuff into branch delay slot */
	jr $k0
	nop

	.section ".rodata"
	.align 5
	.global fsys_rawcga
fsys_rawcga:
	.incbin "rawcga.bin"
	.align 5
	.global fsys_s3m_test
fsys_s3m_test:
	.incbin "dat/s2-cez.s3m"
	.align 5

