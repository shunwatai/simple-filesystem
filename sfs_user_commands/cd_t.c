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

int main(int argc, char *argv[]){    
    char *path = argv[1];
    if(argc!=2){
        printf("usage: cd_t <absolute_path>\n");
        return -1;
    }
    if(path[0]!='/'){
        printf("please use absolute path\n");
        return -1;
    }
           
}
