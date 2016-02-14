#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../sfs.h"    // SFS structures provided by ar sir
#include "sys_call.h"  // header file that included this open_t for user commands

// forward declaration
int print_inode(struct inode);
int split_path(char *, char **);
int get_inode(int , struct inode , char **);

/* the open_t functino will open the HD and read out all the sb info.
 * and the first inode info. for the entries of root dir.  */
int open_t(const char *path, int flags){ // path start at root dir & flags

    ssize_t ret; // get size of bytes
    int fd; // for open that 110M HDD
    fd = open("../HD", O_RDWR); // open HDD read/write
    if(fd==-1){
        perror("fail to open: ");
        return -1;
    }

    // read boot sector (useless)
    char boot[512]; // useless boot sector
    ret = read(fd, &boot, sizeof(boot)); // read it out
    if( ret!=sizeof(boot) ){
        perror("read failed");
        return -1;
    } else {
        //printf("boot: %s\n---\n", boot); // print useless boot message
    }

    // read superblock
    //lseek(fd, 512, SEEK_SET); // go to 512byte(start of sb region)
    struct superblock sb; // superblock
    ret = read(fd, &sb, sizeof(sb)); // read superblock info.
    if(ret!=sizeof(sb)){
        perror("could not read sb info");
        return -1;
    } else {
        // print superblock info.
        /*printf("inode offset: %d\ndata offset: %d\nmax inode: %d\nmax blk: %d\nnx_avail_inode: %d\nnx_avail_blk: %d\nblk_size: %d\n",
                sb.inode_offset,
                sb.data_offset,
                sb.max_inode,
                sb.max_data_blk,
                sb.next_available_inode,
                sb.next_available_blk,
                sb.blk_size
              );
        printf("---------------\n");*/
    }

    // read 1st inode for root dir
    lseek(fd, INODE_OFFSET, SEEK_SET); // go to 4096byte(start of inodes region)
    struct inode inodes; // inodes
    read(fd, &inodes, sizeof(inodes));
    //print_inode(inodes); // print inode info


    /* read directory entry (maybe for ls command) */
/*
    char buf[BLOCK_SIZE]; // buffer for read a 4K block
    DIR_NODE *dir_entries; // ptr point to address of entry
    if(inodes.i_size/4096<=1) // <=1 1blk, >1 use indirect blk
    lseek(fd, inodes.direct_blk[0], SEEK_SET); // go to that data block by offset, read the real data
    ret = read(fd, &buf, sizeof(buf));

    printf("inode#\ttype\tsize\tname\t\tcreate on\n"); // print useless title on top
    printf("=========================================\n");
    int entry = 0; // 1st entry start at 0
    while(entry < ret){ // while less than read buffer(4K)
        dir_entries = (DIR_NODE*)(buf+entry); // cast that buf to DIR_NODE
        if(!*dir_entries->dir){break;} // check if the entry is empty
        struct inode one_entry;
        int i_offset = dir_entries->inode_number*INODE_OFFSET; // inode offset
        lseek(fd, INODE_OFFSET+i_offset, SEEK_SET); // goto that inode offset
        read(fd, &one_entry, sizeof(struct inode)); // read that inode to one_entry
        //print_inode(one_entry);
        printf("#%d %9d %7d %7s %36s",
                dir_entries->inode_number,
                one_entry.i_type,
                one_entry.i_size,
                dir_entries->dir,
                ctime(&one_entry.i_mtime));
        entry = entry + sizeof(DIR_NODE);
    }
*/

    // read the abs path and split them in entry_name
    char *entry_name[MAX_NESTING_DIR]; // separate the path by "/" and save, most dir nest 10.
    split_path((char*)path+1, entry_name); // split the path into pieces,
                                           // path+1 for ignore the first "/" since the HD not mount on "/"
    //printf("entry[0]: %s\n",entry_name[0]);

    /* pass the entries of dir/file for return inode number */
    int inum = get_inode(fd, inodes, entry_name); // get the inode number
    //printf("inum: %d\n",inum); // print out the inode number after searched by get_inode()

    close(fd); // comment it later, leave it to read_t/write_t to close
    free(*entry_name);
    return inum;
    //return 0;
}

/* function for split the user input path(argv[1]) and store in entry_name. return number of split */
int split_path(char *path, char **entry_name){
    //printf("split_path is: %s\n",path);
    char buf[255] = ""; // store an entry
    int i = 0;       // for loop through the path
    int j = 0;       // for buffer index
    int k = 0;       // for entry_name index
    int slash = '/'; // for check the "/" in path
    int count = 0;   // count the total of split, return at the end
    
    //printf("sizeof(path): %lu\nstrlen(path): %lu\n",sizeof(path),strlen(path));
    while( i <= strlen(path) ){ // i is less than length of path
        //printf("path[%d]:%d, slash:%d \n",i,path[i],slash);               
        if( path[i] != '/' && path[i] != '\0' ){ // if "/" & '\0' NOT detected            
            buf[j] = path[i]; // copy path to buffer char by char
            //printf("pass\n");
            i = i + 1; // index of path increase
            j = j + 1; // index of buf increase            
        } else { // if "/" is detected            
            buf[j+1]='\0'; // add '\0' to the end of buf
            //printf("buf: %s\n",buf);
            entry_name[k] = malloc(sizeof(buf)); // initial space for entry_name[k]
            strncpy(entry_name[k],buf,sizeof(buf)); // copy buf to entry_name[k]
            //printf("- entry[%d]: %s\n",k,entry_name[k]);
            k = k + 1; // index of entry_name increase            
            j = 0; // reset buffer index
            strncpy(buf, "", sizeof(buf)); // empty buffer
            if(path[i]=='/'){                
                count = count + 1;
            } 
            i = i + 1; // path + 1 for passthrough the "/"
            
            if(path[i+1]=='\0'){ // if the next char of path is '\0', finish
                printf("path[%d] is \\0\n",i+1);
                break;
            }
        }
    }
    
    while(k<MAX_NESTING_DIR){ // init the rest of useless entry_name
        entry_name[k] = malloc(sizeof(buf));
        strncpy(entry_name[k],"",sizeof(buf));
        k++;
    }
    //printf("entry[i]: %s\n",entry_name[0]);
    return count;
}

/* The goal of this open_t, function for return the inode num. */
int get_inode(int fd, struct inode inodes, char **entry_name){
    int inum = -1; // inode num for return
    int i=0;
    ssize_t ret=0;
    //print_inode(inodes);

    // read directory entry
    char buf[BLOCK_SIZE]; // buffer for read a 4K block
    DIR_NODE *dir_entries; // ptr point to address of entry
    //if(inodes.i_size/4096<=1) // <=1 1blk, >1 use indirect blk
    lseek(fd, inodes.direct_blk[0], SEEK_SET); // go to that data block by offset, read the real data
    ret = read(fd, &buf, sizeof(buf)); // read 4k data blk to buf, also get the 4k blk size to ret

    int entry = 0; // 1st entry start at 0
    while(entry < ret){ // while entry less than read buffer(4K data blk)
        dir_entries = (DIR_NODE*)(buf+entry); // buf + offset(entry), cast as DIR NODE
              
        if(!*dir_entries->dir){break;} // check if the entry is empty

        if( strncmp(entry_name[i],dir_entries->dir,sizeof(dir_entries->dir)) == 0 ){ // if match name
            //printf("%4s @inode#%d\n",dir_entries->dir,dir_entries->inode_number); // print name + inode#
            inum = dir_entries->inode_number;
            /* on9 inhuman logic created by me. originally for search dir recusively */
            //if( strncmp(dir_entries->dir, ".", sizeof(dir_entries->dir)) != 0 ){ // not "." cur dir
            //    entry = 0;
            //   lseek(fd, INODE_OFFSET*(dir_entries->inode_number), SEEK_SET); // seek to new inode
            //   ret = read(fd, &buf, sizeof(buf)); // read new inode info to buf
            //}
        }

        entry = entry + sizeof(DIR_NODE); // increase offset(entry)
        i = i + 1;
    }

    // print out the splited abs path
    // i = 0;
    //while( *entry_name[i] ){ // if not empty entry
        //printf("entry[%d]: %s\n", i, entry_name[i]);
        //i = i + 1;
    //}
    return inum;
}

int print_sb(struct superblock sb){ //func. for print out sb info
    printf("-----superblk-----\n");
    printf("nx_inode: %d\nnx_blk: %d\n",sb.next_available_inode,sb.next_available_blk);
    printf("------------------\n");
    return 0;
}

/* simple function for print the inode info */
int print_inode(struct inode inodes){
    printf("-------inode------\n");
    printf("inode no.: %d\ninode_size: %d\ninode_type: %d\ninode_direct_blk: %d\ntime: %s",
            inodes.i_number, inodes.i_size, inodes.i_type, inodes.direct_blk[0], ctime(&inodes.i_mtime));
    printf("------------------\n");
    return 0;
}

/*
int main(int argc, char *argv[]){
    open_t( argv[1], atoi(argv[2]) ); // path + flags

    //close(fd);
    return 0;
}
*/
