#!/bin/bash
#DataGraphPath=/home/zubair/Desktop/SuBISOandSI/datasets/zubair/human.txt
#DataGraphPath=/home/zubair/Desktop/SuBISOandSI/datasets/zubair/yeast.txt
#DataGraphPath=/home/zubair/Desktop/SuBISOandSI/datasets/zubair/hprd
DataGraphPath=/home/zubair/Desktop/SuBISOandSI/datasets/zubair/RunningExampleDataGraph.igraph
echo "Data graph $DataGraphPath" > output
NumberofQueryGraph=1
Timeout=1000
#cp /dev/null output
	#for c in 2
	for (( c=0; c<$NumberofQueryGraph; c++ ))
	do  
		#QueryGraphPath=/home/zubair/Desktop/SuBISOandSI/querysets/zubair/humanQueries/q25/q$c.igraph
		#QueryGraphPath=/home/zubair/Desktop/SuBISOandSI/querysets/zubair/yeastQueries/q25/q$c.igraph
		#QueryGraphPath=/home/zubair/Desktop/SuBISOandSI/querysets/zubair/hprdQueries/q25/q$c.igraph
		QueryGraphPath=/home/zubair/Desktop/SuBISOandSI/querysets/RunningExampleQueryGraph.igraph
		echo "Query graph $QueryGraphPath" >> output
		./OrderedSubGlasgow $QueryGraphPath $DataGraphPath $Timeout>> output	
		echo "End" >> output
		grep "TotalSolCount" output > output.ods
	done
echo "TotalSolCount"
grep "TotalSolCount" output|awk '{ SUM += $2} END { print SUM }' 
echo "TotalRuntime"
grep "TotalRuntime" output|awk '{ SUM += $4} END { print SUM }' 
echo "Total Execution Time"
grep "milliseconds" output|awk '{ SUM += $4} END { print SUM }'
grep -h "milliseconds" output >> output.ods

