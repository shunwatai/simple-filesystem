#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include "../sfs.h"

static int open_t(int fd){
    ssize_t ret;

    // read boot sector
    char boot[512]; // useless boot sector
    ret = read(fd, &boot, sizeof(boot)); // read it out
    if( ret!=sizeof(boot) ){
        perror("read failed");
        return -1;
    } else {
        printf("boot: %s\n", boot); // print useless boot message
    }

    // read superblock
    lseek(fd, 512, SEEK_SET); // go to 512byte(start of sb region)
    struct superblock sb; // superblock
    ret = read(fd, &sb, sizeof(sb)); // read superblock info.
    if(ret!=sizeof(sb)){
        perror("could not read sb info");
        return -1;
    } else {
        // print superblock info.
        printf("inode offset: %d\ndata offset: %d\nmax inode: %d\nmax blk: %d\nnx_avail_inode: %d\nnx_avail_blk: %d\nblk_size: %d\n",
                sb.inode_offset,
                sb.data_offset,
                sb.max_inode,
                sb.max_data_blk,
                sb.next_available_inode,
                sb.next_available_blk,
                sb.blk_size            
              );
        printf("---------------\n");
    }
    
    // read 1st inode for root dir
    lseek(fd, 4096, SEEK_SET); // go to 4096byte(start of inodes region)
    struct inode inodes; // inodes
    read(fd, &inodes, sizeof(inodes));
    
    printf("inode no.: %d\ninode_size: %d\ninode_type: %d\ninode_direct_blk: %d\n", 
            inodes.i_number, inodes.i_size, inodes.i_type, inodes.direct_blk[0]);   
    printf("---------------\n");
    
    // read directory entry
    char buf[BLOCK_SIZE]; // buffer for read a 4K block
    DIR_NODE *dir_entries; // ptr point to address of entry
    lseek(fd, inodes.direct_blk[0], SEEK_SET); // go to that data block by offset
    ret = read(fd, &buf, sizeof(buf));
    
    int entry = 0; // 1st entry start at 0
    while(entry < ret){ // while less than read buffer(4K)
        dir_entries = (DIR_NODE*)(buf+entry);
        if(!*dir_entries->dir){break;} // check if the entry is empty
        printf("%s @inode#%d\n",dir_entries->dir,dir_entries->inode_number);
        entry = entry + sizeof(DIR_NODE);        
    }
    //printf("buf: %s\n",buf);

    return 0;
}

int main(int argc, char *argv[]){
    int fd;
    fd = open(argv[1], O_RDONLY);
    if(fd==-1){
        perror("fail to open: ");
        return -1;
    }
    open_t(fd);

    close(fd);
    return 0;
}

