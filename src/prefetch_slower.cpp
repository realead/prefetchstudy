/*
  
  uses prefetch in order to slow down the calculation:

    1) this is a memory bound task
    2) prefetch just fetches memory which is not needed and gets envicted from the cache before it can be used
    3) the memory band width is reduced because it has to be shared with unnecessary prefetches

*/
#include <vector>
#include <iostream>
#include <iomanip>

#include <timeitmagic.h>


template<bool prefetch>
struct Worker{
   double *mem;
   int n;
   double result;
   void operator()(){
        for(int i=0;i<n;i++){
            //prefetch memory which cannot be used
            if(prefetch){
                int offset=(i*8+6000)%n;
                __builtin_prefetch(mem+offset);
            }
            result+=mem[i];
        }
   }

   Worker(std::vector<double> &mem_):
       mem(mem_.data()), n(mem_.size()), result(1.0)
   {}
};

const int N=2000000;
const int ITER=25;
const int TRIES=10;

int main(){
 
   //init working memory:
   double val;
   std::cin>>val;
   std::vector<double> vec(N);
   for(int i=0;i<N;i++)
       vec[i]=i+val;

   Worker<false> without_prefetch(vec);
   Worker<true> with_prefetch(vec);

   std::cout<<"time no prefetch\ttime prefetch\tfactor\n";
   std::cout<<std::setprecision(17);
   double time_no_prefetch=timeitmagic::timeit(without_prefetch, TRIES, ITER).time;
   double time_with_prefetch=timeitmagic::timeit(with_prefetch,  TRIES, ITER).time;
   std::cout<<time_no_prefetch<<"\t"
            <<time_with_prefetch<<"\t"
            <<(time_no_prefetch/time_with_prefetch)<<"\n";

   //ensure that nothing is optimized away:
   std::cout<<"Results: "<<without_prefetch.result<<" vs. "<<with_prefetch.result<<"\n";
}

