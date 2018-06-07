#!/bin/bash

bash -c /home/matze/workspace/uni/2018S/osue/own/3/bin/supervisor &

for i in {1..3}
do
    timeout 5s /home/matze/workspace/uni/2018S/osue/own/3/bin/generator 0-1 0-3 0-4 1-2 1-3 1-4 1-5 2-4 2-5 3-4 4-5
done



