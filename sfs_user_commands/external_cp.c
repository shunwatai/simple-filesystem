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
        printf("usage: ./external_cp <(src)> <(des.)>\n");    
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
    
    char *buf=malloc(size);
    ret = read(src, buf, size);
    //printf("%c",*buf);

    //printf("sizeof(buf):%lu\n",sizeof(buf));
    //printf("strlen(buf):%lu\n",strlen(buf));
    /* write */
   
    //free(buf);
    //close(fd)
}
