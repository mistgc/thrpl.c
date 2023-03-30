#!/bin/bash

PREFIX=header-only
FILE=thrpl.h
FILE_2=thrpl.c

if [[ ! -d ${PREFIX} ]]; then
    mkdir ${PREFIX}
fi

cp thrpl.h ${PREFIX}/${FILE}

NUM_LINE=$(cat ${PREFIX}/${FILE} | wc -l)
NUM_LINE_MINUS_ONE=$((NUM_LINE - 1))
FILE_END=$(sed -n "${NUM_LINE_MINUS_ONE},${NUM_LINE}p" ${PREFIX}/${FILE})
sed -i "${NUM_LINE_MINUS_ONE},${NUM_LINE}d" ${PREFIX}/${FILE}

STD_HAEDER_ARRAY=($(echo $(cat ${FILE_2} | sed -n "/^#include.*>\$/p")))
STD_HAEDER=""
for i in "${!STD_HAEDER_ARRAY[@]}"; do
    if ((i == 0)); then
        STD_HAEDER="${STD_HAEDER_ARRAY[${i}]} ${STD_HAEDER_ARRAY[$((i+1))]}"
    elif ((i % 2 == 0)); then
        STD_HAEDER="${STD_HAEDER}\n${STD_HAEDER_ARRAY[${i}]} ${STD_HAEDER_ARRAY[$((i+1))]}"
    fi
done
sed -i "/^#include.*>\$/a ${STD_HAEDER}" ${PREFIX}/${FILE}
echo >> ${PREFIX}/${FILE}
echo "#ifdef THRPL_IMPLEMENTATION" >> ${PREFIX}/${FILE}
cat ${FILE_2} | sed "/^#include/d" >> ${PREFIX}/${FILE}
echo >> ${PREFIX}/${FILE}
echo "#endif // THRPL_IMPLEMENTATION" >> ${PREFIX}/${FILE}
cat << EOF >> ${PREFIX}/thrpl.h
${FILE_END}
EOF
