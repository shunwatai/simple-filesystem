#please read the Assign1.pdf for details 

##This homework not finish yet, and I may not finish it. Also, many bugs + unreadable codes

###All the mkdir_t, external_cp etc. are only used one direct data block only, 2nd direct blk and the indirect blk are not implementd to use yet, so the file should not greater than 4096bytes

####my steps to do this shit:
1. make the mkfs_t.c, form the layout of HD
2. in the mkfs_t, also make the "root" directory on inode#0 at the end of execution
3. work on the open_t.c for get the inode number. Test it by found the inode# of root dir
4. work on ls_t.c for list the entries of dir according the inode# return from open_t. I followed the sample code of getdents(), type ```man 2 getdents()``` in terminal.
5. work on mkdir_t.c
6. work on external_cp.c & write_t.c

####notice:
1. t_shell NOT develop yet
2. cp_t, cd_t, cat_t, read_t are not start to do yet
3. all the command must use absolute path to execute
4. For superblock, the instructions in PDF said that use "index number" to indicate the next available inode & data block. But I used the "offset"

####TODO:
- [ ] read_t.c
- [ ] cat_t.c
- [ ] tshell.c

run the buildHD.sh to make the 110M fake "harddisk" and form the layout of
superblock, inode table, datablocks

compile the files play with them:

    ./buildHD.sh

compile system call functions:

    cd sfs_functions
    clang -c open_t.c -o open_t.o
    clang -c write_t.c -o write_t.o

compile user commands:

    cd sfs_user_commands
    clang ../sfs_functions/{open_t.o,write_t.o} ls_t.c -o ls_t
    clang ../sfs_functions/{open_t.o,write_t.o} mkdir_t.c -o mkdir_t
    clang ../sfs_functions/{open_t.o,write_t.o} external_cp.c -o external_cp

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
