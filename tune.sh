#! /bin/bash
read -p "Enter source, crossV, cold, minTopic, maxTopic:" source crossV cold min max
echo 
for ((k = $min; k <= $max; k = k+10))
do
    ./ctr --prefix /home/ll5fy/lab/dataset --source $source --cold $cold --crossV $crossV --num_factors $k --save_lag 20
done
