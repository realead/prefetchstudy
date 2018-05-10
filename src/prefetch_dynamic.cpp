
#include <vector>
#include <iostream>
#include <iomanip>
#include <cmath>

#include <timeitmagic.h>

const int SIZE=1024*1024*1;
const int STEP_CNT=1024*1024*10;

unsigned int next(unsigned int current){
   return (current*10001+328)%SIZE;
}


template<bool prefetch>
struct Worker{
   std::vector<int> mem;

   double result;
   int start;

   void operator()(){
        unsigned int prefetch_index=0;
        for(int i=0;i<start;i++)
            prefetch_index=next(prefetch_index);

        unsigned int index=0;
        for(int i=0;i<STEP_CNT;i++){
            //prefetch memory block used in a future iteration
            if(prefetch){
                __builtin_prefetch(mem.data()+prefetch_index,0,1);    
            }
            //actual work:
            result+=mem[index];

            //prepare next iteration
            prefetch_index=next(prefetch_index);
            index=next(mem[index]);
        }
   }

   Worker(std::vector<int> &mem_):
       mem(mem_), result(0.0), start(0)
   {}
};

 int main() {
     std::vector<int> keys(SIZE);
     for (int i=0;i<SIZE;i++){
       keys[i] = i;
     }
     
     Worker<false> without_prefetch(keys);
     Worker<true> with_prefetch(keys);

     std::cout<<"#preloops\ttime no prefetch\ttime prefetch\tfactor\n";
     std::cout<<std::setprecision(17);
     
     for(int i=0;i<40;i++){
         without_prefetch.start=i;
         with_prefetch.start=i;
         double time_no_prefetch=timeitmagic::timeit(without_prefetch, 3, 1).time;
         double time_with_prefetch=timeitmagic::timeit(with_prefetch,  3, 1).time;
         std::cout<<i<<"\t"
                  <<time_no_prefetch<<"\t"
                  <<time_with_prefetch<<"\t"
                  <<(time_no_prefetch/time_with_prefetch)<<"\n";
     }

 }
