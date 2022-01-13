//fusermount -u /home/birsan/Desktop/rezultat
//gcc ex.c -o ex `pkg-config fuse --cflags --libs`
//./ex -f /home/birsan/Desktop/rezultat
#include <sys/types.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>


//-------------------------
#ifndef FUSE_USE_VERSION
	#define FUSE_USE_VERSION 30
#endif
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>


char users[256][256]; 
char _users[256][256]; 
char _users_pr[256][256];
int u_idx = 0;


size_t u_procs_len(char *user, char *ms)
{
	char command[32] = "ps -u "; 
	char data[1024], msg[10000] = "";
	size_t l;
	strcat(command, user);
	FILE* file = popen(command, "r");
	while (fgets(data, sizeof(data)-1, file) != NULL) {
    	strcat(msg,data);
	}
	strcpy(ms, msg);
	l = strlen(ms);
	return l;
}

void get_users()
{
	struct passwd *p;
    	while((p = getpwent())) {
        strcpy(users[u_idx], p->pw_name);
        char x[50] = "/";
        strcat(x, p->pw_name);
        strcpy(_users[u_idx], x);
        strcat(x, "/procs");
        strcpy(_users_pr[u_idx], x);
        u_idx++;
    }
}

static int fs_getattr(const char *path, struct stat *stbuf)
{
	char msg[10000] = "";
    	if(strcmp(path, "/") == 0) {
        	stbuf->st_mode = S_IFDIR | 0755; 
        	stbuf->st_nlink = 2;
        	return 0;
    	}
     	for(int i = 0; i < u_idx; i++) 
    		if(strcmp(path, _users[i]) == 0) {
        	stbuf->st_mode = S_IFDIR | 0755;
        	stbuf->st_nlink = 2;
        	return 0;
    	}
    	for(int i = 0; i < u_idx; i++) 
    		if(strcmp(path, _users_pr[i]) == 0) {
        	stbuf->st_mode = S_IFREG | 0755;
        	stbuf->st_nlink = 1;
        	
        	stbuf->st_size = u_procs_len(users[i], msg);
        	return 0;
    	}
    	
    	return -ENOENT;
}

static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{
	(void) offset;
    	(void) fi;

    	filler(buf, ".", NULL, 0);
    	filler(buf, "..", NULL, 0);
    	for(int i = 0; i < u_idx; i++) {
    	filler(buf, users[i], NULL, 0);
    	
    	}
	filler(buf, "procs", NULL, 0);
    	return 0;
}

static int fs_open(const char *path, struct fuse_file_info *fi)
{
    	return 0;
}

static int fs_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
    	(void) fi;
    	int ok = 0;
    	char user[20];
    	for(int i = 0; i < u_idx; i++) 
    	{
    		if(strcmp(path, _users_pr[i]) == 0)
    			{
    				ok = 1;
    				strcpy(user, users[i]);
    			}
    	}
    	if(ok == 0) 
        	return -ENOENT;

    	char msg[10000] = "";
    	char command[50] = "ps -u "; 
    	char data[1024];
    	size_t len = 0;
    	len = u_procs_len(user, msg);
    	if(offset >= len)
    		return 0;
    	if(offset + size > len){
    		memcpy(buf, msg + offset, len - offset);
    		return len - offset;
    	}
    	memcpy(buf, msg + offset, size);
    	return size;

}

static struct fuse_operations fs_oper = {
    .getattr	= fs_getattr,
    .readdir	= fs_readdir,
    .open	= fs_open,
    .read	= fs_read,
};

int main(int argc, char *argv[])
{
	get_users();
    	return fuse_main(argc, argv, &fs_oper, NULL);
}




