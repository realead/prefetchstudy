#start sh run_prefetch.sh name_of_test
#
# name_of_test:
#          slower
#          static
#          static2
#          binsearch
#


set -e

TARGET=prefetch_$1

cd src
g++ $TARGET.cpp -O3 -std=c++11 -I../timeitcpp/include -o $TARGET
printf 0.99 | ./$TARGET 

