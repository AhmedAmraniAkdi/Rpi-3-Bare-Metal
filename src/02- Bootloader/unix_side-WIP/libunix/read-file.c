#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libunix.h"

// read entire file into buffer.  return it.   zero pads to a 
// multiple of 4.
//
// make sure to cleanup!
uint8_t *read_file(unsigned *size, const char *name) {

    struct stat* stats = (struct stat*)malloc(sizeof(struct stat));
    stat(name, stats);
    
    *size = roundup(stats->st_size, 4);
    uint8_t *buf = (uint8_t*)calloc(*size, sizeof(uint8_t));

    int fd = 0;
    fd = open(name, O_RDONLY);
    if( fd == -1)
        panic("problem opening file");

    if(read(fd, buf, stats->st_size) == -1)
        panic("problem reading file into buffer");
    
    close(fd);
    free(stats);

    return buf;
}
