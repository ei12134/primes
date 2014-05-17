#include <stdio.h>
#include "quicksort.h"

#define NELEMS(x)  (sizeof(x) / sizeof(x[0]))

// int main() {
	// int arr[]={1,3,2,4,10,4};
	// int i;
	// int n = NELEMS(arr); //tamanho do array "arr"

 	// quick_sort(arr,0,n-1);

 	// for(i=0 ; i<n ; i++)
 	// {
 	 // printf(" %d \n",arr[i]);
 	// } 
 	// return 0;
// }

void quick_sort(int arr[],int minimo,int maximo)
{
 int pivot,j,temp,i;
 if(minimo<maximo)
 {
  pivot = minimo;
  i = minimo;
  j = maximo;
 
  while(i<j)
  {
   while((arr[i]<=arr[pivot])&&(i<maximo))
   {
    i++;
   }
 
   while(arr[j]>arr[pivot])
   {
    j--;
   }
 
   if(i<j)
   { 
    temp=arr[i];
    arr[i]=arr[j];
    arr[j]=temp;
   }
  }
 
  temp=arr[pivot];
  arr[pivot]=arr[j];
  arr[j]=temp;
  quick_sort(arr,minimo,j-1);
  quick_sort(arr,j+1,maximo);
 }
}