// https://www.raspberrypi.org/forums/viewtopic.php?f=72&t=266107&p=1618106&hilit=CNTFRQ#p1618106
// https://developer.arm.com/architectures/learn-the-architecture/generic-timer/single-page
// writing to the timer compare value clears the irq flag
// we good to go
.globl SET_CORE_TIMER
SET_CORE_TIMER:
	mrs x1, CNTPCT_EL0    // Read the EL1 physical timer
	add x1, x1, x0		  // Add nextCount timer count
	msr CNTP_CVAL_EL0, x1 // Update the EL1 physical timer compare value
	ret						

.globl ENABLE_CORE_TIMER
ENABLE_CORE_TIMER:
	mov x0, #1
	msr CNTP_CTL_EL0, x0  // Enable EL1 physical timer
	ret						 
