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

    //char buf[size]; // allocate memory to buf to store the string in file
    char *buf=malloc(size+1); // +1 for null char
    ret = read(src, buf, size); // read the file to buf

    /* just print out the text from file */
    printf("buf: ");
    for(int i=0; i<ret; i++){
        printf("%c ",buf[i]);
    }

    /* now read out the superblock for available inode and datablk to write */
    struct superblock sb={};
    lseek(fd, SB_OFFSET, SEEK_SET); // goto superblk region
    ret = read(fd, &sb, sizeof(struct superblock)); // read superblk info. to sb
    //print_sb(sb);

    /* get the inode number. I try to calculate the inode# on9ly
     * instead of get that inode info directly.(I initialed 0-99 inode# when run the mkfs_t) */
    int inode_num = (sb.next_available_inode-INODE_OFFSET) / sizeof(struct inode);
    printf("the external file will use inode#%d\n",inode_num);

    /* use sys_call function write_t to alocate new inode for the file and update sb info. */
    //strncpy(fname, buf, size);
    ret = write_t(inode_num, sb.next_available_blk, buf, size);
    free(buf);

    if(ret!=size){
        perror("write_t failed");
    }

    /* after write_t success, add the new entry to the dir according to the arg[2](destination) */
    char *dpath;
    if(strncmp(argv[2],"/",sizeof(int)) == 0){ // because of my on9 coding, just a alias from "/" -> "/."
        dpath = "/."; // change "/" to "/."
    }else{
        dpath = argv[2]; // assign the complete path
    }

    int dir_inode = open_t(dpath,2); // get inode number of that dir
    printf("inode of current dir: %d\n",dir_inode);

/* *****if dir_inode=-1, the last entry is the filename, else user did not specific the name of copied file, then use the src argv[1] */
    char path[1024] = "";  // if dpath containing the newfile name, trim it out and concat the splited path for get its inode# again.
    char filename[512] = ""; // maximum character of name 512

    char *entry_name[MAX_NESTING_DIR]; // separate the path by "/" and save, most dir nest 10.
    int count_split = split_path(dpath+1, entry_name) + 1; // split the path into pieces. this func. from open_t.c

    if(count_split==1){ // that file copying to root dir
        if(strncmp(dpath,"/.",strlen(dpath))!=0){ // if not just a "/", but with a specific name, then use it.
            strncpy(filename,entry_name[0],strlen(entry_name[0]));
        }
    }

    if(dir_inode == -1){ // get the filename, also search for the dir inode again
        printf("the last part of path: %s\n",entry_name[count_split-1]);
        strncpy(filename, entry_name[count_split-1], strlen(entry_name[count_split-1])); // copy last entry as filename that will be created
        strncpy(path,"/", sizeof(char)); // abs. path begin with "/"
        for(int i=0; i<count_split; i++){
            strncat(path, entry_name[i], sizeof(entry_name[i])); // start concat the entry_name
            if(i+1 == count_split-1){ // if i+1 is the last of entry_name[](new dir), break
                break;
            }
            strncat(path,"/", sizeof(char));
        }
        dir_inode = open_t(path,2); // get inode number again of that dir
    } else { // else the use did not specific the file name. just the path of dir
        strncpy(filename, argv[1], sizeof(filename));
    }

    /* add the copied file into the directory entry */
    struct inode inodes={};
    lseek(fd,INODE_OFFSET+sizeof(struct inode)*dir_inode,SEEK_SET); // seek to dir inode offset
    ret = read(fd, &inodes, sizeof(struct inode)); // read inode info to inodes
    lseek(fd, inodes.direct_blk[0]+sizeof(DIR_NODE)*inodes.file_num, SEEK_SET); // goto writable offset of datablk

    DIR_NODE entry={}; // for the new file
    strncpy(entry.dir, filename, sizeof(entry.dir)); // new file name
    entry.inode_number=inode_num; // new file inode number

    ret = write(fd, &entry, sizeof(DIR_NODE));  // write new entry into the directory

    inodes.i_size = inodes.i_size + size;
    inodes.file_num = inodes.file_num + 1;

    lseek(fd,INODE_OFFSET+sizeof(struct inode)*dir_inode,SEEK_SET);
    ret = write(fd, &inodes, sizeof(struct inode));

    //print_inode(inodes);
    //print_sb(sb);

    close(fd);
}
