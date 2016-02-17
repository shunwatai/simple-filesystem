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

int main(int argc, char* argv[]){
    if(argc != 3){
        printf("usage: ./external_cp <src> <des>\n");    
        printf("src: existing file on real file system\ndes:absolute path of SFS\n");
        return -1;
    }
    
    ssize_t ret=0; // get the bytes of read/write
    
    /* open HD */
    int fd;
    fd = open("../HD",O_RDWR); // oprn HD for read write
    
    /* read the source file */
    int src; // file descripter for src file
    struct stat st; // to get the file size later
    
    src = open(argv[1],O_RDONLY); // open the source file for read only
    fstat(src,&st); // get the status of src file?
    ssize_t size = st.st_size; // get the size    
    //printf("size of src file: %zd\n",size); 
    
    char *buf=malloc(size); // allocate memory to buf to store the string in file
    ret = read(src, buf, size); // read the file to buf
    
    /* just print out the text from file */
    printf("buf: ");
    for(int i=0; i<ret; i++){
        printf("%c",buf[i]);
    }   
        
    /* now read out the superblock for available inode and datablk to write */
    struct superblock sb;
    lseek(fd, SB_OFFSET, SEEK_SET); // goto superblk region
    ret = read(fd, &sb, sizeof(struct superblock)); // read superblk info. to sb
    print_sb(sb);
    
    /* get the inode number. I try to calculate the inode# on9ly 
     * instead of get that inode info directly.(I initialed 0-99 inode# when run the mkfs_t) */
    int inode_num = sb.next_available_inode / INODE_OFFSET - 1;
    //printf("the external file will use inode#%d\n",inode_num);
    
    /* write */    
    ret = write_t(inode_num, sb.next_available_blk, buf, strlen(buf));
    if(ret!=strlen(buf)){
        perror("write_t failed");
    }
   
    /* after write_t success, add the new entry to the dir according to the arg[2](destination) */
    char *dpath;
    if(strncmp(argv[2],"/",sizeof(int)) == 0){ // because of my on9 coding, just a alias from "/" -> "/."
        dpath = "/."; // convert "/" to "/."
    }
    int dir_inode = open_t(dpath,2); // get inode number of that dir
    //printf("inode of dir: %d\n",dir_inode);
    
    struct inode inodes;
    lseek(fd,INODE_OFFSET+(INODE_OFFSET*dir_inode),SEEK_SET); // seek to dir inode offset
    ret = read(fd, &inodes, sizeof(struct inode)); // read inode info to inodes
    
    lseek(fd, inodes.direct_blk[0]+inodes.i_size, SEEK_SET); // goto writable offset of datablk
    //char *filename = argv[1];
    DIR_NODE entry;
    strncpy(entry.dir, argv[1], sizeof(entry.dir)); // new dir name
    entry.inode_number=inode_num; // new dir inode number
    
    ret = write(fd, &entry, sizeof(DIR_NODE));
    
    inodes.i_size = inodes.i_size + strlen(buf);
    inodes.file_num = inodes.file_num + 1;
    
    
    lseek(fd,INODE_OFFSET+(INODE_OFFSET*dir_inode),SEEK_SET);
    ret = write(fd, &inodes, BLOCK_SIZE);
    
    free(buf);
    close(fd);
}
