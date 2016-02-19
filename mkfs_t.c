#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#include "sfs.h" // all the superblock, inode struct info.

/*********************************************************************/
// create superblock
static int create_superblk(int fd){ 
    struct superblock sb={}; // set all the superblock variables
    sb.inode_offset = INODE_OFFSET;
    sb.data_offset = DATA_OFFSET;
    sb.max_inode = MAX_INODE;
    sb.max_data_blk = MAX_DATA_BLK;
    sb.next_available_inode = INODE_OFFSET; // +4096(4K) after new file created
    sb.next_available_blk = DATA_OFFSET; // +4096(4K) after a block is fulled
    sb.blk_size = BLOCK_SIZE;
/*
    printf("inode_off: %d\ndata_off: %d\nmax_inode: %d\nmax_datablk: %d\nnx_inode: %d\nnx_blk: %d\nblk_size: %d\n",
            sb.inode_offset,sb.data_offset,sb.max_inode,sb.max_data_blk,sb.next_available_inode,sb.next_available_blk,sb.blk_size);
*/
    printf("sizeof(sb): %lu\n",sizeof(sb)); // sizeof(sb) = 28bytes

    ssize_t ret;
    lseek(fd, SB_OFFSET, SEEK_SET);
    ret = write( fd, &sb, sizeof(sb) ); //write the superblock region 4096-512
    printf("SB write %zdBytes\n", ret);
    if( ret != sizeof(sb) ){
        printf("bytes written [%d] are not equal to the default block size\n", (int)ret);
        return -1;
    }
    printf("Super block written succesfully\n------\n");
    return ret;
}

/* create inode table */
static int create_inode_table(int fd){
	ssize_t ret=0;

    int num_inodes = MAX_INODE; // set maximum 100 inodes
    printf("total inodes: %d\n",num_inodes);
	struct inode inodes = {0};  // initialise a inode struct

    lseek(fd, INODE_OFFSET, SEEK_SET); // seek to first inode offset 4096
    for(int i=0; i<num_inodes; i++){ 
        //struct inode inodes = {}; //        
        inodes.i_number = i;                
        ret += write(fd, &inodes, sizeof(struct inode));  
    }

    printf("size of each inode %lu x 100 return: %zdbytes\n", sizeof(struct inode), ret);

	if (ret != num_inodes*sizeof(struct inode)) {
		perror("The inode store was not written properly. Retry your mkfs");
		return -1;
	}

	printf("inode region written succesfully\n------\n");        
	return ret;
}

/* create all the available 4K blocks */
static int create_blocks(int fd){
    ssize_t ret=0;
    //char *block[MAX_DATA_BLK];
    char block[BLOCK_SIZE] = "";
    printf("write %lu blk size into disk...\n", sizeof(block));
    
    lseek(fd, DATA_OFFSET, SEEK_SET);
    for(int i=0; i<MAX_DATA_BLK; i++){
        //block[i] = malloc(BLOCK_SIZE);
        ret += write(fd, &block, BLOCK_SIZE);
        //free(block[i]);
    }
    int totalblk = ret/(1024*4);
    printf("totalblk: %d,write: %zdBytes\n------\n",totalblk,ret);

    return ret;
}

/* testing function for create a root dir on the first inode and data blk */
static int createrootdir(int fd){    
    struct superblock sb = {0}; // get the sperblock info.
    struct inode inodes = {0};  // get the inodes info.    
    ssize_t ret=0; // return size of bytes
    
    lseek(fd, SB_OFFSET, SEEK_SET); // goto region of sb
    if( read(fd, &sb, sizeof(struct superblock)) != sizeof(struct superblock) ){ // read sp info and assign to sb
        perror("cannot read SB");
        return -1;
    }
    int ava_inode = sb.next_available_inode; // get available inode by offset    
    
    lseek(fd, ava_inode, SEEK_SET); // goto available inode 
    if( (ret=read(fd, &inodes, sizeof(inodes))) != sizeof(inodes) ){ // read that inode info to inodes
        perror("cannot read inode");        
        return -1;
    }
    
    printf("creating root dir on inode#%d\n",inodes.i_number); // display inode num    
    time_t now = time(0); // get time
    inodes.i_mtime = now; // create time
    inodes.i_type = 0; // 0 for dir
    inodes.i_size = sizeof(DIR_NODE)*2; // initially, an empty dir has 2 entries . ..
    inodes.i_blocks = 1; // use 1 blk sin
    inodes.direct_blk[0] = sb.next_available_blk; // available blk offset    
    inodes.file_num = 2; // "." & ".."

    lseek(fd, -ret, SEEK_CUR); // seek back to available inode for update
    if( (ret=write(fd, &inodes, sizeof(inodes))) != sizeof(inodes) ){ // update inode info.
        perror("inode update failed");
        return -1;
    }
    
    DIR_NODE curdir = {}; // init dir struct (name + inode num)
    strncpy(curdir.dir,".",sizeof(DIR_NODE));
    curdir.inode_number = inodes.i_number;
    
    DIR_NODE parentdir = {}; // init dir struct (name + inode num)
    strncpy(parentdir.dir,"..",sizeof(DIR_NODE));
    parentdir.inode_number = inodes.i_number;
    //printf("sizeof(. ..): %lu %lu\n",sizeof(curdir),sizeof(parentdir));
    
    lseek(fd, inodes.direct_blk[0], SEEK_SET); // goto offset of blk to be written
    if( (ret=write(fd, &curdir, sizeof(curdir))) != sizeof(DIR_NODE) ){ // write dir entry
        perror("entry write failed");
        return -1;
    }
    if( (ret=write(fd, &parentdir, sizeof(parentdir))) != sizeof(DIR_NODE) ){ // write dir entry
        perror("entry write failed");
        return -1;
    }
    printf("root dir created.\n");
    
    /* after all the writes of inode and blk, update their next available var */
    sb.next_available_blk = sb.next_available_blk + BLOCK_SIZE; // increase 4K offset for data blk
    sb.next_available_inode = sb.next_available_inode + sizeof(struct inode); // increase sizeof(struct inode) offset
    lseek(fd, SB_OFFSET, SEEK_SET); // go to sb region
    if( (ret=write(fd, &sb, sizeof(sb))) != sizeof(sb) ){ // update the superblock info
        perror("SB update failed");
        return -1;
    }
    return ret;
}

int main(int argc, char *argv[]){
    printf("mkfs_t testing....\n");
    int fd;
    ssize_t ret;
    char boot[512]="booting Simple File System...\nThis assginment is initialised by Tai Shun Wa"; // useless boot region 512Bytes

    if (argc != 2) {
        printf("Usage: mkfs_t <HD>\n");
        return -1;
    }

    fd = open(argv[1], O_RDWR | O_TRUNC);
    printf("fd open return %d\n",fd);
    if (fd == -1) {
        perror("Error opening the device");
        return -1;
    }

    ret = write( fd, &boot, sizeof(boot) ); // write first 512Bytes for boot
    if(ret!=sizeof(boot)){
        perror("failed to write boot sector");
        return -1;
    }
    printf("boot write at the beginning %luB\n------\n", sizeof(boot));

    ret = create_superblk(fd);
    if(ret==-1){        
        return ret;
    }
    
    ret = create_inode_table(fd); 
    if(ret==-1){
        return ret;
    }
    
    create_blocks(fd);    
    if(ret==-1){
        return ret;
    }
    
    createrootdir(fd);

    close(fd);
}
