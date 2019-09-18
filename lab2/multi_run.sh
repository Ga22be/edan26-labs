#!/bin/bash
#set -e
NSYM=10000
NVERTEX=1000
MAXSUCC=4
NACTIVE=100
PRINT=1

#INDEX=0
#while [ $INDEX -lt 100 ];
#for INDEX in 0 1 2 3 4 5 6 7 8 9 10 11 12;
for INDEX in $(seq 0 20);
do
	echo iteration $INDEX
	FILE=f$INDEX
	scala -classpath classes Driver $NSYM $NVERTEX $MAXSUCC $NACTIVE $PRINT > $FILE
	if [ $INDEX -gt 0 ];
	then
		diff f0 $FILE
	fi
#	INDEX=$INDEX+1
done

echo done
