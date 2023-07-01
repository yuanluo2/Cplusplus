#include <reg51.h>

int main(){
	int positions[] = {
		0x01, 0x02, 0x40, 0x10, 0x08, 0x04, 0x40, 0x20, 0x80
	};
	
	int len = sizeof(positions) / sizeof(int);
	int i;
	int time;
	
	while (1){
		for (i = 0;i < len;++i){
			P2 = positions[i];
			
			time = 10000;
			while(--time);
		}
	}
}
