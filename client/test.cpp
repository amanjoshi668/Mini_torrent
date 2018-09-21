#include<bits/stdc++.h>
#include<thread>
using namespace std;
void fun(string h,int x){
	FILE *fp = fopen("temp.txt", "w");
   	fseek(fp, x*5, SEEK_SET);
    	fwrite(h.c_str(),1,5,fp);
}
int main(){
	FILE *fp = fopen("temp.txt", "w");
   	fseek(fp, 40, SEEK_SET);
    	fputc('\0', fp);
    	fclose(fp);
	for(int i=11111;i<11120;i++){
		thread T(fun ,to_string(i),i-11111);
	}	
	return 0;
}
