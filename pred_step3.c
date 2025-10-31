#include <stdio.h>
#include <stdbool.h>

typedef bool(*predicate_fn)(int v,void *ctx);
struct ctx3{predicate_fn a,b,c;void *c1,*c2,*c3;};
bool ge(int v,void *ctx){int k=*(int*)ctx;return v>=k;}
bool le(int v,void *ctx){int k=*(int*)ctx;return v<=k;}
bool ne(int v,void *ctx){int k=*(int*)ctx;return v!=k;}
bool in_n(int v,void *ctx){
	struct ctx3 *p=ctx;
	return p->a(v,p->c1)&&p->b(v,p->c2)
				&&p->c(v,p->c3);
}
int main(){
	int a[]={1,3,5,7,9,0,8,6,4,2};
	int L=3,R=9,N=5;
	struct ctx3 c3={ge,le,ne,&L,&R,&N};
	printf("{1,3,5,7,9,0,8,6,4,2} in {3,9} "
					"and !5:\n");
	for(int i=0;i<10;i++){
		if(in_n(a[i],&c3)){
			printf("%2d",a[i]);
		}
	}
	puts("");
}
	
