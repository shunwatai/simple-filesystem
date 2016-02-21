#include <stdio.h>     // for printf
#include <time.h>      // for time_t
#include <sys/types.h> // for open/read/write
#include <sys/stat.h>  // for open/read/write
#include <fcntl.h>     // for open/read/write
#include <unistd.h>    // for lseek
#include <string.h>    // for strlen

#include "../sfs.h"    // SFS structures provided by ar sir
#include "../sfs_functions/sys_call.h" // include self implement open_t, read_t, write_t

/* THis ls function is based on the getdents(), "man 2 getdents" to read the example */
int ls(int fd, struct inode inodes){
    ssize_t ret = 0; // return byte size of read/write
    char buf[BLOCK_SIZE]=""; // buffer for read a 4K direct block of an inode
    DIR_NODE *dir_entries; // ptr point to address of entry
    //if(inodes.i_size/4096<=1) // <=1 1blk, >1 use indirect blk
    
    lseek(fd, inodes.direct_blk[0], SEEK_SET); // go to that data block by offset, read the real data
    ret = read(fd, &buf, sizeof(buf));
    //int test = inodes.direct_blk[0];
    
    printf("inode#\ttype\tsize\t\tname\t\tcreate on\n"); // print useless title on top
    printf("=========================================================\n");
    int entry = 0; // 1st entry start at 0
    while(entry < ret){ // while less than read buffer(4K)
        //printf("accessing: %d\n",test);
        dir_entries = (DIR_NODE*)(buf+entry); // cast that buf to DIR_NODE
        if(!*dir_entries->dir){break;} // check if the entry is empty
        struct inode ls_inode; // list the entries according to that inode number
        int i_offset = dir_entries->inode_number*sizeof(struct inode); // inode offset
        
        lseek(fd, INODE_OFFSET+i_offset, SEEK_SET); // goto that inode offset
        read(fd, &ls_inode, sizeof(struct inode)); // read that inode to ls_inode
        //print_inode(ls_inode);
        printf("#%d %9d %7d %15s %36s",
                dir_entries->inode_number,
                ls_inode.i_type,
                ls_inode.i_size,
                dir_entries->dir,
                ctime(&ls_inode.i_mtime));
        entry = entry + sizeof(DIR_NODE);
        //test = test + sizeof(DIR_NODE);
    }

    return 0;
}

int main(int argc, char *argv[]){
    int inum = -1; // get the inode number from open_t()
    char *path;    // get path from user's input
    int flags = 2; // list the existing file or dir (for open_t 2nd param)

    if(argc==1){ // user did not input any specific path/file
        path = "/."; // current dir
    }
    else if(strncmp(argv[1],"/",sizeof(int)) == 0){ // because of my on9 coding, just a alias from "/" -> "/."
        path = "/."; // root dir
    }
    else {
        path = argv[1]; // get the user input path
    }

    inum = open_t(path, flags);  // use the open_t to get the inode#
    if(inum==-1){
        perror("director/file not exist");
        return -1;
    }

    //printf("inum: %d\n",inum);
    int offset = sizeof(struct inode) * inum; // sizeof(inode) * inode_number = offset

    /* after get the inode# from open_t, now list the dir entries by ls() according to that inode# */
    int fd; // for open HD
    struct inode inodes={}; // for store the inode info.
    ssize_t ret = 0; // get byte size of read/write

    fd = open("../HD", O_RDWR); // open HD
    if(fd==-1){ // check HD failed to open
        perror("could not open HD");
        return -1;
    }

    lseek( fd, INODE_OFFSET+offset, SEEK_SET ); // goto the offset of that inode
    ret = read( fd, &inodes, sizeof(struct inode) );  // read the inode info. to inodes
    if(ret != sizeof(struct inode)){
        perror("failed to read inode");
        return -1;
    }
    //print_inode(inodes);
    ls(fd,inodes); // list out the dir content
}
