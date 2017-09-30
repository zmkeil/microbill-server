#!/bin/bash

for tt in `ls test_* | grep -v "\."`;
do
    echo "---${tt}---"
    ./${tt}
    echo ""
    echo ""
done

