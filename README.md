##please read the Assign1.pdf for details 

##This homework not finish yet, and I may not finish it. Also, many bugs + unreadable codes, please read the entire README.

###important notice:
1. for the open_t.c, it is very mess. Also, I DID NOT(forgot) use the parameter "flags", actually I think if flags=0 run create file func.; if flags=1 run make directory func.; if flags=2 run read file func. However, I just put those func directly in externt_cp, mkdir_t, cat_t....
2. must use absolute path on the SFS
3. For superblock, the instructions in PDF said that use "index number" to indicate the next available inode & data block. But I used the "offset"
4. only used one direct data block, 2nd direct blk and the indirect blk are not implemented yet, so DO NOT copy file that greater than 4096bytes

####my steps to do this shit:
1. made the ```mkfs_t.c```, form the layout of HD
2. in the ```mkfs_t```, also made the "root" directory on inode#0 at the end of execution
3. worked on the ```open_t.c``` for get the inode number. Test it by found the inode# of root dir
4. worked on ```ls_t.c``` for list the entries of dir according its inode# return from ```open_t```. I followed the sample code of getdents(), type ```man 2 getdents``` in terminal.
5. worked on ```mkdir_t.c```
6. worked on ```external_cp.c``` & ```write_t.c```
7. worked on ```cat_t.c``` & ```read_t.c```


####TODO:
- [x] mkfs_t.c
- [x] open_t.c
- [x] ls_t.c
- [x] mkdir_t.c
- [x] external_cp.c
- [x] write_t.c
- [x] read_t.c
- [x] cat_t.c
- [ ] cd_t.c
- [ ] cp_t.c
- [ ] tshell.c

####quick demo:

You can just use ```gcc``` instead of ```clang``` as the compiler

```buildHD.sh``` just a shortcut for make the 110M HD and compile and execute ```mkfs_t```:

    ./buildHD.sh

compile system call functions:

    cd sfs_functions
    clang -c open_t.c -o open_t.o
    clang -c write_t.c -o write_t.o
    clang -c read_t.c -o read_t.o

compile user commands:

    cd sfs_user_commands
    clang ../sfs_functions/{open_t.o,write_t.o} ls_t.c -o ls_t
    clang ../sfs_functions/{open_t.o,write_t.o} mkdir_t.c -o mkdir_t
    clang ../sfs_functions/{open_t.o,write_t.o} external_cp.c -o external_cp
    clang ../sfs_functions/{open_t.o,read_t.o} external_cp.c -o external_cp

Now try to ls, "./ls_t /":

    ./ls_t /
    inode#  type    size            name            create on
    =========================================================
    #0         0      32               .            Thu Feb 18     01:00:06 2016
    #0         0      32              ..            Thu Feb 18     01:00:06 2016

make a directory:

    ./mkdir_t /test_dir
    entry[0]: test_dir
    path splited: 1
    new dir test_dir wrote on inode#1.

ls again:

    ./ls_t
    inode#  type    size            name            create on
    =========================================================
    #0         0      48               .            Thu Feb 18 01:00:06 2016
    #0         0      48              ..            Thu Feb 18 01:00:06 2016
    #1         0      32        test_dir            Thu Feb 18 01:09:34 2016

try external_cp:

    ./external_cp hello.txt /
    ./ls_t
    inode#  type    size            name            create on
    =========================================================
    #0         0      59               .            Fri Feb 19 10:09:40 2016
    #0         0      59              ..            Fri Feb 19 10:09:40 2016
    #1         0      32        test_dir            Fri Feb 19 10:12:17 2016
    #2         1      11       hello.txt            Fri Feb 19 10:13:36 2016

try cat_t:

    ./cat_t /hello.txt
    hello, sfs
