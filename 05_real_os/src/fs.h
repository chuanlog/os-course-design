#ifndef FS_H
#define FS_H

void fs_init(void);
void fs_format(void);
void fs_list(void);
void fs_write_file(const char* name, const char* content);
void fs_read_file(const char* name);

/* Directory Support */
void fs_mkdir(const char* name);
void fs_cd(const char* name);
void fs_pwd(void);
void fs_get_cwd(char* buf);

#endif