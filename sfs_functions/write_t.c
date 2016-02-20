#include <stdio.h>     // for printf
#include <time.h>      // for time_t
#include <sys/types.h> // for open/read/write
#include <sys/stat.h>  // for open/read/write
#include <fcntl.h>     // for open/read/write
#include <unistd.h>    // for lseek
#include <string.h>    // for strlen

#include "../sfs.h"    // SFS structures provided by ar sir
#include "sys_call.h"  // header file that included this read_t for user commands

/* inode_number & offset are the available for write. 
 * buf & count are the file content and size in bytes to be written */
int write_t(int inode_number, int offset, void *buf, int count){      
    //printf("inode#%d\noffset: %d\nbuf: %scount: %d\n",inode_number,offset,buf,count);
    //printf("=====================\n");
    ssize_t ret=0; // get bytes of read/write
    
    /* open HD */
    int fd; // for HD
    fd = open("../HD", O_RDWR); // open the HD to fd
    if(fd == -1){
        perror("failed to open HD");
        return -1;
    }
    
    /* read out that inode info. by inode# */
    int inode_offset = INODE_OFFSET + sizeof(struct inode) * inode_number; // get offset of inode
    struct inode inodes={}; // for get inode 
    //printf("ioff: %d\n",inode_offset);
    
    lseek(fd, inode_offset, SEEK_SET); // go to offset of that inode
    //printf("fd: %d\n",fd);
    ret = read(fd, &inodes, sizeof(struct inode)); // read out inode info. to inodes
    if(ret!=sizeof(struct inode)){
        perror("failed to read inode info");
        return -1;
    }
    
    /* assign inode info. */
    time_t now = time(0);            // get time
    inodes.i_mtime = now;            /* Creation time of inode*/
    inodes.i_type = 1;               /* Regular file for 1, directory file for 0 */
    inodes.i_size = count;           /* The size of file */    
    inodes.i_blocks = count/4096+1;  /* The total numbers of data blocks    */
    inodes.direct_blk[0] = offset;   /* Two direct data block pointers    */
    inodes.indirect_blk = -1;        /* One indirect data block pointer */
    inodes.file_num = 0;             /* The number of file in directory, it is 0 if it is file*/
    //print_inode(inodes);  // see the inode result
    
    /* write to the data blk */
    lseek(fd, inodes.direct_blk[0], SEEK_SET); // go to the data blk region    
    ret = write(fd, buf, count); // write buf to data blk
    if(ret!=count){
        perror("write failed");
        return -1;
    }
    
    /* testing: read the data blk and display the message */
    //char test[count];
    //lseek(fd, inodes.direct_blk[0], SEEK_SET);
    //ret = read(fd,test,count);
        //if(ret!=count){
        //perror("read failed");
        //return -1;
    //}
    //printf("test: %s",test);
    
    /* update inode info */   
    lseek(fd, inode_offset, SEEK_SET);
    ret = write(fd, &inodes, sizeof(struct inode));
    if(ret!=sizeof(struct inode)){
        perror("failed to update inode");
        return -1;
    }
    
    /* update super blk info */ 
    struct superblock sb={};
    lseek(fd, SB_OFFSET, SEEK_SET);
    ret = read(fd, &sb, sizeof(struct superblock));
        if(ret!=sizeof(struct superblock)){
        perror("failed to read superblk");
        return -1;
    }
    
    sb.next_available_inode = sb.next_available_inode + sizeof(struct inode);
    sb.next_available_blk = sb.next_available_blk + BLOCK_SIZE;
    //print_sb(sb);
    
    lseek(fd, SB_OFFSET, SEEK_SET);
    ret = write(fd, &sb, sizeof(struct superblock));
    if(ret!=sizeof(struct superblock)){
        perror("failed to update superblk");
        return -1;
    }
    
    close(fd);    
    return count;
}
