#include <stdio.h>     // for printf
#include <time.h>      // for time_t
#include <sys/types.h> // for open/read/write
#include <sys/stat.h>  // for open/read/write
#include <fcntl.h>     // for open/read/write
#include <unistd.h>    // for lseek
#include <string.h>    // for strlen
#include <stdlib.h>

#include "../sfs.h"    // SFS structures provided by ar sir
#include "sys_call.h"  // header file that included this read_t for user commands

/* inode_number & offset for the location of the file. buf is the space for content and count is bytes size.
 * This function for user command cat_t */
int read_t( int inode_number, int offset, void *buf, int count){ 
    //printf("inode#: %d\ndata_offset: %d\nsizeof(buf): %lu\ncount: %d\n",inode_number,offset,sizeof(buf),count);
    ssize_t ret = 0; // get the bytes of read
        
    /* open HD */
    int fd; // for open HD to read
    fd = open("../HD",O_RDONLY); // open ../HD for readonly

    /* seek to inode according to "inode_number" parameter start at INODE_OFFSET */
    int inode_offset = INODE_OFFSET + sizeof(struct inode) * inode_number; // get the inode# offset
    lseek(fd, inode_offset, SEEK_SET); // seek to inode_offset (the target file to read)
    
    /* get inode info. in order to get file bytes size + data blk offset */
    struct inode inodes = {};
    ret = read(fd, &inodes, sizeof(struct inode)); // read inode info to inodes
    if(ret!=sizeof(struct inode)){
        perror("failed to read inode");
        return -1;
    }
    if(inodes.i_type!=1){ // not regular file
        printf("did you enter the dir???\n");
        return -1;
    }
    
    //int file_size = inodes.i_size; // get file size
    int num_blocks = inodes.i_blocks; // get number of blocks
    
    /* if file size greater than 4096, 2nd direct blk is needed to read. 
     * if greater than 8192, indirect blk is needed.
     * but I dont know how to implement yet, so just read the 1st direct blk here */
    //int datablk_offset = inodes.direct_blk[0]; // offset of 1st direct datablk
    
    /* seek to data blk according to offset parameter */
    lseek(fd, offset, SEEK_SET); // seek to data blk which storing real data
    
    /* read to buf based on parameter "count" */   
    ret = read(fd, buf, count); // read the content to buf    
    //printf("str: %s",buf);
    
    /* return ret */
    if(ret != count){ // if read failed       
        perror("failed to read");        
        return -1;    // return -1  
    }   
    
    close(fd);
    return ret; // num of byte that had read
    
    //return 0;
}

//int main(){
//    
//}
