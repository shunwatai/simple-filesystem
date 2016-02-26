#ifndef SYS_CALL_H_
#define SYS_CALL_H_

/* functions in open_t.c */
extern int open_t(const char *path, int flags);
extern int print_sb(struct superblock);
extern int print_inode(struct inode);
extern int split_path(char*, char**);
extern int makedir(int , struct superblock *, struct inode *, char *, int );

/* functions in read_t.c */
extern int read_t( int, int, void*, int );

/* functions in write_t.c */
extern int write_t( int, int, void*, int );

#endif /* SYS_CALL_H_ */
