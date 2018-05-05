
#include <vector>
#include <iostream>
#include <iomanip>

#include <timeitmagic.h>


const int BLOCK_SIZE=8;

template<bool prefetch>
struct Worker{
   double *mem;
   int n;
   int times;  
   double result;
   void operator()(){
        int cnt=0;
        for(int i=0;i<n;i+=BLOCK_SIZE){
            //prefetch memory block used in the next iteration
            if(prefetch){
                __builtin_prefetch(mem+cnt+BLOCK_SIZE);
            }
            //process a cache line
            for(int j=0;j<BLOCK_SIZE;j++){
                double d=mem[cnt];
                //do some work, which cannot be optimized away
                for(int t=0;t<times;t++){
                    d=(d-mem[cnt])+mem[cnt];
                }
                result+=d;
                cnt++;
            }
        }
   }

   Worker(std::vector<double> &mem_):
       mem(mem_.data()),n(static_cast<int>(mem_.size())), times(0), result(1.0)
   {}
};


const int N=BLOCK_SIZE*1000000;
const int ITER=2;
const int TRIES=5;

int main(){
 
   //init working memory:
   double val;
   std::cin>>val;
   std::vector<double> vec(N);
   for(int i=0;i<N;i++)
       vec[i]=i+val;

   Worker<false> without_prefetch(vec);
   Worker<true> with_prefetch(vec);

   std::cout<<"#work\ttime no prefetch\ttime prefetch\tfactor\n";
   std::cout<<std::setprecision(17);
   for(int t=0;t<20;t++){
      without_prefetch.times=t;
      with_prefetch.times=t;
      double time_no_prefetch=timeitmagic::timeit(without_prefetch, TRIES, ITER).time;
      double time_with_prefetch=timeitmagic::timeit(with_prefetch,  TRIES, ITER).time;
      std::cout<<t<<"\t"
               <<time_no_prefetch<<"\t"
               <<time_with_prefetch<<"\t"
               <<(time_no_prefetch/time_with_prefetch)<<"\n";
   }

   //ensure that nothing is optimized away:
   std::cout<<"Results: "<<without_prefetch.result<<" vs. "<<with_prefetch.result<<"\n";
}

