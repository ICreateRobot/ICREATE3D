#include <REG51.h>
sbit LED = P3^3;
sbit key = P3^2;
int flag=0;
void DELAY_MS (unsigned int a){
    unsigned int i;
    while ( --a != 0 )    {
        for (i=0;i<=600;i++);
    }
}
 void GPIO_Init()
{
//	P3M0|=0x00;//0000 0000
//	P3M1|=0x04;//0000 0000 
	P3M0|=0x08;//0000 0000
	P3M1|=0x04;//0000 0000 
//	P3M1=0xA0;//0000 0100B
//	P3M0=0xC0;//1111 1011B
}
void main (void) 
	{
		 key=0;
		 LED=0;
		 GPIO_Init();

     while (1)    
			 {

				 if(key==1)
				 {
				flag=1;
				 }
				 if(flag==1)
				 {
				  LED = 1;
				 }
   }
}

