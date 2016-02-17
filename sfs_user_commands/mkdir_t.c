#include <stdio.h>     // for printf
#include <time.h>      // for time_t
#include <sys/types.h> // for open/read/write
#include <sys/stat.h>  // for open/read/write
#include <fcntl.h>     // for open/read/write
#include <unistd.h>    // for lseek
#include <stdlib.h>    // for malloc
#include <string.h>    // for strlen

#include "../sfs.h"    // SFS structures provided by ar sir
#include "../sfs_functions/sys_call.h" // header file that included this open_t for user commands

/*
 # overall tasks that mkdir performs:
 1. get nx ava inode & data blk from superblock
 2. edit inode info. and update it
 3. write "." & ".." entries to data blk
 4. get the current dir inode#
 5. increase its file_num by 1 and then update it
 6. edit nx ava inode & blk and then update it
 */

static int makedir(int fd, struct superblock *sb, struct inode *inodes, char *dir_name, int parent_inode){
    ssize_t ret = 0;

    /* inode editing */
    //printf("check point 1: new inode initialise\n================\n");
    time_t now = time(0); // get time
    inodes->i_mtime = now; // set create time
    inodes->i_type = 0;    // 0 mean dir, 1 mean file
    inodes->i_size = sizeof(DIR_NODE)*2; // initially, an empty dir has 2 entries . ..
    inodes->i_blocks = 1;  // required blocks
    inodes->direct_blk[0] = sb->next_available_blk; // the offset of data blk
    inodes->indirect_blk = -1;  // no indirect blk
    inodes->file_num = 2;  // num of files, initial with "." ".."
    //print_inode(*inodes);

    /* data blk editing */
    //printf("check point 2: write .->%d ..->%d to data blk\n================\n", inodes->i_number,parent_inode);
    DIR_NODE curdir = {".", inodes->i_number}; //name + inode#
    DIR_NODE parentdir = {"..", parent_inode}; //name + parent inode#
    lseek(fd, inodes->direct_blk[0], SEEK_SET); // goto the offset of that data blk
    ret = write(fd, &curdir, sizeof(DIR_NODE));
    ret += write(fd, &parentdir, sizeof(DIR_NODE));
    if(ret != sizeof(DIR_NODE)*2){
        perror("failed to write data blk");
        return -1;
    }

    /* update the info. of inode */
    //printf("check point 3: write back new inode\n================\n");
    lseek(fd, sb->next_available_inode, SEEK_SET); // goto offset of inode that will be updated
    ret = write(fd, inodes, BLOCK_SIZE); // write new dir info into new inode
    if(ret!=BLOCK_SIZE){ // if not written 4096 bytes, error
        perror("failed write inodeled to write inode");
        return -1;
    }
    /* update the info. of superblk */
    //printf("check point 4: update superblk\n================\n");
    sb->next_available_inode = sb->next_available_inode+BLOCK_SIZE; // nx available inode offset increased 4096
    sb->next_available_blk = sb->next_available_blk+BLOCK_SIZE; // nx available datablk offset increased 4096
    lseek(fd, SB_OFFSET, SEEK_SET); // goto offset of superblk
    ret = write(fd, sb, INODE_OFFSET-SB_OFFSET); // update superblk
    if(ret!=INODE_OFFSET-SB_OFFSET){
        perror("failed to update superblk");
        return -1;
    }
    printf("new dir %s wrote on inode#%d. \n-----\n", dir_name, inodes->i_number);

    /* update inode of parent dir(add the dir name) */
    //printf("check point 5: add new entry to parent dir\n================\n");
    DIR_NODE newdir; // entry for new dir
    strncpy(newdir.dir, dir_name, sizeof(newdir.dir)); // new dir name
    newdir.inode_number=inodes->i_number; // new dir inode number

    struct inode pinode; // for read the parent inode.
    lseek(fd, INODE_OFFSET+(parent_inode*INODE_OFFSET), SEEK_SET); // goto offset of parent inode
    ret = read(fd, &pinode, sizeof(struct inode)); // read parent inode info. into pinode
    if(ret != sizeof(struct inode)){
        perror("failed to read inode of parent dir");
        return -1;
    }

    pinode.file_num = pinode.file_num + 1; // new dir created, so parent file_num(entry) +1
    /* seek to data blk and write the new dir entry */
    lseek(fd, pinode.direct_blk[0]+pinode.i_size, SEEK_SET); // seek to the offset of writable data blk
    ret = write(fd, &newdir, sizeof(DIR_NODE)); // write the new dir entry to the data blk
    if(ret!=sizeof(DIR_NODE)){
        perror("failed to write new dir entry into parent dir's datablk");
        return -1;
    }
    pinode.i_size = pinode.i_size + sizeof(DIR_NODE); // after write success, increase size
    //print_inode(pinode);
    lseek(fd, INODE_OFFSET+(parent_inode*INODE_OFFSET), SEEK_SET); // go to parent inode again for update
    ret = write(fd, &pinode, BLOCK_SIZE); // update parent inode info.
    if(ret!=BLOCK_SIZE){
        perror("failed to update parent inode");
    }

    return 0;
}

int main(int argc, char *argv[]){
    if(argc==1){
        printf("usage: mkdir_t <dir_name>\n");
        return -1;
    }
    if(strncmp(argv[1],"/",sizeof(int)) == 0){
        printf("'/' root dir already existing, cannot mkdir\n");
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
    char buf[1024] = "";  // for concat the splited path for get parent inode#. Assume a file/dir name < 1024bytes
    int parent_inode = 0; // get the parent inode number
    int count_split = 0;  // count how many part of the path being splited
    char *entry_name[MAX_NESTING_DIR]; // separate the path by "/" and save, most dir nest 10.

    count_split = split_path(dir_name+1, entry_name) + 1; // split the path into pieces. this func. from open.c
    /* for testing, print all the entries */
    for(int i=0; i<MAX_NESTING_DIR; i++){
        printf("entry[%d]: %s\n",i,entry_name[i]);
        if(!*entry_name[i+1]){ // if empty, break
            break;
        }
    }
    printf("path splited: %d\n",count_split);

    /* the else part for get the path of parent dir */
    if(count_split==1){ // root dir, only 1 "/" detected
        parent_inode = 0; // 1st inode offset is root dir
        strncpy(dir_name, entry_name[count_split-1], strlen(dir_name));
        //strncpy(buf,"/", sizeof(char)); // abs. path begin with "/"
    } else { // trim out the last entry(new dir), concat the splited entries back to string
        strncpy(buf,"/", sizeof(char)); // abs. path begin with "/"
        for(int i=0; i<count_split; i++){
            strncat(buf, entry_name[i], sizeof(entry_name[i])); // start concat the entry_name
            if(i+1 == count_split-1){ // if i+1 is the last of entry_name[](new dir), break
                dir_name = entry_name[count_split-1];
                break;
            }
            strncat(buf,"/", sizeof(char));
        }
        printf("parent_dir: %s\n",buf); // print out the concated parent path
        printf("dir_name: %s\n",dir_name); // name of new dir that will be created

        /* after got the parent path, now use open_t to get the inode# for ".." */
        parent_inode = open_t(buf+1, 2); // buf+1 for ignore 1st "/", flag is 2 for existing dir
    }
    //printf("p_n: %d\n",parent_inode);

    makedir(fd, &sb, &inodes, dir_name, parent_inode); // create the dir by makedir()

    return 0;
}
