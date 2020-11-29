// https://github.com/LdB-ECM/Raspberry-Pi-Multicore/blob/master/xRTOS_MMU/SmartStart64.S
.equ MT_DEVICE_NGNRNE,	0
.equ MT_DEVICE_NGNRE,	1
.equ MT_DEVICE_GRE,		2
.equ MT_NORMAL_NC,		3
.equ MT_NORMAL,		    4
.equ MAIR1VAL, (0x00 << (MT_DEVICE_NGNRNE * 8))|(0x04 << (MT_DEVICE_NGNRE * 8))|(0x0c << (MT_DEVICE_GRE * 8))|(0x44 << (MT_NORMAL_NC * 8))|(0xff << (MT_NORMAL * 8))

/*.equ MAIR1VAL, ( (0x00 << (MT_DEVICE_NGNRNE * 8)) | // device non gathering, non reorder, non early write ack
                 (0x04 << (MT_DEVICE_NGNRE * 8)) |\ // device non gathering non reorder early write ack
				 (0x0c << (MT_DEVICE_GRE * 8)) |\ // device gathering reorder early write ack
                 (0x44 << (MT_NORMAL_NC * 8)) |\ // normal non cachable
				 (0xff << (MT_NORMAL * 8)) )    // normal inner/outer write back non transient write allocate cache*/

// Specify mapping characteristics in translate control register
// can you imaging ctrl+f not existing when reading documentation puah
// no need to define for TTBR1 since won't use it, for completness
#define TCREL1VAL  ( (0b00 << 37) |   /* TBI=0, no tagging */\
					 (0b00 << 32) |   /* IPS= 32 bit ... 000 = 32bit, 001 = 36bit, 010 = 40bit */\
					 (0b10 << 30)  |  /* TG1=4k ... options are 10=4KB, 01=16KB, 11=64KB ... take care differs from TG0 */\
					 (0b11 << 28)  |  /* SH1=3 inner ... options 00 = Non-shareable, 01 = INVALID, 10 = Outer Shareable, 11 = Inner Shareable */\
					 (0b01 << 26)  |  /* ORGN1=1 write back .. options 00 = Non-cacheable, 01 = Write back cacheable, 10 = Write thru cacheable, 11 = Write Back Non-cacheable */\
					 (0b01 << 24)  |  /* IRGN1=1 write back .. options 00 = Non-cacheable, 01 = Write back cacheable, 10 = Write thru cacheable, 11 = Write Back Non-cacheable */\
					 (0b0  << 23)  |  /* EPD1 ... Translation table walk disable for translations using TTBR1_EL1  0 = walk, 1 = generate fault */\
					 (25   << 16)  |  /* T1SZ=25 (512G) ... The region size is 2 POWER (64-T1SZ) bytes */\
					 (0b00 << 14)  |  /* TG0=4k  ... options are 00=4KB, 01=64KB, 10=16KB,  ... take care differs from TG1 */\
					 (0b11 << 12)  |  /* SH0=3 inner ... .. options 00 = Non-shareable, 01 = INVALID, 10 = Outer Shareable, 11 = Inner Shareable */\
					 (0b01 << 10)  |  /* ORGN0=1 write back .. options 00 = Non-cacheable, 01 = Write back cacheable, 10 = Write thru cacheable, 11 = Write Back Non-cacheable */\
					 (0b01 << 8)   |  /* IRGN0=1 write back .. options 00 = Non-cacheable, 01 = Write back cacheable, 10 = Write thru cacheable, 11 = Write Back Non-cacheable */\
					 (0b0  << 7)   |  /* EPD0  ... Translation table walk disable for translations using TTBR0_EL1  0 = walk, 1 = generate fault */\
					 (33   << 0) ) 	  /* T0SZ=25 (512G)  ... The region size is 2 POWER (64-T0SZ) bytes */

#define SCTLREL1VAL ( (0xC00800) |		/* set mandatory reserved bits */\
					  (1 << 12)  |      /* I, Instruction cache enable. This is an enable bit for instruction caches at EL0 and EL1 */\
					  (1 << 4)   |		/* SA0, stack alignment Check Enable for EL0 */\
					  (1 << 3)   |		/* SA, stack alignment Check Enable */\
					  (1 << 2)   |		/* C, Data cache enable. This is an enable bit for data caches at EL0 and EL1 */\
					  (1 << 1)   |		/* A, Alignment check enable bit */\
					  (1 << 0) )		/* set M, enable MMU */

.globl enable_mmu_tables;
enable_mmu_tables:
	dsb sy

	/* Set the memattrs values into mair_el1*/
	ldr x2, =MAIR1VAL
    msr mair_el1, x2

	/* Bring both tables online and execute memory barrier */
	msr ttbr0_el1, x0
	// msr ttbr1_el1, x1 won't use it
	isb

	ldr x0, =TCREL1VAL
	msr tcr_el1, x0
	isb

	mrs x0, sctlr_el1
	ldr x1, =SCTLREL1VAL
	orr x0, x0, x1
	msr sctlr_el1, x0

	ret