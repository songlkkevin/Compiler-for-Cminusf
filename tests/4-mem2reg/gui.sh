#!/bin/bash

cases=$(ls "./output"/*."ll")

if [ -d "./graph" ];then
    rm -rf ./graph
fi

mkdir ./graph

cd ./graph

for case in $cases; do
    name=$(basename "$case" .ll)
    mkdir $name
    cd $name
    opt -passes=dot-cfg ../.$case >> /dev/null
    files=$(ls -a | grep "dot")
    for file in $files; do
        dot $file -Tpng > $(basename "$file" .dot).png
        echo $file
    done
    cd ../
done