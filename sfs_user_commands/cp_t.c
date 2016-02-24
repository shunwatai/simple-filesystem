#include <stdio.h>     // for printf
#include <time.h>      // for time_t
#include <sys/types.h> // for open/read/write
#include <sys/stat.h>  // for open/read/write
#include <fcntl.h>     // for open/read/write
#include <unistd.h>    // for lseek
#include <string.h>    // for strlen
#include <stdlib.h>    // for malloc

#include "../sfs.h"    // SFS structures provided by ar sir
#include "../sfs_functions/sys_call.h" // include self implement open_t, read_t, write_t

/* 1. get inode# of argv[1]
 * 2. read its size and the data blk to a buf
 * 3. get nx ava inode & datablk from superblk
 * 4. write the buf to them ^ by write_t()
 * 5. add this new entry to the current directory
 */

int main(int argc, char *argv[]){
    if(argc != 3){ // check arguments
        printf("usage: cp_t <src> <des>\n");
        return -1;        
    }
    
    char *dpath = argv[2]; // store the destination path to car dpath
    if( dpath[0]!='/' ){ // if not start with / abs. path
        printf("please use absolute path on the SFS!!\n");
        return -1;
    }
    
    /* open HD for read/write */
    ssize_t ret = 0; // just for get the bytes of read/write
    int fd = open("../HD", O_RDWR);
    
    /* step 1: get the inode# of the src file */
    char *src = argv[1]; // get the src from user input    
    int src_inode_num = open_t(src, 2); // get the inode# by open_t, flag 2 means existing file but it is useless on my implementation
    printf("src inode# is %d\n", src_inode_num);
    
    /* step 2: get the size and read its data to a buf */
    ssize_t src_size = 0;    
    struct inode src_inode={};
    
    lseek(fd, INODE_OFFSET+src_inode_num*sizeof(struct inode), SEEK_SET); // goto the src inode offset
    ret = read(fd, &src_inode, sizeof(struct inode)); // read the inode to src_inode
    
    src_size = src_inode.i_size;   // get the file byte size
    char buf[src_size];  // make a buffer for the src file content
    
    lseek(fd, src_inode.direct_blk[0], SEEK_SET); // goto the datablk offset for read the real data to buf
    ret = read(fd, &buf, src_size);
    //printf("buf: %s",buf);
    
    
    
    close(fd);
    return 0;
}
