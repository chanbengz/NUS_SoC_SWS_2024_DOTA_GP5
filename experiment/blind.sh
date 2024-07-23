ulimit -s 65520

# test 
iter=0
while [ $iter -lt 10 ]; do
    sudo ./src/blinding.out >> ../data/blinding.txt
    iter=$((iter+1))
done

