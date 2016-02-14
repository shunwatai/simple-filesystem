#include <stdio.h>     // for printf
#include <time.h>      // for time_t
#include <sys/types.h> // for open/read/write
#include <sys/stat.h>  // for open/read/write
#include <fcntl.h>     // for open/read/write
#include <unistd.h>    // for lseek
#include <stdlib.h>    // for malloc
#include <string.h>    // for strlen

#include "../sfs.h"
#include "../sfs_functions/sys_call.h"

/*
 # overall tasks that mkdir performs:
 1. get nx ava inode & data blk from superblock
 2. edit inode info. and update it
 3. write "." & ".." entries to data blk
 4. get the current dir inode#
 5. increase its file_num by 1 and then update it
 6. edit nx ava inode & blk and then update it
 */

static int makedir(int fd, struct superblock *sb, struct inode *inodes, char *dir_name){
    ssize_t ret = 0;
    
    /* inode editing */    
    time_t now = time(0); // get time
    inodes->i_mtime = now; // set create time      
    inodes->i_type = 0;    // 0 mean dir, 1 mean file
    inodes->i_size = 0;    // initially, an empty dir size should be 0
    inodes->i_blocks = 1;  // required blocks
    inodes->direct_blk[0] = sb->next_available_blk; // the offset of data blk
    inodes->indirect_blk = -1;  // no indirect blk
    inodes->file_num = 2;  // num of files, initial with "." ".."
    
    /* data blk editing */   
    DIR_NODE curdir = {".", inodes->i_number}; //name + inode#
    DIR_NODE parentdir = {"..", inodes->i_number}; //name + parent inode#
    lseek(fd, inodes->direct_blk[0], SEEK_SET); // goto the offset of that data blk
    ret = write(fd, &curdir, sizeof(DIR_NODE));
    ret += write(fd, &parentdir, sizeof(DIR_NODE));
    if(ret != sizeof(DIR_NODE)*2){
        perror("failed to write data blk");
        return -1;
    }
    
    /* update the info. of superblk & inode */
    lseek(fd, sb->next_available_inode, SEEK_SET); // goto offset of inode that will be updated
    ret = write(fd, inodes, BLOCK_SIZE); 
    if(ret!=BLOCK_SIZE){ // if not written 4096 bytes, error
        perror("faiwrite inodeled to write inode");
        return -1;
    }
    
    sb->next_available_inode = sb->next_available_inode+BLOCK_SIZE;
    sb->next_available_blk = sb->next_available_blk+BLOCK_SIZE;
    lseek(fd, SB_OFFSET, SEEK_SET); // goto offset of superblk
    ret = write(fd, sb, INODE_OFFSET-SB_OFFSET);
    if(ret!=INODE_OFFSET-SB_OFFSET){
        perror("failed to update superblk");
        return -1;
    }
    
    /* add entry toparent dir */

    return 0;
}

int main(int argc, char *argv[]){
    if(argc==1){
        printf("usage: mkdir_t <dir_name>\n");
        return -1;
    }
    char *dir_name = argv[1]; // get the directory name that will be created
    int fd; // for open HD
    ssize_t ret = 0; // get read/write bytes
    fd = open("../HD", O_RDWR); // open HD for read & write
    
    /* read superblk info */
    struct superblock sb; // store the superblk info
    int nx_ava_inode=0;   // offset of next available inode
    int nx_ava_blk=0;     // offset of next available data blk
    lseek(fd, SB_OFFSET, SEEK_SET); // goto offset of superblk
    ret = read(fd, &sb, sizeof(sb)); // read superblk info to sb
    if(ret==-1){ // if read failed
        perror("failed to read sb region");
        return -1;
    }
        
    nx_ava_inode = sb.next_available_inode; // get inode offset
    nx_ava_blk = sb.next_available_blk;     // get data blk offset
    //print_sb(sb); // print superblk info
    
    /* read new inode info that to be written */    
    struct inode inodes;
    lseek(fd, nx_ava_inode, SEEK_SET); // go to the offset of that inode
    ret = read(fd, &inodes, sizeof(struct inode)); // read available inode to inodes
    if(ret==-1){ // if read failed
        perror("failed to read sb region");
        return -1;
    }    
    //print_inode(inodes);
    
    /* get the parent dir inode# */    
    char buf[1024] = ""; // for concat the splited path for get parent inode#
    int parent_inode = 0; // get the parent inode offset
    int count_split = 0; // count how many part of the path being splited
    char *entry_name[MAX_NESTING_DIR]; // separate the path by "/" and save, most dir nest 10.

    count_split = split_path(dir_name, entry_name); // split the path into pieces. this func. from open.c
    printf("path splited: %d\n",count_split);    
    
    if(count_split==1){ // root dir, only 1 "/" detected
        parent_inode = INODE_OFFSET; // 1st inode offset is root dir
    } else { // trim out the last entry(new dir), concat the splited entries back to string 
        //strncpy(buf,"/", sizeof(char)); // abs. path begin with "/"        
        for(int i=0; i<MAX_NESTING_DIR; i++){            
            strncat(buf, entry_name[i], sizeof(entry_name[i])); // start concat the entry_name
            if(!*entry_name[i+1]){ // if empty, break
                break;
            }
            strncat(buf,"/", sizeof(char)); 
        }
    }
    printf("buf: %s\n",buf);
    
    //makedir(fd, &sb, &inodes, dir_name);
    
    return 0;    
}
