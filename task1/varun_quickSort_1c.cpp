#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include<time.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <limits.h>
#include <time.h>
#include <math.h>
#include<chrono>

int *lt;
int *gt;
long *copyArray;    
using namespace std::chrono;
int m;
int random_int (unsigned int low, unsigned int high)
{
    int random = rand();
    if (RAND_MAX == random) {
        return random_int(low, high);
    }

    int range = high - low,
        remain = RAND_MAX % range,
        slot = RAND_MAX / range;

    if (random < RAND_MAX - remain) {
        return low + random / slot;
    } else {
        return random_int (low, high);
    }
}
/* printf if PRINTMODE is defined */

void printArray(long *A, int lo, int hi){
    int i;

    printf("[");
    for (i = lo; i <= hi ; i++) {
        printf("%ld", A[i]);
        if ( i != hi) {
            printf(", ");
        }
    }
    printf("]\n");
}

/* print an array if PRINTMODE is defined */


void parallel_prefix_sum(int *lt, int *gt,  int start, int n, int k) {

    int i, h;
    for(h = 1 ; h <= k ; h++) {
        cilk_for (i = 1; i <= (n >> h); i++) {
            lt[start+i * (1 << h) - 1] += lt[start+(1 << h) * i - (1 << (h-1)) - 1];
            gt[start+i * (1 << h) - 1] +=  gt[start+(1 << h) * i - (1 << (h-1)) - 1];
        }
    }

    for (h = k ; h >= 1; h--) {
        cilk_for(i = 2; i <= (n >> (h-1)); i++){
            if (i % 2) {
                lt[start+i * (1 << (h-1)) -1] += lt[start+i * (1 << (h-1)) - (1 << (h-1)) - 1];
                gt[start+i * (1 << (h-1)) -1] += gt[start+i * (1 << (h-1)) - (1 << (h-1)) - 1];
            }
        }
    }

}

void sequential_insertionSort(long *array, int left, int right) {

    int i, j, val;
    for(i = left; i <= right; i++) {
        val = array[i];
        j = i - 1;
        while( j >= 0 && array[j] > val) {
            array[j+1] = array[j];
            j--;
        }
        array[j+1] = val;
    }
}

int partition(long *array, int left, int right){

    int n = (right - left + 1),
        k = (int) log2(n),
        i;

    long pivot = array[right];

    cilk_for (i = left; i <= right; i++) {

        copyArray[i] = array[i];

        if (array[i] < pivot) {
            lt[i]=1;
            gt[i]=0;
        }
        else if(array[i]>pivot) {
            lt[i]=0;
            gt[i]=1;
        }
        else{
            if(i==right){
                lt[i]=0;
            }
            else {
                lt[i]=1;
            }
            gt[i]=0;
        }
    }

    parallel_prefix_sum(lt, gt, left, n,k);

    int pivotIndex = left+lt[right];
    array[pivotIndex] = pivot;

    cilk_for (i = left; i < right; i++){
        if(copyArray[i]<=pivot){
            array[left+lt[i]-1] = copyArray[i];
        }
        else if (copyArray[i]>pivot){
            array[pivotIndex+gt[i]] = copyArray[i];
        }
    }

    return pivotIndex;
}

void quicksort(long *array,int left,int right){

    if(left >= right) {
        return;
    }

    if (right - left + 1 <= 32) {
        return sequential_insertionSort(array, left, right);
    }
    else {
        int splitPoint;

            splitPoint = partition(array,left, right);

        cilk_spawn quicksort(array,left,splitPoint-1);
        quicksort(array,splitPoint+1,right);
    }
}

void checkIfSorted(long *arr, int size){
    int flag=1;
    for(int i =0; i<size;i++){
        if(arr[i]>arr[i+1]){
           printf("array is not sorted %ld ..... %ld\n ", arr[i],arr[i+1]); 
           flag=0;
        }
    }
    if(flag){
        printf("Array is sorted\n");
    }
}
uint64_t g_seed = time(0);
static int randomCoinToss() { 
        g_seed = (214013*g_seed+2531011);
        return ((g_seed>>16)&0x7FFF)%1024;
    }
int main(int argc, char **argv) {
	
    __cilkrts_set_param("nworkers","1");
    double start, stop, time_elapsed;
   int length = atoi(argv[1]);
    copyArray = (long *) malloc (sizeof(long) * length);
    lt = (int *) malloc (sizeof(int) * length);
    gt = (int *) malloc (sizeof(int) * length);
    for(int i = 0 ; i < length ; i++) {
        copyArray[i] = 0;
        lt[i] = 0;
        gt[i] = 0;
    }

    long* array = (long *) malloc(length * sizeof(long));
    int i;
   for(i = 0; i < length; i++){
        array[i] =randomCoinToss();
    }
    printf("Unsorted Array\n");
   /* printArray(array,0, length-1);*/
    __int64 now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    quicksort(array, 0,length-1);
    __int64 now1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    printf("Sorted Array\n");
    /*printArray(array,length);*/
    checkIfSorted(array , length-1); 
    float timeTaken=float(now1- now);
    std::cout<<timeTaken<<std::endl;
    return 0;
}
