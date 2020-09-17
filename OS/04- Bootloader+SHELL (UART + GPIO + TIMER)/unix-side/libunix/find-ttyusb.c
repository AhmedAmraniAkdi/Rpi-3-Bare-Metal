// engler, cs140e.
#define _GNU_SOURCE
#include <assert.h>
#include <fcntl.h>
#include <string.h>

#include "libunix.h"

#include <dirent.h>
static const char *ttyusb_prefixes[] = {
	"ttyUSB",	// linux
	"cu.SLAB_USB", // mac os
	0
};

// find the TTY-usb device (if any) by using <scandir> to search for
// a device with a prefix given by <ttyusb_prefixes> in /dev
// returns:
//  - device name.
// panic's if 0 or more than 1.
//
char *find_ttyusb(void) {
    char *p;
    int ocurrences = 0;
    // use <alphasort> in <scandir>
    // return a malloc'd name so doesn't corrupt.

    struct dirent **fileList;
    int noOfFiles;
    char* path = "/dev";

    noOfFiles = scandir(path, &fileList, NULL, alphasort);
    
    if (noOfFiles == -1) {
        panic("scandir error");
    }

    for(int i = 0; i < noOfFiles; i++){
        if(strstr(fileList[i]->d_name, ttyusb_prefixes[0]) != NULL){
            ocurrences++;
            (*p) = fileList[i]->d_name;
        }
        free(fileList[i]);
    }
    free(fileList);

    if(ocurrences != 1){
        panic("#TTY-USB != 1");
    }

    printf("%s\n", (*p));
    return p;
}
