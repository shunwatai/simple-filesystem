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

int main(int argc, char *argv[]){
    if(argc!=2){
        printf("usage: cat_t <file>\n");
        return -1;
    }

    /* get the inode number of the input path */
    char *path = argv[1];
    if(path[0]!='/'){
        printf("please use absolute path\n");
        return -1;
    }
    
    int inode_num = open_t(path, 2); // get the inode number
    //printf("inode#%d will be read for disply the file content\n",inode_num);
    
    /* open HD */
    int fd = open("../HD",O_RDONLY);
    
    /* read out that inode info and pass those info to read_t */
    ssize_t ret=0; // get bytes from read/write 
    struct inode inodes = {};
    int inode_offset = INODE_OFFSET+sizeof(struct inode)*inode_num; // get the offset of that inode number
    
    lseek(fd, inode_offset, SEEK_SET); // goto offset of that inode
    ret = read(fd, &inodes, sizeof(struct inode)); // read the inode to var inodes
    
    int data_offset = inodes.direct_blk[0]; // get the datablk offset of real data
    int file_size = inodes.i_size; // file size in bytes 
    //printf("inodes.i_size: %d\n", inodes.i_size);
    //char buf[file_size]; // buffer for store the content    
    char *buf = malloc(file_size+1); // buffer for store the content   

    ret = read_t( inode_num, data_offset, buf, file_size);
    if(ret!=file_size){
        perror("failed to read the file...");
        return -1;
    }
    
    /* the goal of cat_t. after read_t, print out the file content */
    for(int i=0; i<file_size; i++){ // print char by char
        printf("%c",buf[i]);
    }
    
    close(fd);
    return 0;
}
