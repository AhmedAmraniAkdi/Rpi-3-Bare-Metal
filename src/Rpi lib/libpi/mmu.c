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
 *  - Why use mmu for 1:1 mapping? enabling D and I caches and some other attributes, we can use MAIR register which is an array of 8 byte elements where we store 
 *    attributes configurations and in the descriptor we indicate the index.
 * 
 *  - There is also a TLB cache in order to make it easier to do the walk table, translations are directly stored, that way we wont need to do the table walk everyime
 *    but we need to make TLB maintenance if we change the tables, because we can have same virtual addresses but mapped to different physical addresses (won't be a
 *    a problem here), this is also a reasonw e have TTBRL0 and TTBRL1, normally one goes to the user and the other to the kernel imaging having only 1, 
 *    then we will need to be maintaining everytime we change from kernel to user (context switch) and populating the tables, etc.
 * 
 *  - How does cache work? the first time the kernel access and memory cell, it stores the address and it contents on a cache line (tag), the next time we access the
 *    same memory address we go look for it at cache first, if there isnt there then to memory, this poses a problem -> coherence-> there is hardware and attributes
 *    that ensure coherence. Maintenance can be needed when: non coherent DMA, changing instructions, changing memory attributes -> we won't do any of these we, because
 *    the kernel always sees the same memory and nothign moves around.
 * 
 *  - Ah yes, the descriptor:
 * 
 *          +---+--------+-----+-----+---+------------------------+---+----+----+----+----+------+----+----+
 *           | R |   SW   | UXN | PXN | R | Output address [47:12] | R | AF | SH | AP | NS | INDX | TB | VB |
 *           +---+--------+-----+-----+---+------------------------+---+----+----+----+----+------+----+----+
 *               63  58    55 54    53    52  47                    12 11  10   9  8 7  6 5    4    2 1    0
 *
 *   R    - reserve
 *   SW   - reserved for software use
 *   UXN  - unprivileged execute never
 *   PXN  - privileged execute never
 *   AF   - access flag
 *   SH   - shareable attribute
 *   AP   - access permission
 *   NS   - security bit
 *   INDX - index into MAIR register
 *   TB   - table descriptor bit
 *   VB   - validity descriptor bit
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
 *        +------+  |        PGD        |                     |       +------------------------->| Physical address |
 *        | ttbr |---->+-------------+  |           PUD       |                            |     |------------------|
 *        +------+  |  |             |  | +->+-------------+  |            PMD             |     |                  |
 *                  |  +-------------+  | |  |             |  | +->+-----------------+     |     +------------------+
 *                  +->| PUD address |----+  +-------------+  | |  |                 |     |     |                  |
 *                     +-------------+  +--->| PMD address |----+  +-----------------+     |     |                  |
 *                     |             |       +-------------+  +--->| Section address |-----+     |                  |
 *                     +-------------+       |             |       +-----------------+           |                  |
 *                                           +-------------+       |                 |           |                  |
 *                                                                 +-----------------+           |                  |
 *                                                                                               +------------------+
 * https://github.com/s-matyukevich/raspberry-pi-os/blob/master/docs/lesson06/rpi-os.md
 * https://lowenware.com/blog/osdev/aarch64-mmu-programming/
 * https://github.com/LdB-ECM/Raspberry-Pi/tree/master/10_virtualmemory
 */