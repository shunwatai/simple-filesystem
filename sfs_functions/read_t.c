#include <stdio.h>     // for printf
#include <time.h>      // for time_t
#include <sys/types.h> // for open/read/write
#include <sys/stat.h>  // for open/read/write
#include <fcntl.h>     // for open/read/write
#include <unistd.h>    // for lseek
#include <string.h>    // for strlen

#include "../sfs.h"    // SFS structures provided by ar sir
#include "sys_call.h"  // header file that included this read_t for user commands

/* i dont know how to use the "offset". I assume it is 0 sin which read at start of data blk. 
 * This function for user command cat_t */
int read_t( int inode_number, int offset, void *buf, int count){ 
    ssize_t ret = 0; // get the bytes of read
    
    /* open HD */
    int fd; // for open HD to read
    fd = open("../HD",O_RDONLY); // open ../HD for readonly
    
    /* seek to inode according to "inode_number" parameter start at INODE_OFFSET */
    int inode_offset = INODE_OFFSET + (INODE_OFFSET * inode_number);
    lseek(fd, inode_offset, SEEK_SET); // seek to inode_offset
    
    /* get inode info. inorder to get  */
    struct inode inodes;
    ret = read(fd, &inodes, sizeof(struct inode)); // read inode info to inodes
    if(ret!=sizeof(struct inode)){
        perror("failed to read inode");
        return -1;
    }
    int file_size = inodes.i_size; // get file size
    int num_blocks = inodes.i_blocks; // get number of blocks
    /* if file size greater than 4096, 2nd direct blk is need. 
     * if greater than 8192, indirect blk is needed.
     * but I dont know how to implement yet, so just read the 1st direct blk here */
    int datablk_offset = inodes.direct_blk[0]; // 1st direct datablk
    
    /* seek to data blk according to offset parameter */
    lseek(fd, datablk_offset+offset, SEEK_SET); // seek to data blk which storing real data
    
    /* read to buf based on parameter "count" */
    ret = read(fd, buf, count); // read the from the offset to buf
    
    close(fd);
    /* return 0 or -1 */
    if(ret != count){ // if read failed       
        perror("failed to read");        
        return -1;    // return -1  
    } else {
        return count; // read nothing
    }
}

//int main(){
//    
//}
