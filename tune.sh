#! /bin/bash
read -p "Enter source, crossV, cold, minTopic, maxTopic:" source crossV cold min max
echo
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
export CFLAGS="-I/usr/local/include"
export LDFLAGS="-L/usr/local/lib"
root_path=~/lab/dataset
dir=$root_path/output/$source/byUser_20k_review/ctr
user=$root_path/$source/byUser_20k_review/CTR/user_0.txt
item=$root_path/$source/byUser_20k_review/CTR/item_0.txt
mult=$root_path/$source/byUser_20k_review/CTR/train_0.txt
for ((k = $min; k <= $max; k = k + 10))
do
    theta=$root_path/$source/byUser_20k_review/CTR/fold0_10.doc.states
    beta=$root_path/$source/byUser_20k_review/CTR/fold0_10.topics
    ./ctr --directory $dir --user $user --item $item --a 1 --b 0.01 --lambda_u 0.01 --lambda_v 100 \
  --mult $mult --theta_init $theta \
  --beta_init $beta --num_factors $k --save_lag 20
done
