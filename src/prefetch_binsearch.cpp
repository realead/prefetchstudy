 #include <time.h>
 #include <stdio.h>
 #include <stdlib.h>

#include <vector>
#include <iostream>
#include <iomanip>

#include <timeitmagic.h>

double d;
template<bool prefetch>
struct Worker{
   std::vector<double> mem;
   std::vector<double> keys;

   int result;

   int binarySearch(double *array, int number_of_elements, double key) {
         int low = 0, high = number_of_elements-1, mid;
         while(low <= high) {
                 mid = (low + high)/2;
            if(prefetch){
            // low path
            __builtin_prefetch (&array[(mid + 1 + high)/2], 0, 1);
            // high path
            __builtin_prefetch (&array[(low + mid - 1)/2], 0, 1);
            }
                 double val=array[mid]*d/d*d/d*d/d*d/d*d/d*d/d*d/d*d/d;
                 if(val< key)
                         low = mid + 1; 
                 else if(val == key)
                         return mid;
                 else if(val > key)
                         high = mid-1;
         }
         return -1;
   }


   void operator()(){
     for (auto key :keys){
       result += binarySearch(mem.data(), mem.size(), key);
     }
   }

   Worker(std::vector<double> &mem_, std::vector<double> &keys_):
       mem(mem_),keys(keys_), result(0)
   {}
};

 int main() {
     std::cin>>d;
     int SIZE = 1024*1024*256;
     std::vector<double> array(SIZE);
     for (int i=0;i<SIZE;i++){
       array[i] = i;
     }
     int NUM_LOOKUPS = 1024*1024*8;
     srand(time(NULL));
     std::vector<double> keys(NUM_LOOKUPS);
     for (int i=0;i<NUM_LOOKUPS;i++){
       keys[i] = rand() % SIZE;
     }
     
     Worker<false> without_prefetch(array, keys);
     Worker<true> with_prefetch(array, keys);

     std::cout<<"time no prefetch\ttime prefetch\tfactor\n";
     std::cout<<std::setprecision(17);
     
     double time_no_prefetch=timeitmagic::timeit(without_prefetch, 3, 1).time;
     double time_with_prefetch=timeitmagic::timeit(with_prefetch,  3, 1).time;
      std::cout<<time_no_prefetch<<"\t"
               <<time_with_prefetch<<"\t"
               <<(time_no_prefetch/time_with_prefetch)<<"\n";
 }
