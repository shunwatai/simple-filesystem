#please read the Assign1.pdf for details 

##This homework not finish yet, and I may not finish it. Also, many bugs + unreadable codes

###All the mkdir_t, external_cp etc. are only used one direct data block only, 2nd direct blk and the indirect blk are not implementd to use yet, so the file should not greater than 4096bytes

####t_shell NOT develop yet

run the buildHD.sh to make the 110M fake "harddisk" and form the layout of
superblock, inode table, datablocks

compile the files play with them:

    ./buildHD.sh

compile system call functions

    cd sfs_functions
    clang -c open_t.c -o open_t.o
    clang -c write_t.c -o write_t.o

compile user commands

    cd sfs_user_commands
    clang ../sfs_functions/{open_t.o,write_t.o} ls_t.c -o ls_t
    clang ../sfs_functions/{open_t.o,write_t.o} mkdir_t.c -o mkdir_t
    clang ../sfs_functions/{open_t.o,write_t.o} external_cp.c -o external_cp

Now try to ls, "./ls_t /"

    ./ls_t /
    inode#  type    size            name            create on
    =========================================================
    #0         0      32               .            Thu Feb 18     01:00:06 2016
    #0         0      32              ..            Thu Feb 18     01:00:06 2016

make a directory

    ./mkdir_t /test_dir
      entry[0]: test_dir
      path splited: 1
      new dir test_dir wrote on inode#1.

ls again

    ./ls_t
    inode#  type    size            name            create on
    =========================================================
    #0         0      48               .            Thu Feb 18 01:00:06 2016
    #0         0      48              ..            Thu Feb 18 01:00:06 2016
    #1         0      32        test_dir            Thu Feb 18 01:09:34 2016
