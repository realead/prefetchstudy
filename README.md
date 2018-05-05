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


