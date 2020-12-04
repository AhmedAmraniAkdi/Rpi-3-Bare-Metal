/* Quick overview:
 *  - MMU is used to virtualize memory, different processes can think they are in the same address but syke they are on different physical addresses,
 *    they can even not be contiguos, or not put entirely on memory.
 *  - Allows to describe memory pages with different attributes, that way we can get isolation, being able to get cached, shareable or not, etc.
 *  - Kernel sees all memory mapped 1:1 that way he can allocate new pages to processes that need them.
 *  - How does it work? We take the virtual address and seeing how we configured some registers we start a table walk: first we see if we will be using
 *    TTBR_L0 or TTBR_L1 (63 - 48), these points to the first entry of the first translation table (it has 512 positions) we'll calle it L1 table, then some bits 
 *    (47 -39) indicate the index on this L1 table, then at position L1_table[index] we will have a descriptor (8 bytes) indicating the address of the next translation 
 *    table L2, and then again some bits (38- 30) of the virtual address will indicate the index and this goes on and on, until the descriptor indicates that the next 
 *    address is a memory address and not a pointer to the next table and then we will use the remaining bits to find the offset on the memory page.
 *    
 *  - virtual address format (4K granule): bit(63-48), bits(47-39), bits(38-30), bits(29-21), bits(20-12), bits(11-0) - can be different, depends on register TCL_EL1 
 *    (T0SZ and T1SZ control the address space - how many MSB are set to 1 or to 0 always, and the granule size)!!
 * 
 *  - Rpi3B+ has 1 GB ram + a bit more as in QA7_rev3.4.pdf, I will just use a 1:1 mapping, not virualization as i won't be doing user processes, so,
 *    we will need 2 translation levels, why? we will use bits (20 - 0) as offset in memory so 2MB per block, bits (29 - 20) gives us 512 entries, so
 *    512 * 2 Mb = 1GB, so to finish we will use 2 entries on table (38 - 30) for 2GB, since we need a bit more than 1Gb. this leaves us with: using TTBR_L0 
 *    ( all msb 0), using T0SZ = 33 -> leaves us with 64 - 33 = 31 bits -> 2GB. 
 *    Memory layout is: 0 -> VC core -> MMIO -> QA7_rev3.4.pdf
 * 
 *  - Why use mmu for 1:1 mapping? enabling D and I caches and some other attributes, we can use MAIR register  where we store attributes configurations 
 *    and in the descriptor we indicate the index.
 * 
 *  - There is also a TLB cache in order to make it easier to do the walk table, translations are directly stored, that way we wont need to do the table walk everyime
 *    but we need to make TLB maintenance if we change the tables, because we can have same virtual addresses but mapped to different physical addresses (won't be a
 *    a problem here), this is also a reasonw e have TTBRL0 and TTBRL1, normally one goes to the user and the other to the kernel imaging having only 1, 
 *    then we will need to be maintaining everytime we change from kernel to user (context switch) and populating the tables, etc.
 *    Just found out you can tag TLB with process ASID (identifiers) so that makes it easier i guess, lots and lots of stuff out there
 * 
 *  - How does cache work? the first time the kernel access and memory cell, it stores the address and it contents on a cache line (tag), the next time we access the
 *    same memory address we go look for it at cache first, if there isnt there then to memory, this poses a problem -> coherence-> there is hardware and attributes
 *    that ensure coherence. Maintenance can be needed when: non coherent DMA, changing instructions, changing memory attributes -> we won't do any of these we, because
 *    the kernel always sees the same memory and nothign moves around.
 * 
 *  - The descriptor format depends on translation stage and levels, the memory attributes differ, but the basic struct is: 
 *      some upper MSB for upper attributes, bits 47 to 12 memory address of next table/memory address, then some other lower mem attributes.
 *
 *   - Walking procces:
 *
 *                                 Virtual address                                               Physical Memory
 *        +-----------------------------------------------------------------------+              +------------------+
 *        |         | PGD Index | PUD Index | PMD Index |      Section offset     |              |                  |
 *        +-----------------------------------------------------------------------+              |                  |
 *        63        47     |    38      |   29     |    20            |           0              |    Section N     |
 *                         |            |          |                  |                    +---->+------------------+
 *                         |            |          |                  |                    |     |                  |
 *                  +------+            |          |                  |                    |     |                  |
 *                  |                   |          +----------+       |                    |     |------------------|
 *        +------+  |     PGD(LVL0)     |                     |       +------------------------->| Physical address |
 *        | ttbr |---->+-------------+  |       PUD(LVL1)     |                            |     |------------------|
 *        +------+  |  |             |  | +->+-------------+  |         PMD(LVL2)          |     |                  |
 *                  |  +-------------+  | |  |             |  | +->+-----------------+     |     +------------------+
 *                  +->| PUD address |----+  +-------------+  | |  |                 |     |     |                  |
 *                     +-------------+  +--->| PMD address |----+  +-----------------+     |     |                  |
 *                     |             |       +-------------+  +--->| Section address |-----+     |                  |
 *                     +-------------+       |             |       +-----------------+           |                  |
 *                                           +-------------+       |                 |           |                  |
 *                                                                 +-----------------+           |                  |
 *                                                                                               +------------------+
 * 
 *  - And as I said, not virtualization -> won't be needing a virtual page allocator, or to treat TTBR_L1 case
 * 
 * https://github.com/s-matyukevich/raspberry-pi-os/blob/master/docs/lesson06/rpi-os.md
 * https://lowenware.com/blog/osdev/aarch64-mmu-programming/
 * https://github.com/LdB-ECM/Raspberry-Pi/tree/master/10_virtualmemory
 * https://armv8-ref.codingbelief.com/en/chapter_d4/d43_1_vmsav8-64_translation_table_descriptor_formats.html
 * https://developer.arm.com/architectures/learn-the-architecture/aarch64-virtualization/stage-2-translation
 */

// will keep it as simple as possible using only what we need.

#include "mmu.h"
#include "rpi.h"
#include "VCmailbox.h"
#include <stdint.h>

static int populated = 0;

// need alignement, address is at 12th bit
static uint64_t __attribute__((aligned(4096))) L2_table[1024] = {0};
static uint64_t __attribute__((aligned(4096))) L1_table[2] = {0};

void populate_tables(void){
    uint32_t i_block = 0;
    uint32_t vc_mem_block = vc_mem_start_address()/(1 << 21); // divide by 2MB to get the memory address in 2MB block counts

    // we start to populate
    // level 1 table
    // the 3 for next is a table address
    // the 8 for non secure
    L1_table[0] = (0x8000000000000000) | ((uintptr_t)&L2_table[0]) | 3;
    L1_table[1] = (0x8000000000000000) | ((uintptr_t)&L2_table[512]) | 3;

    // level 2 table
    // 0x0 to Vc_mem
    for(i_block = 0; i_block < vc_mem_block; i_block++){
        L2_table[i_block] = (((uintptr_t)i_block) << 21)         // 2MB memory block
                            | (1 << 10)                          // accesability flag
                            | (STAGE2_SH_INNER_SHAREABLE << 8 )  // shareability // depends on how Rpi was built?
                            | (MT_NORMAL << 2)                   // memory type attribute on MAIR
                            | (1 << 0);                          // block descriptor
                            // other attributes are for access permission and security
    }

    // Vc_mem to MMIO (MMIO is 0x3F000000 which is 1GB - 16 MB which is 512 - 8 in terms of blocks)
    for(; i_block < (512 - 8); i_block++){
        L2_table[i_block] = (((uintptr_t)i_block) << 21)         // 2MB memory block
                            | (1 << 10)                          // accesability flag
                            | (MT_NORMAL_NC << 2)                // memory type attribute on MAIR
                            | (1 << 0);                          // block descriptor
                            // other attributes are for access permission and security
    /* Note:
    * The shareability field is only relevant if the memory is a Normal Cacheable memory type. All Device and Normal
    * Non-cacheable memory regions are always treated as Outer Shareable, regardless of the translation table
    * shareability attributes.
    */
    }

    // the 16 MB of peripherals
    for(; i_block < 512; i_block++){
        L2_table[i_block] = (((uintptr_t)i_block) << 21)         // 2MB memory block
                            | (1 << 10)                          // accesability flag
                            | (MT_DEVICE_NGNRNE << 2)            // memory type attribute on MAIR
                            | (1 << 0);                          // block descriptor
                            // other attributes are for access permission and security
    }

    // finally the 2MB of mailboxes and QA7_rev stuff
    L2_table[512] = (((uintptr_t)512) << 21)             // 2MB memory block
                    | (1 << 10)                          // accesability flag
                    | (MT_DEVICE_NGNRNE << 2)            // memory type attribute on MAIR
                    | (1 << 0);                          // block descriptor
                     // other attributes are for access permission and security

    // and that's it
    // won't do virtualization so no need for virtual allocation, or another set of virtual tables for TTBRL1
    populated = 1;
}

extern void enable_mmu_tables(uint64_t *table_entry);

#define CORE_MAILBOX_WRITETOSET 0x40000080
void mmu_enable_secondary(void){
    enable_mmu_tables(&L1_table[0]);
}
void mmu_enable(void){
    if(!populated){
        populate_tables();
    }
	enable_mmu_tables(&L1_table[0]);
    PUT32(CORE_MAILBOX_WRITETOSET + 16, (uintptr_t)&mmu_enable_secondary);
	PUT32(CORE_MAILBOX_WRITETOSET + 32, (uintptr_t)&mmu_enable_secondary);
	PUT32(CORE_MAILBOX_WRITETOSET + 48, (uintptr_t)&mmu_enable_secondary);
	WAKE_CORES();
}