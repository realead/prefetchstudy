# prefetch study

my experiments to get a visible impact by using prefetching `__builtin_prefetch()`.


### Instalation

https://github.com/realead/timeitcpp is used as submodule, thus clone it with `git clone --recursive`.


### Prerequisites

    gcc-compiler

## Experiments

### prefetch_slower

First experiment is to slow down the calculation with help of `__builtin_prefetch()` in order to see, that there is really an impact.

The idea is quite simple: take a memory-bound task and prefetch data which has to be loaded into the cache and gets envicted from the cache before it can be used and thus hurting the data throughput for the real calcualtion.

Here is the core of the implementation:

    void operator()(){
        for(int i=0;i<n;i++){
            if(prefetch){
                //prefetch memory which cannot be used
                int offset=(i*8+6000)%n;
                __builtin_prefetch(mem+offset);
            }
            result+=mem[i];
        }
    }

prefetched version war 4 times slower on my machine. 

Use `sh run_prefetch.sh slower` to run this experiment.

### prefetch_static

The idea is to fetch a cache-line, which will be needed in the next iteration and to interleave calculation with data reading.

However, the hardware seems to be doing Ok on its own - there was no speed-up.

The core implementation is:


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

the number of operation could be variable.

Use `sh run_prefetch.sh static` to run this experiment.


### prefetch_static2

The idea is to prefetch a cache-line, which will be needed in an iteration somewhere further in the future



The difference to the first implementation is the prefech-line:


    __builtin_prefetch(mem+cnt+30*BLOCK_SIZE);

the prefetched memory is 30 blocks in the future and not only one.

The timings are:

    #work	time no prefetch	time prefetch	factor
    0	0.025815299999999999	0.025547713499999999	1.0104739901674566
    1	0.025897100999999999	0.0252688605	1.0248622410179518
    2	0.025598140500000002	0.025570628500000001	1.0010759219312892
    3	0.025965724499999999	0.025574537000000001	1.0152959758372164
    4	0.027150131000000001	0.025689754499999998	1.0768466506754668
    5	0.034004219500000002	0.034286592499999997	0.99176433178654322

It looks like as if the CPU is doing a very decent job interleaving reading and calculating, when the differences between reading and calculating are big. However, it might use a little bit help when the times for fetching data and calculating are almost equal (here in the case of `work=4`. This were our prefetch helps a little - the CPU doesn't have to think and prefetches data which might be used in the future.

Obviously, prefetching the next block will not help due to latency of the memory - so we need to prefetch data further in the future, so even with latency the data will be on time.

Use `sh run_prefetch.sh static2` to run this experiment.


### prefetch_binsearch

In the examples above the memory run almost on its limit, there was not much to improve. During the binary search the memory bandwidth isn't exhausted at all, the bottle-neck is the latency of the access - we can afford to fetch twice amount of data and discard the one half not needed later without a major negative impact.

Here is the code:


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

the CPU prefetcher cannot prefetch anything, because the index must be calculated. There is however not much to win - there is clearly not enough calculations going on.

timings are:

    time no prefetch	time prefetch	factor
    22.215248963000001	16.594471300999999	1.3387138740395597

About 30% faster!

Use `sh run_prefetch.sh binsearch` to run this experiment.
