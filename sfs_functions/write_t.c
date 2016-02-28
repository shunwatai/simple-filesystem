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
    inodes.file_num = 0;             /* The number of file in directory, it is 0 if it is file*/
    
    
    int remain_size = count; // get the size that remaining for write to blks
    /* if less than 4096, use 1 blk */
    if(count<=BLOCK_SIZE){
        inodes.direct_blk[0] = offset;   
        /* write to the data blk */
        lseek(fd, inodes.direct_blk[0], SEEK_SET); // go to the data blk region    
        ret = write(fd, buf, count); // write buf to data blk
        if(ret!=count){
            perror("write failed");
            return -1;
        }
    }else{    
        /* write 4096 sin */
        inodes.direct_blk[0] = offset; // i am a on9
        lseek(fd, inodes.direct_blk[0], SEEK_SET); // go to the data blk region    
        ret = write(fd, buf, BLOCK_SIZE); // write buf to data blk
        remain_size = remain_size - BLOCK_SIZE;
        if(ret!=BLOCK_SIZE){
            perror("write failed");
            return -1;
        }        
    }

    /* if count greater than 4096 2nd direct */
    if(count>BLOCK_SIZE && remain_size<=BLOCK_SIZE){
        //("writing on 2nd directblk...\n");
        //printf("buf start at 4096: \n%s", buf+BLOCK_SIZE);
        inodes.direct_blk[1] = offset+BLOCK_SIZE; // 2nd blk is the 4096bytes next to 1st blk
        lseek(fd, inodes.direct_blk[1], SEEK_SET);   // go to the data blk region  
        ret = write(fd, buf+BLOCK_SIZE, remain_size); // write buf to data blk
        if(ret!=remain_size){
            perror("write failed");
            return -1;
        }
    }
    if(remain_size>BLOCK_SIZE){
        //("dllm\n");
        inodes.direct_blk[1] = offset+BLOCK_SIZE; // 2nd blk is the 4096bytes next to 1st blk
        lseek(fd, inodes.direct_blk[1], SEEK_SET);   // go to the data blk region  
        ret = write(fd, buf+BLOCK_SIZE, BLOCK_SIZE); // write buf to data blk
        remain_size = remain_size - BLOCK_SIZE;
        if(ret!=BLOCK_SIZE){
            perror("write failed");
            return -1;
        }
    }

    
    /* handle size greater than 8192, use indirect blk */
    if(count>BLOCK_SIZE*2){
        inodes.indirect_blk = offset+BLOCK_SIZE+BLOCK_SIZE; // indirectblk start at this offset
        
        //int remain_size = count - 8192; // get the size that remaining for write to indrtblks
        int total_indirectblk = inodes.i_blocks - 2; // get the num of total blk pointers for store in indirectblks
        
        for(int i=0; i<total_indirectblk; i++){      // loop the need of total blk pointers
            int indrtblk_offset = inodes.indirect_blk + BLOCK_SIZE*(i+1); // set the data blk pointer offset for store the real data
            lseek(fd, inodes.indirect_blk + sizeof(int) * i, SEEK_SET);   // go into indirectblk region
            write(fd, &indrtblk_offset, sizeof(int)); // write that pointer offset into indirectblk
            
            /* write the data to pointer offset region */            
            lseek(fd, indrtblk_offset, SEEK_SET);  // goto offset of indirct blk pointer
            //("remain: %d\n",remain_size);
            if(remain_size<=BLOCK_SIZE){   // if the remaining size less than 4k blk,                
                write(fd, buf+((BLOCK_SIZE*2 + i*BLOCK_SIZE)), remain_size); // just write it 
                //char *test=buf+((BLOCK_SIZE*2 + i*BLOCK_SIZE));                  
                //for(int i=0; i<remain_size; i++){
                    //printf("%c",test[i]);
                //}
                break;
            }
            write(fd, &buf+(BLOCK_SIZE*2 + i*BLOCK_SIZE), BLOCK_SIZE); // write the file content to blk
            remain_size = remain_size - BLOCK_SIZE; // decrease the remaining size to write
        }
        
    }else{
        inodes.indirect_blk = -1;    // this inode do not need to use indirect blk
    }
    //print_inode(inodes);  // see the inode result
    
    
    
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
    //print_sb(sb);
    
    sb.next_available_inode = sb.next_available_inode + sizeof(struct inode);
    if(inodes.i_blocks<3){ // no indirect blk
        sb.next_available_blk = sb.next_available_blk + BLOCK_SIZE*inodes.i_blocks; // inodes.i_blocks either 1 or 2
    }else{
        sb.next_available_blk = sb.next_available_blk + BLOCK_SIZE*inodes.i_blocks+1; // indirectblk
    }
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
