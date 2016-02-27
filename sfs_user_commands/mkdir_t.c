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



int main(int argc, char *argv[]){
    if(argc==1){
        printf("usage: mkdir_t <dir_name>\n");
        return -1;
    }
    char *path = argv[1];
    if(strncmp(&path[0],"/",sizeof(int)) == 0){
        printf("'/' root dir already existing, cannot mkdir\n");
        return -1;
    }
    if(path[0]!='/'){
        printf("please use absolute path\n");
        return -1;
    }

    open_t(path,1);
    
    return 0;
}
