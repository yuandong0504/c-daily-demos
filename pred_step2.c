#include <stdio.h>
#include <stdbool.h>

typedef bool (*predicate_fn)(int v,void *ctx);
bool between(int v,void *ctx){
	int *a=(int *)ctx;
	return((v>=a[0])&&(v<=a[1]));
}
void filter_print(int *a,int n,predicate_fn pred,int *b){
	for(int i=0;i<n;i++){
		if(pred(a[i],b)){
			printf("%d ",a[i]);
		}
	}
	puts("");
}
int main(){
	int a[]={10,5,8,15,12,20,13,30};
	int range[2]={5,13};
	printf("{10,5,8,15,12,20,13,30} filtered by {5,13}:");
	filter_print(a,8,between,range);
} 
