#include <stdio.h>

const char *hello_str = "Hello world.\n";

int main(){
	int value = printf(hello_str);
	if (value != strlen(hello_str)){
		return -1;
	} else return 0;
}
