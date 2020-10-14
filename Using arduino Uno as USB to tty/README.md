The Arduino board has a built in USB to tty that works on 5V, we need 3.3V.

First connect the reset to ground, this will disable the microcontroller.

Normally we connect the Tx pin of one device to the Rx of the other and vice versa, but not this time, since the Tx of the arduino ( the one on the board ) is connected to the Rx of the USB to tty ( on the board ), which means we just connect the 
Tx of Rpi to the Tx of Arduino, and the Rx to the Rx. This way we have Rxrpi connected to Txusbtty and viceversa. It's easier understood seeing the schematic of the Arduino board.

The gpios on the Rpi work on 3.3V, THE Arduino on 5V, we use a Voltage devider using 1Kohm and 2Kohm resistors ( as in the circuit), this way we have 5 * 2/3 = 3.3 V for the Rx line of the Rpi, however we connect the Tx line of the Rpi directly, since Arduino works perfectly with 3.3V as logic high.

Finally, we shortcuit the Gnd lines, otherwise, funny stuff happen, you get everything you type on the serial terminal echoed back (even when the Rpi is off) and can't communicate with the Rpi.
