#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include<time.h>
#include<iostream>
#include <cmath>
#include <vector>
#include<chrono>
#include<array>
#include <ctime>
#include <stdlib.h>
#include <inttypes.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
using namespace std::chrono;
uint64_t g_seed = time(0);


void printArray(std::vector<int> &A, int size){

    for (int i = 0; i < size ; i++) {
        printf("%   d", A[i]);
        if ( i != size-1) {
            printf(", ");
        }
    }
    printf("\n");
}

void parallel_prefix_sum(std::vector<int> &arr, int n, std::vector<int> &index) {
  
   if (n == 1) {
       index[0] = arr[0];
       return;
   }

   std::vector<int> arr1(n/2, 0);
   std::vector<int> arr2(n/2, 0);

   for(int i = 0; i < n/2; ++i) {
      arr1[i] = arr[2 * i] + arr[(2 * i) + 1];
   }

   parallel_prefix_sum(arr1, n/2, arr2);

   for (int i = 0; i < n; ++i) {
      if (i == 0) {
          index[0] = arr[0];
      } else if (i % 2 == 1) {
          index[i] = arr2[i / 2];
      } else {
          index[i] = arr2[(i - 1)/2] + arr[i];
      }
   }

}

void par_Counting_Rank (std::vector<int> &array, int n, int d, std::vector<int> &r, int p  ){

    std::vector< std::vector <int> > f( pow(2,d), std::vector<int>(p));
    std::vector< std::vector <int> > r1( pow(2,d), std::vector<int>(p));
    std::vector<int> js(p);
    std::vector<int> je(p);
    std::vector<int> ofs(p);


    cilk_for (int i =0;i < p; i++){
        for (int j =0;j< pow(2,d);j++){
            f[j][i]=0;
        }
        js[i]= (i)*floor(n/p);

        if(i<p-1){
            je[i]=((i+1) * floor(n/p) )-1;
        }else{
           je[i]=n-1;
        }
        for (int j =js[i];j<=je[i];j++){
            f[array[j]][i] +=  1;
        }

    }

    for (int j =0;j< pow(2,d);j++){
            std::vector<int> temp(p, 0);
            parallel_prefix_sum(f[j], p, temp); 
            f[j]=temp;

        }

    cilk_for (int i =0;i< p;i++){
        ofs[i]=0;
        for (int j =0;j< pow(2,d);j++){
            if(i==0){
                r1[j][i] = ofs[i];
            }else{
                r1[j][i] =ofs[i] + f[j][i-1];
            }
            ofs[i]=ofs[i]+f[j][p-1];
        }
        for (int j =js[i];j<=je[i];j++){
            r[j] = r1[array[j]][i];
            r1[array[j]][i] = r1[array[j]][i] + 1;
        }       
    }

}

int ExtractBitSegment(int n, int start, int q) {
    int end = start +q-1;
    unsigned int mask = ~(~0 << (end - start + 1));
    return mask & (n >> start);
}
void par_radix_sort_with_counting(std::vector<int> &arr, int n , int bits, int p) {
    std::vector<int> S(n);
    std::vector<int> r(n);
    std::vector<int> b(n);
    int q;
    float l = log( (float(n) )/ ( p*log(n) ) );
    int d =ceil(l);

    for (int k =0;k< bits;k++){

        if(k + d <= bits){
             q = d;
        }else{
             q  = bits -k ;
        }
        cilk_for(int i =0;i<n;i++){
            S[i] = ExtractBitSegment(arr[i], k,q);    
        }
        par_Counting_Rank(S, n, q,r,p);

        cilk_for(int i =0;i<n;i++){
            b[r[i]]=arr[i];
        }
        cilk_for(int i =0;i<n;i++){
            arr[i]=b[i];
        }
    }

}

void checkIfSorted(std::vector<int> &arr , int size){
    int flag=1;
    for(int i =0; i<size;i++){
        if(arr[i]>arr[i+1]){
           printf("array is not sorted %d ..... %d,.....%d\n ", arr[i],arr[i+1],i); 

           flag=0;
        }
    }
    if(flag){
        printf("Array is sorted\n");
    }
}
int randomCoinToss() { 
        g_seed = (214013*g_seed+2531011);
        return ((g_seed>>16)&0x7FFF)%1024;
    }
int main(int argc, char **argv) {

    double start, stop, time_elapsed;
     __cilkrts_set_param("nworkers","68");
    int arraySize = atoi(argv[1]);
    std::vector<int> arr(arraySize, 0);
    for(int i = 0; i < arraySize; ++i) {
       arr[i] = randomCoinToss() % 1024;
   }
    printf("Unsorted Array\n"); 
    std::cout<<"calling   "<<arraySize<<std::endl;

    __int64 now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    par_radix_sort_with_counting(arr, arraySize,10,68);
    __int64 now1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    printf("Sorted Array\n");
    checkIfSorted(arr, arraySize-1);
    float timeTaken=float (now1- now);


    std::cout<<"time taken  ::"<<timeTaken<<std::endl;
        
    return 0;
}
