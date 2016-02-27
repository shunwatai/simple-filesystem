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
int get_inode(int , struct inode , char **, int);
int makedir(int, struct superblock *, struct inode *, char *, int );


/* the open_t function will open the HD and read out all the sb info.
 * and the first inode info. for the entries of root dir.  */
int open_t(const char *path, int flags){ // path start at root dir & flags
    //printf("path: %s\n", path);
    ssize_t ret; // get size of bytes
    int fd; // for open that 110M HDD
    fd = open("../HD", O_RDWR); // open HDD read/write
    if(fd==-1){
        perror("fail to open: ");
        return -1;
    }

    /* read boot sector (useless) */
    char boot[512]; // useless boot sector
    ret = read(fd, &boot, sizeof(boot)); // read it out
    if( ret!=sizeof(boot) ){
        perror("read failed");
        return -1;
    } else {
        //printf("boot: %s\n---\n", boot); // print useless boot message
    }

    /* read superblock */
    //lseek(fd, 512, SEEK_SET); // go to 512byte(start of sb region)
    struct superblock sb = {}; // superblock
    ret = read(fd, &sb, sizeof(sb)); // read superblock info.
    if(ret!=sizeof(sb)){
        perror("could not read sb info");
        return -1;
    }

    // read 1st inode for root dir
    lseek(fd, INODE_OFFSET, SEEK_SET); // go to 4096byte(start of inodes region)
    struct inode inodes = {}; // inodes
    read(fd, &inodes, sizeof(inodes));

/*-------------------mkdir------------------------*/
    if(flags==1){
        char *dir_name = calloc(sizeof(path),sizeof(char*));        
        memcpy(dir_name,path,strlen(path)); // get the directory name that will be created
        //int fd; // for open HD
        ssize_t ret = 0; // get read/write bytes
        //fd = open("../HD", O_RDWR); // open HD for read & write

        /* read superblk info */
        struct superblock sb={}; // store the superblk info
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
        //printf("before mkdir\n");
        //print_sb(sb); // print superblk info

        /* read new inode info that to be written */
        struct inode inodes={};
        lseek(fd, nx_ava_inode, SEEK_SET); // go to the offset of that inode
        ret = read(fd, &inodes, sizeof(struct inode)); // read available inode to inodes
        if(ret==-1){ // if read failed
            perror("failed to read sb region");
            return -1;
        }
        //print_inode(inodes);

        /* get the parent dir inode# */
        char buf[1024] = "";  // for concat the splited path for get parent inode#. Assume the full abs. path < 1024characters
        int parent_inode = 0; // get the parent inode number
        int count_split = 0;  // count how many part of the path being splited
        char *entry_name[MAX_NESTING_DIR]; // separate the path by "/" and save, most dir nest 10.

        count_split = split_path((char*)dir_name+1, entry_name) + 1; // split the path into pieces. this func. from open_t.c
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
                    dir_name = entry_name[count_split-1]; // the last element of array is the new dir that will be created
                    break;
                }
                strncat(buf,"/", sizeof(char));
            }
            printf("parent_dir: %s\n",buf); // print out the concated parent path
            printf("dir_name: %s\n",dir_name); // name of new dir that will be created

            /* after got the parent path, now use open_t to get the inode# for ".." */
            parent_inode = open_t(buf, 2); // buf+1 for ignore 1st "/", flag is 2 for existing dir
        }
        printf("p_n: %d\n",parent_inode);

        makedir(fd, &sb, &inodes, dir_name, parent_inode); // create the dir by makedir()
    
    //close(fd);
    }
/*------------------------------------------------*/

/*-------------------external_cp------------------*/



/*------------------------------------------------*/
    // read the abs path and split them in entry_name
    char *entry_name[MAX_NESTING_DIR]={}; // separate the path by "/" and save, most dir nest 10.
    int splits = split_path((char*)path+1, entry_name); // split the path into entry_name array,
                                                        // path+1 for ignore the first "/" since the HD not mount on "/"
    /* print out the splited abs path */
    //int i = 0;
    //while( *entry_name[i] ){ // if not empty entry
        //printf("entry[%d]: %s\n", i, entry_name[i]);
        //i = i + 1;
    //}

    /* pass the entries of dir/file for return inode number */
    int inum = get_inode(fd, inodes, entry_name, splits+1); // get the inode number
    //printf("inum: %d\n",inum); // print out the inode number after searched by get_inode()

    close(fd); // close the HD
    free(*entry_name);
    return inum;  // return the inode# for user's command ls_t etc....
    //return 0;
}

/* function for split the user input path(argv[1]) and store in entry_name. return number of split */
int split_path(char *path, char **entry_name){
    //printf("split_path is: %s\n",path);
    char buf[255] = ""; // store an entry
    int i = 0;       // for loop through the path
    int j = 0;       // for buffer index
    int k = 0;       // for entry_name index
    //int slash = '/'; // for check the "/" in path
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
                //printf("path[%d] is \\0\n",i+1);
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
int get_inode(int fd, struct inode inodes, char **entry_name, int splits){
    int inum = -1; // inode num for return
    int i=0;
    ssize_t ret=0;
    //print_inode(inodes);

    // read directory entry
    char buf[BLOCK_SIZE];  // buffer for read a 4K block(the dir file datablk)
    DIR_NODE *dir_entries; // ptr point to address of entry
    //if(inodes.i_size/4096<=1) // <=1 1blk, >1 use indirect blk
    lseek(fd, inodes.direct_blk[0], SEEK_SET); // go to that data block by offset, read the real data
    ret = read(fd, &buf, sizeof(buf)); // read 4k data blk to buf, also get the 4k blk size to ret

    int entry = 0; // 1st entry start at 0
    while(entry < ret){ // while entry less than read buffer(4K data blk)
        dir_entries = (DIR_NODE*)(buf+entry); // buf + offset(entry), cast as DIR NODE

        if(!*dir_entries->dir){ // check if the entry is empty
            if(splits != i){    // i != total splits means sub-dir not exsit
                inum=-1;
            }
            break;
        }

        //printf("%4s @inode#%d\n",dir_entries->dir,dir_entries->inode_number); // print all entries in dir, name + inode#
        //printf("entry_name[%d]: %s  <=> dir_entries: %s    inode#: %d\n",i,entry_name[i],dir_entries->dir,inum); // see if entry_name exist in dir_entries
        /* on9 inhuman logic created by me. originally wanted to search dir recusively */
        if( strncmp(entry_name[i],dir_entries->dir,sizeof(dir_entries->dir)) == 0 ){ // if match name, update inode
            inum = dir_entries->inode_number; // update the inode num to that entry
            struct inode subdir_inode={};
            lseek(fd,INODE_OFFSET+inum*sizeof(struct inode),SEEK_SET); // go to the offset of the sub-directory inode
            read(fd,&subdir_inode,sizeof(struct inode)); // read the inode info. to subdir_inode
            lseek(fd, subdir_inode.direct_blk[0], SEEK_SET); //seek to the data blk
            read(fd, &buf, sizeof(buf)); // read the new directory file entries into buf
            entry = 0; // reset entry to 0 for search in new dir
            i = i + 1;
        }

        entry = entry + sizeof(DIR_NODE); // increase offset(entry)
        //printf("splits: %d   i: %d\n",splits,i);
    }
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
    printf("inode no.: %d\ninode_size: %d\ninode_blocks:%d\ninode_type: %d\ninode_direct_blk: %d\ntime: %s",
            inodes.i_number, inodes.i_size, inodes.i_blocks, inodes.i_type, inodes.direct_blk[0], ctime(&inodes.i_mtime));
    printf("------------------\n");
    return 0;
}


int makedir(int fd, struct superblock *sb, struct inode *inodes, char *dir_name, int parent_inode){
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
    DIR_NODE curdir = {}; //name + inode#
    strncpy(curdir.dir, ".", sizeof(curdir.dir));
    curdir.inode_number = inodes->i_number;
    
    DIR_NODE parentdir = {}; //name + parent inode#
    strncpy(parentdir.dir, "..", sizeof(parentdir.dir));
    parentdir.inode_number = parent_inode;
    
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
    ret = write(fd, inodes, sizeof(struct inode)); // write new dir info into new inode
    if(ret!=sizeof(struct inode)){ // if not written sizeof inode, error
        perror("failed write inodeled to write inode");
        return -1;
    }
    
    /* update the info. of superblk */
    //printf("check point 4: update superblk\n================\n");
    sb->next_available_inode = sb->next_available_inode+sizeof(struct inode); // nx available inode offset increased 4096
    sb->next_available_blk = sb->next_available_blk+BLOCK_SIZE; // nx available datablk offset increased 4096
    lseek(fd, SB_OFFSET, SEEK_SET); // goto offset of superblk
    ret = write(fd, sb, sizeof(struct superblock)); // update superblk
    if(ret!=sizeof(struct superblock)){
        perror("failed to update superblk");
        return -1;
    }
    printf("new dir %s wrote on inode#%d. \n-----\n", dir_name, inodes->i_number);

    /* update inode of parent dir(add the dir name) */
    //printf("check point 5: add new entry to parent dir\n================\n");
    DIR_NODE newdir = {}; // entry for new dir
    strncpy(newdir.dir, dir_name, sizeof(newdir.dir)); // new dir name
    newdir.inode_number=inodes->i_number; // new dir inode number

    struct inode pinode={}; // for read the parent inode.
    lseek(fd, INODE_OFFSET+parent_inode*sizeof(struct inode), SEEK_SET); // goto offset of parent inode
    ret = read(fd, &pinode, sizeof(struct inode)); // read parent inode info. into pinode
    if(ret != sizeof(struct inode)){
        perror("failed to read inode of parent dir");
        return -1;
    }

    /* seek to data blk and write the new dir entry */
    lseek(fd, pinode.direct_blk[0]+pinode.file_num*sizeof(DIR_NODE), SEEK_SET); // seek to the offset of writable data blk
    ret = write(fd, &newdir, sizeof(DIR_NODE)); // write the new dir entry to the data blk
    if(ret!=sizeof(DIR_NODE)){
        perror("failed to write new dir entry into parent dir's datablk");
        return -1;
    }
    
    pinode.file_num = pinode.file_num + 1; // new dir created, so parent file_num(entry) +1
    pinode.i_size = pinode.i_size + sizeof(DIR_NODE); // after write success, increase size   
    //print_inode(pinode); // print out updated info of parent inode
    lseek(fd, INODE_OFFSET+(parent_inode*INODE_OFFSET), SEEK_SET); // go to parent inode again for update
    ret = write(fd, &pinode, sizeof(struct inode)); // update parent inode info.
    if(ret!=sizeof(struct inode)){
        perror("failed to update parent inode");
    }

    return 0;
}

/*
int main(int argc, char *argv[]){
    open_t( argv[1], atoi(argv[2]) ); // path + flags

    //close(fd);
    return 0;
}
*/
