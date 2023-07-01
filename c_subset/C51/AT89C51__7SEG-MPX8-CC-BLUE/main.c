#include <reg51.h>

int main(){	
	int digits[] = {
		0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
	};
	
	int len = sizeof(digits) / sizeof(int);
	int i;
	int time;
	P2 = 0x00;
	
	while (1){
		for (i = 0;i < len;++i){
			P0 = digits[i];
			
			time = 50000;
			while(--time);
		}
	}
}
