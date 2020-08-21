Testing the board, the toolchain and that I can even start this.

For the toolchain I use aarch64-none-elf, followed these steps:

wget https://cs140e.sergio.bz/files/aarch64-none-elf-linux-x64.tar.gz

tar -xzvf aarch64-none-elf-linux-x64.tar.gz

sudo mv aarch64-none-elf /usr/local/bin

Add PATH="/usr/local/bin/aarch64-none-elf/bin:$PATH" to ~/.profile

from https://cs140e.sergio.bz/assignments/0-blinky/

For blinking the led I used this folder:

https://github.com/dwelch67/raspberrypi-three/tree/master/bplus/aarch64/blinker01

https://www.raspberrypi.org/forums/viewtopic.php?t=214367

Since following CS140e doesn't work for me, but this works.
