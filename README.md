###last commit. 屌，唔撚做喇！ 冇時間搵mid-term
###please read the Assign1.pdf for details 
###This homework not finish yet, and I may not finish it on time. Also, many bugs + unreadable codes, please read the entire README.

###important notice!!!:
1. for the open_t.c, it is very mess. Also, I DID NOT(forgot) use the parameter "flags", actually I think if flags=0 run create file func.; if flags=1 run make directory func.; if flags=2 run read file func. However, I just put those func directly in externt_cp, mkdir_t, cat_t....
2. must use "ABSOLUTE PATH" in the SFS. Why?? Actually, this SFS not mount on any "/" directory. The prefix of "/" in SFS is fake!!! when user input the abs. path, the first char "/" will be ignored, thats why the parameter "path" in ```split_path()``` func. always +1.
3. When running tshell, ```cat_t```, ```ls_t```, ```cp_t``` are partial support relative path
4. For superblock, the instructions in PDF said that use "index number" to indicate the next available inode & data block. But I used the "offset"
5. only used the direct data block & 2nd direct blk. The indirect blk are not implemented yet, so DO NOT copy file that greater than 8192bytes
6. the ```split_path()``` func. in ```open_t.c``` is buggy. I dont know there is a ```strsep()``` in C when I doing this homework. I may use strsep() to replace my self-implement split func. if I have time.
7. File name in SFS maximum support 10characters because in ```struct dir_mapping```, its var is ```dir[10]```

####My calculation of the maximum size of a single file that can store in this SFS:
    
    each inode has 2 direct blocks & 1 indirect block.

    2 direct blocks -> 4096 x 2 = 8192bytes
    
    1 indirect block -> this block only need to store the "offset"(a number) of another data block, 
    and each "offset" is just a int, the "sizeof(int)" is 4 bytes. So this indirect block can store 4096/4=1024 offset of data blocks. 
    Since each data block is 4096bytes, this indirect block can store 1024(num of offset) x 4096(size of each datablk) = 4194304bytes
    
    The maximum size of a file is 8192bytes + 4194304bytes = 4202496bytes, which maybe around 4Mb.

####my steps to do this shit:
1. made the ```mkfs_t.c```, form the layout of HD
2. in the ```mkfs_t```, also made the "root" directory on inode#0 at the end of execution
3. worked on the ```open_t.c``` for get the inode number. Test it by found the inode# of root dir
4. worked on ```ls_t.c``` for list the entries of dir according its inode# return from ```open_t```. I followed the sample code of getdents(), type ```man 2 getdents``` in terminal.
5. worked on ```mkdir_t.c```
6. worked on ```external_cp.c``` & ```write_t.c```
7. worked on ```cat_t.c``` & ```read_t.c```
8. worked on ```cp_t.c```, just copied most of the codes from ```external_cp.c``` except the handling the source(argv[1]), just use ```open_t``` to find its inode#
9. implemented 2nd direct block & indirect block
9. made the tshell
10. hardcode the ```cd_t``` in tshell

####TODO:
- [x] mkfs_t.c
- [x] open_t.c
- [x] ls_t.c
- [x] mkdir_t.c
- [x] external_cp.c
- [x] write_t.c
- [x] read_t.c
- [x] cat_t.c
- [x] cd_t.c (hard code in tshell)
- [x] cp_t.c
- [x] tshell.c
- [x] list the nest dir -> edit ```get_inode()``` func. in open_t.c
- [x] use 2nd directblk 
- [x] use indirect blk
- [x] fix issues on ```external_cp.c``` => destination with its name, now just allow the dir name as the destination
- [ ] handle the ```flags``` parameter in```open_t.c```

###====================================================

###quick demo:
####compile all the C programs
You can just use ```gcc``` instead of ```clang``` as the compiler

```buildHD.sh``` just a shortcut for make the 110M HD and compile and execute ```mkfs_t```:

    ./buildHD.sh    

compile system call functions [details](https://stackoverflow.com/questions/2831361/how-can-i-create-c-header-files):

    cd sfs_functions
    clang -c open_t.c -o open_t.o
    clang -c write_t.c -o write_t.o
    clang -c read_t.c -o read_t.o
    cd ..

compile user commands:

    cd sfs_user_commands
    clang ../sfs_functions/open_t.o ls_t.c -o ls_t
    clang ../sfs_functions/{open_t.o,write_t.o} mkdir_t.c -o mkdir_t
    clang ../sfs_functions/{open_t.o,write_t.o} external_cp.c -o external_cp
    clang ../sfs_functions/{open_t.o,write_t.o} cp_t.c -o cp_t
    clang ../sfs_functions/{open_t.o,read_t.o} cat_t.c -o cat_t
    clang ../sfs_functions/open_t.o cd_t.c -o cd_t
    clang ../sfs_functions/open_t.o tshell.c -o tshell   
    
####run tshell and some basic commands
Now run ```tshell```:

    ./tshell
    tshell### [/]$
    
Try ```ls_t```:

    tshell### [/]$ ls_t
    inode#  type    size            name            create on
    =========================================================
    #0         0      32               .            Sat Feb 27 11:38:58 2016
    #0         0      32              ..            Sat Feb 27 11:38:58 2016

Try ```mkdir_t```:

    tshell### [/]$ mkdir_t /test_dir
    entry[0]: test_dir
    path splited: 1
    p_n: 0
    new dir test_dir wrote on inode#1.
    
Try to ```ls_t``` again to see the directory just created:

    tshell### [/]$ ls_t
    inode#  type    size            name            create on
    =========================================================
    #0         0      48               .            Sat Feb 27 11:38:58 2016
    #0         0      48              ..            Sat Feb 27 11:38:58 2016
    #1         0      32        test_dir            Sat Feb 27 11:41:32 2016

####Use external_cp to copy 3 files
Try ```external_cp``` a small file<4096 which use 1st direct block:

    tshell### [/]$ external_cp hello.txt /
    the external file will use inode#2
    inode of current dir: 0
    
Try ```external_cp``` a small file<8192 which use 2nd direct block:

    tshell### [/]$ external_cp dbtest.txt /      
    the external file will use inode#3
    inode of current dir: 0
    
Try ```external_cp``` a small file>8192 which use indirect block:
    
    tshell### [/]$ external_cp ch01.htm /
    the external file will use inode#4
    inode of current dir: 0

Try ```ls_t``` to see the copied files:

    tshell### [/]$ ls_t
    inode#  type    size            name            create on
    =========================================================
    #0         0   14562               .            Sat Feb 27 11:38:58 2016
    #0         0   14562              ..            Sat Feb 27 11:38:58 2016
    #1         0      32        test_dir            Sat Feb 27 11:41:32 2016
    #2         1      14       hello.txt            Sat Feb 27 11:44:07 2016
    #3         1    4625      dbtest.txt            Sat Feb 27 11:50:06 2016
    #4         1    9875        ch01.htm            Sat Feb 27 11:51:11 2016
    
####Now use cat_t to view the content:

    tshell### [/]$ cat_t hello.txt 
    hello, sfs :D
    
    tshell### [/]$ cat_t dbtest.txt       
    create table person(id int, name varchar(255));
    create table location(placename varchar(255), attrname varchar(255), attrtype varchar(255), country varchar(255));
    create table visited(id int, placename varchar(255), year year);

    insert into person values(001, 'sam');
    ...   
    
    tshell### [/]$ cat_t ch01.htm
    <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
    <html xmlns="http://www.w3.org/1999/xhtml">

    <head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
    ...
    
####Now try cp_t and cd_t:
    tshell### [/]$ cp_t hello.txt /test_dir/bye
    
    tshell### [/]$ cd_t /test_dir
    
    tshell### [/test_dir]$ ls_t
    inode#  type    size            name            create on
    =========================================================
    #1         0      46               .            Sat Feb 27 11:41:32 2016
    #0         0   14562              ..            Sat Feb 27 11:38:58 2016
    #5         1      14             bye            Sat Feb 27 12:02:44 2016

    tshell### [/test_dir]$ cat_t bye
    hello, sfs :D
