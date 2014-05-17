#include <stdio.h>
#include "numberofodds.h"


/*int main()
 {
	 
	int n = 10; 
	numbodds( n);
 	return 0;
 } */

int numbodds(int n)
{ 
 int totalodds=0;

 if(n<2)
	totalodds=1;

 else
 {
	totalodds=(((n-2)+1)/2)+1;
 }

 printf("total is %d", totalodds);
 printf("\n");

 return 0;
}
