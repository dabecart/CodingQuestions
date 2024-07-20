#include <stdio.h>

double findMedianSortedArrays(int* nums1, int nums1Size, int* nums2, int nums2Size) {
    int finalLen = (nums1Size+nums2Size)/2 + 1;

    int i1 = 0, i2 = 0;
    int* p1 = NULL;
    int* p2 = NULL;
    while((i1+i2) < finalLen){
        p2 = p1;

        if(i2 >= nums2Size || (i1 < nums1Size && *nums1 <= *nums2)){
            p1 = nums1;    
            nums1++;
            i1++;
        }else{
            p1 = nums2;
            nums2++;
            i2++;
        }
    }

    if((nums1Size+nums2Size) % 2){
        return *p1;
    }else{
        return (*p1 + *p2)/2.0;
    }
}

int main(){
  int a[] = {1,2};
  int b[] = {3,4};
  float res = findMedianSortedArrays(a,2,b,2);
  printf("%f", res);
}