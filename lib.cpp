
#include "lib.h"

string itoa(int num ) {
    if (num == 0) {
        return "0";
    }
    char str[INT_LEN], tmp[INT_LEN];
    memset(&str, '\0', INT_LEN);
    memset(&tmp, '\0', INT_LEN);
	int i=0,j;
	while(num>0){
		tmp[i] = (char)((num%10)+'0');
		num/=10;
		++i;
	}
	for(j=i;j>=0;--j)
		str[i-j-1]=tmp[j];
    return str;
}