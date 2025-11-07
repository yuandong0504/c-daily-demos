#include "pred_filter.h"
#include <stdio.h>
int main(void){
	File f[]={{1,512,"sys.log"},
		{0,1024,"test.txt"},
		{1,800,"log.log"}};
	int t=1;size_t z=600;char *name="log";
	pred pds[]={{is_type,&t},
		{size_gt,&z},
		{has_name,name}};
	for(int i=0;i<3;i++){
		if(all_of(&f[i],pds,3)){
			printf("%s\n",f[i].name);
		}
	}
	return 0;
}
