#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(void){
	struct stat st;
	if(stat("hello.c",&st)==-1){
		perror("stat");
		return 1;
	}
	printf("File:hello.c\n");
	printf("Size:%ld bytes\n",st.st_size);
	printf("Mode:%o\n",st.st_mode);
	printf("UID:%d,GID:%d\n",st.st_uid,st.st_gid);
	printf("Last modified:%ld\n",st.st_mtime);
}
