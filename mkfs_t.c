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
    struct superblock sb; // set all the superblock variables
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
    ret = write( fd, &sb, sizeof(int)*896 ); //write the superblock region 4096-512
    printf("SB write %zdBytes\n", ret);
    if( ret != sizeof(int)*896 ){
        printf("bytes written [%d] are not equal to the default block size\n", (int)ret);
        return -1;
    }
    printf("Super block written succesfully\n");
    return 0;
}

/* create inode table */
static int create_inode_table(int fd){
	ssize_t ret=0;

    int num_inodes = MAX_INODE; // set maximum 100 inodes
    printf("total inodes: %d\n",num_inodes);
	struct inode *inodes[num_inodes]; // 

    //lseek(fd, 4096, SEEK_SET);
    for(int i=0; i<num_inodes; i++){
        inodes[i] = malloc(sizeof(struct inode));        
        inodes[i]->i_number=i;
        //ret += write(fd, inodes[i], sizeof(struct inode));
        ret += write(fd, inodes[i], sizeof(int)*1024);
        free(inodes[i]);
    }

    printf("inode wrote ret: %zd\n", ret);

	if (ret != num_inodes*sizeof(int)*1024) {
		printf
		    ("The inode store was not written properly. Retry your mkfs\n");
		return -1;
	}

	printf("inode region written succesfully\n");
	return 0;
}

/* create all the available 4K blocks */
static int create_blocks(fd){
    ssize_t ret=0;
    char *block[MAX_DATA_BLK];

    //ret = write(fd, &block, sizeof(int)*1024*3840);
    lseek(fd, 10485760, SEEK_SET);
    for(int i=0; i<MAX_DATA_BLK; i++){
        block[i] = malloc(sizeof(int)*1024);
        ret += write(fd, &block[i], sizeof(int)*1024);
        free(block[i]);
    }
    int totalblk = ret/(1024*4);
    printf("totalblk: %d,write: %zdBytes\n",totalblk,ret);

    return 0;
}

/* testing function for create a root dir on the first inode and data blk */
static int createrootdir(int fd){    
    struct superblock sb; // get the sperblock info.
    struct inode inodes; //get the inodes info.    
    
    /*
     make a var for creat datetime
     */
    
    lseek(fd, 512, SEEK_SET); // goto region of sb
    if( read(fd, &sb, sizeof(sb)) != sizeof(sb) ){ // read sp info and assign to sb
        perror("cannot read SB");
        return -1;
    }
    int ava_inode = sb.next_available_inode; // get available inode by offset
    
    lseek(fd, ava_inode, SEEK_SET); // goto available inode 
    if( read(fd, &inodes, sizeof(inodes)) != sizeof(inodes) ){ // read that inode info to inodes
        perror("cannot read inode");
        return -1;
    }    
    printf("you are now playing with inode#%d\n",inodes.i_number);    
    inodes.i_type = 0; // 0 for dir
    inodes.i_size = sb.blk_size; // size of dir, try 4096byte sin    
    inodes.i_blocks = 1; // use 1 blk sin
    inodes.direct_blk[0] = sb.next_available_blk; // available blk offset    
    inodes.file_num = 2; // "." & ".."
    lseek(fd, ava_inode, SEEK_SET); // seek back to available inode for update
    if( write(fd, &inodes, sizeof(int)*1024) != sizeof(int)*1024 ){ // update inode info.
        perror("inode update failed");
        return -1;
    }
    
    DIR_NODE curdir = {".", inodes.i_number}; // name + inode num
    DIR_NODE parentdir = {"..", inodes.i_number}; // name + inode num
    printf("sizeof(. ..): %lu %lu\n",sizeof(curdir),sizeof(parentdir));
    
    lseek(fd, inodes.direct_blk[0], SEEK_SET); // goto offset of blk to be written
    if( write(fd, &curdir, sizeof(curdir)) != sizeof(curdir) ){ // write entry
        perror("entry write failed");
        return -1;
    }
    if( write(fd, &parentdir, sizeof(parentdir)) != sizeof(parentdir) ){ // write entry
        perror("entry write failed");
        return -1;
    }
    printf("root dir created.\n");
    
    /* after all the writes of inode and blk, update their next available var */
    sb.next_available_blk = sb.next_available_blk + BLOCK_SIZE; // add 4K offset
    sb.next_available_inode = sb.next_available_inode + BLOCK_SIZE; // add 4K offset
    lseek(fd, 512, SEEK_SET); // go to sb region
    if( write(fd, &sb, sizeof(sb)) != sizeof(sb) ){ // update the superblock info
        perror("SB update failed");
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[]){
    printf("mkfs_t testing\n");
    int fd;
    ssize_t ret;
    char boot[512]="boot sector"; // useless boot region 512Bytes

    if (argc != 2) {
        printf("Usage: mkfs-simplefs <device>\n");
        return -1;
    }

    fd = open(argv[1], O_RDWR | O_TRUNC);
    printf("fd open return %d\n",fd);
    if (fd == -1) {
        perror("Error opening the device");
        return -1;
    }

    write( fd, &boot, sizeof(boot) ); // write first 512Bytes for boot
    printf("boot write at the beginning %luB\n", sizeof(boot));

    create_superblk(fd);
    create_inode_table(fd); 
    create_blocks(fd);    
    createrootdir(fd);

    close(fd);
}
