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

Use `sh run_prefetch_slower.sh` to run this experiment.

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

Use `sh run_prefetch_static.sh` to run this experiment.


