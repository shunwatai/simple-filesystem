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
    printf("inode#: %d\ndata_offset: %d\ncount: %d\n",inode_number,offset,count);
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
    char tmpbuf[count]; // store the read string temporary
    printf("sizeof(tmpbuf): %lu\n",sizeof(tmpbuf));
    
    int remainsize = count; // for count the remain size of file
    /* read 1st directblk */
    if(count<=BLOCK_SIZE){ // only one direct blk
        /* read to buf based on parameter "count" */   
        ret = read(fd, buf, count); // read the content to buf   
    }
    else{ // read 4k blk sin in 1st directblk
        ret = read(fd, &tmpbuf, BLOCK_SIZE); // read out the 4096 bytes of content to tmpbuf
        memcpy(buf, tmpbuf, BLOCK_SIZE);   // copy tmpbuf to buf        
        remainsize = remainsize - BLOCK_SIZE; // count the remaining size to read
        printf("1st firect blk read:\n");
        //printf("%s\n",buf);
    }
    
    /* read 2nd directblk */
    if(count>BLOCK_SIZE && remainsize<BLOCK_SIZE){
        printf("reading remaining buf: %d...\n", remainsize);
        lseek(fd, inodes.direct_blk[1], SEEK_SET); // goto 2nd direct blk
        ret += read(fd, &tmpbuf, remainsize); // read remainsize to tmpbuf
        tmpbuf[remainsize] = '\0'; // add a null char to terminate the string at the end
        //printf("%s\n",tmpbuf);
        memcpy(buf+BLOCK_SIZE, tmpbuf, remainsize); // concatinate buf with tmpbuf
    }
    if(remainsize>BLOCK_SIZE){ // remaining size still greater than 4096
        printf("reading 4k buf in 2nd direct...\n");
        lseek(fd, inodes.direct_blk[1], SEEK_SET); // goto 2nd direct blk        
        ret += read(fd, &tmpbuf, BLOCK_SIZE); // read 4096 to tmpbuf
        memcpy(buf+BLOCK_SIZE, tmpbuf, BLOCK_SIZE);
        remainsize = remainsize - BLOCK_SIZE; // count the remaining size to read
    }
    
    /* indirectblk needed */
    if(num_blocks>2){ // this inode used more than 2 blk
        int ptr_offset=0;
        for(int i=0; i<num_blocks-2; i++){
            lseek(fd, inodes.indirect_blk+i*sizeof(int), SEEK_SET); // goto indirectblk
            read(fd, &ptr_offset, sizeof(int)); // read the offset of indirectblk ptr
            lseek(fd, ptr_offset, SEEK_SET); // go to offset of that datablk
            if(remainsize<BLOCK_SIZE){ // if less than 4k,
                ret += read(fd, &tmpbuf, remainsize); // just read the remaining
                tmpbuf[remainsize] = '\0'; // add a null char to terminate the string at the end
                memcpy(buf+(BLOCK_SIZE*2+BLOCK_SIZE*i), tmpbuf, remainsize); // concat it to buf and break. BLOCK_SIZE*2 is offset of 2 directblk, BLOCK_SIZE*i is directblk offset
                break;
            }
            ret += read(fd, &tmpbuf, BLOCK_SIZE); 
            memcpy(buf+(BLOCK_SIZE*2+BLOCK_SIZE*i), tmpbuf, BLOCK_SIZE); // concat. BLOCK_SIZE*2 is offset of 2 directblk, BLOCK_SIZE*i is directblk offset
        }
    }
    
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
