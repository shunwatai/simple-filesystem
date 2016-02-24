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

/* 
 * 1. get inode# of argv[1]
 * 2. read its size and the data blk to a buf
 * 3. get nx ava inode & datablk from superblk
 * 4. write the buf to them ^ by write_t()
 * 5. add this new entry to the current directory
 */

int main(int argc, char *argv[]){
    if(argc != 3){ // check arguments
        printf("usage: cp_t <src> <des>\n");
        return -1;        
    }
    
    char *spath = argv[1]; // store the source path to var spath
    if( spath[0]!='/' ){   // if not start with / abs. path
        printf("please use absolute path on the SFS!!\n");
        return -1;
    }
    
    char *dpath = argv[2]; // store the destination path to var dpath
    if( dpath[0]!='/' ){   // if not start with / abs. path
        printf("please use absolute path on the SFS!!\n");
        return -1;
    }
    
    /* open HD for read/write */
    ssize_t ret = 0; // just for get the bytes of read/write
    int fd = open("../HD", O_RDWR);
    
    /* step 1: get the inode# of the src file */
    char *src = argv[1]; // get the src from user input    
    int src_inode_num = open_t(src, 2); // get the inode# by open_t, flag 2 means existing file but it is useless on my implementation
    printf("src inode# is %d\n", src_inode_num);
    
    /* step 2: get the size and read its data to a buf */
    ssize_t src_size = 0;    
    struct inode src_inode={};
    
    lseek(fd, INODE_OFFSET+src_inode_num*sizeof(struct inode), SEEK_SET); // goto the src inode offset
    ret = read(fd, &src_inode, sizeof(struct inode)); // read the inode to src_inode
    if(ret!=sizeof(struct inode)){
        perror("failed to read out the src inode");
        return -1;
    }
    
    src_size = src_inode.i_size;   // get the file byte size
    char buf[src_size];  // make a buffer for the src file content
    
    lseek(fd, src_inode.direct_blk[0], SEEK_SET); // goto the datablk offset for read the real data to buf
    ret = read(fd, &buf, src_size);
    if(ret!=src_size){
        perror("failed to read the file content");
        return -1;
    }
    printf("buf: %s",buf);
    
    /* step 3: get the nx ava inode & datablk from superblk */
    struct superblock sb={};
    lseek(fd, SB_OFFSET, SEEK_SET); // goto superblk offset
    
    ret = read(fd, &sb, sizeof(struct superblock)); // read the superblk to sb
    if(ret!=sizeof(struct superblock)){
        perror("failed to read superblk");
        return -1;
    }
    
    int inode_num = (sb.next_available_inode-INODE_OFFSET) / sizeof(struct inode);
    printf("new file will copy to inode#%d\n", inode_num);
    
    /* write the file content to the new inode and datablk by write_t */
    write_t(inode_num, sb.next_available_blk, buf, src_size);
    
    /* update the the directory file of the destination directory */ 
    char filename[10] = ""; // DIR_NODE.dir char[10], so name 10 characters
    char path[strlen(dpath)];
    char *entry_name[MAX_NESTING_DIR]; // for store the path in pieces
    int splits = split_path(dpath+1, entry_name) + 1; // split the path and store in entry_name
    
    int dpath_inode = open_t(dpath, 2); // find the inode# of the user specificed path
    if(dpath_inode == -1){ // the user intput the spcific file name, cannot find
        strncpy(filename,entry_name[splits-1],sizeof(filename)); // copy last part as filename that will be created
        
        strncpy(path, "/.", sizeof(path)); // copy "/." as path
        printf("path: %s, sizeof(path): %lu\n",path, sizeof(path));
        
        if(splits>1){ // if split more than 1, copy to sub dir
            strncpy(path, "/", sizeof(char)+1);
            for(int i=0; i<splits; i++){ // remake the path as string for store in path
                strncat(path, entry_name[i], strlen(entry_name[i]));
                //printf("path: %s, entry[%d]: %s\n", path, i, entry_name[i]);
                if(i+1 == splits-1){
                    break;
                }
                strncat(path, "/", sizeof(char)+1);
            }            
        }
        dpath_inode = open_t(path, 2);
        printf("path: %s, fname: %s\n",path, filename);
        printf("INdestination inode#%d\n",dpath_inode);
    }else{
        strncpy(path, dpath, strlen(dpath));
    }
    printf("destination inode#%d\n",dpath_inode);
    
    lseek(fd, INODE_OFFSET+dpath_inode*sizeof(struct inode), SEEK_SET); // goto inode offset of dir
    struct inode d_inode={}; // directory inode
    ret = read(fd, &d_inode, sizeof(struct inode)); // read inode info to d_inode
    if(ret!=sizeof(struct inode)){
        perror("failed to read directory inode");
        return -1;
    }
    
    lseek(fd, d_inode.direct_blk[0]+d_inode.file_num*sizeof(DIR_NODE), SEEK_SET);
    DIR_NODE newfile={0};
    strncpy(newfile.dir, filename, sizeof(newfile.dir));
    newfile.inode_number = inode_num;
    ret = write(fd, &newfile, sizeof(DIR_NODE));
    
    d_inode.i_size = d_inode.i_size + src_size;
    d_inode.file_num = d_inode.file_num + 1;
    
    lseek(fd, INODE_OFFSET+dpath_inode*sizeof(struct inode), SEEK_SET);
    ret = write(fd, &d_inode, sizeof(struct inode));  
    
    close(fd);
    return 0;
}
