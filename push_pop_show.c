#include <stdio.h>

void recurse(int depth,int max,void *prev,long diffs[],int *idx){
	int x=depth;
	//printf("in  depth=%d,&x=%p\n",depth,(void*)&x);
	//printf("%p\n",&x);
	if(prev)
		diffs[(*idx)++]=(char*)prev-(char*)&x;
		//printf("diff=%ld\n",(char *)prev-(char *)&x);
	if(depth<max)recurse(depth+1,max,&x,diffs,idx);
	//printf("out depth=%d,&x=%p\n",depth,(void*)&x);
	//printf("%p\n",&x);
}
int main(){
	long diffs[64];
	int n=0;
	recurse(1,6,NULL,diffs,&n);
	for(int i=0;i<n;++i)
		printf("%ld\n",diffs[i]);
}
