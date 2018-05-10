# prefetch study

my experiments to get a visible impact by using prefetching `__builtin_prefetch()`.


### Instalation

https://github.com/realead/timeitcpp is used as submodule, thus clone it with `git clone --recursive`.


### Prerequisites

    g++-compiler, c++11

### Remarks
   
   See also my answer on stackoverflow: https://stackoverflow.com/a/50280085/5769463

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

### prefetch_dynamic

The idea of this experiment is: 

   - a task where latency is the bottle-neck
   - for this to work, the next address must depend on data which was fetched in the current state
   - we have an oracle which predicts the address of a read access in the future

That leads to:


    void operator()(){
        //let the oracle to see "start"-steps in the future
        unsigned int prefetch_index=0;
        for(int i=0;i<start;i++)
            prefetch_index=next(prefetch_index);

        unsigned int index=0;
        for(int i=0;i<STEP_CNT;i++){
            //prefetch memory block used in a future iteration
            if(prefetch){
                __builtin_prefetch(mem.data()+prefetch_index,0,1);    //use oracle
            }
            //actual work:
            result+=mem[index];

            //prepare next iteration
            prefetch_index=next(prefetch_index);  //update oracel
            index=next(mem[index]);               //find next address, important dependent on the currently read data!
        }
    }

the timings are:

    #preloops	time no prefetch	time prefetch	factor
    0	4.9063679090000001	1.203082424	4.0781643976539383
    1	5.3876680219999997	1.0445899620000001	5.1576869565974244
    2	5.0467276940000003	0.96847048499999999	5.2110289081241339
    3	5.6067651109999996	0.92860332499999998	6.0378473348671235
    4	6.4164107310000000	1.10179924400000000	5.8235751802712254
    5	7.2576901500000002	0.93963182999999995	7.7239722179271011
    6	7.0159207700000001	0.88947950099999995	7.8876699936449697
    7	6.1601189139999999	0.85285508300000001	7.2229374448132351
    8	6.1971650140000003	0.83146420300000001	7.4533154784536171
    9	6.4531074630000003	0.84810824699999998	7.608825271805193
    10	6.2590855139999997	0.82296781399999996	7.6055046230520018
    11	6.0823899319999999	0.82182061500000003	7.4011162788852642
    12	6.3203557430000004	0.83311849800000004	7.5863826792620319
    13	6.1884857300000000	0.84075347199999995	7.3606424904540866
    14	6.4069166900000001	0.85721438000000005	7.4741124734748379
    15	6.1613906839999997	0.86057395800000003	7.1596294853254197
    16	6.1588444669999998	0.86444315000000005	7.1246379440915222
    17	6.0621799899999997	0.86445854799999999	7.0126902024687938
    18	6.0074136640000004	0.86624884300000005	6.9349745313310622
    19	6.0111945499999999	0.87757806900000002	6.8497547538417347
    20	4.6404857179999999	0.83469427399999996	5.5595034763590583
    21	4.6747815509999997	0.83781637200000003	5.5797209355560309
    22	4.4734154190000002	0.84093902099999995	5.31954791880207
    23	4.4506488290000004	0.84109674499999998	5.2914826450790748
    24	4.4516640499999998	0.84378333800000005	5.2758378241405843
    25	4.5036931910000000	0.84558327799999999	5.3261379549182619
    26	4.7338579149999997	0.86002131599999998	5.504349516611283

So almost 8 times faster with prefetch!

Use `sh run_prefetch.sh dynamic` to run this experiment.
