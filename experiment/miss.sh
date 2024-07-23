ulimit -s 65520
# sudo ./src/quick_check.out 0 # enable dmp

# test efficiency core
iter=0
while [ $iter -lt 10 ]; do
    sudo ./src/miss_rate.out 1 >> ../../efficiency.txt
    iter=$((iter+1))
done

# sudo ./src/quick_check.out 0 # enable dmp
iter=0
while [ $iter -lt 10 ]; do
    sudo ./src/miss_rate.out 4 >> ../../dmp_enable.txt
    iter=$((iter+1))
done

# sudo ./src/quick_check.out 1 # disable dmp
iter=0
while [ $iter -lt 10 ]; do
    sudo ./src/miss_rate.out 4 >> ../../dmp_disable.txt
    iter=$((iter+1))
done
