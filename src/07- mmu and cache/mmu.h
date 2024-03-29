#ifndef _MMU_H
#define _MMU_H

// NC = non cachable
// NGN : no gathering
// NR : no re order
// NE : no early write acknowledge
typedef enum{
    MT_DEVICE_NGNRNE =	0,
    MT_DEVICE_NGNRE  =  1,
    MT_DEVICE_GRE	 =	2,
    MT_NORMAL_NC	 =	3,
    MT_NORMAL		 =  4,
} mem_attr;

typedef enum {
	STAGE2_SH_OUTER_SHAREABLE = 2,	//	Outter shareable
	STAGE2_SH_INNER_SHAREABLE = 3,	//	Inner shareable
} shareability;

void populate_tables(void);
void mmu_enable(void);

#endif