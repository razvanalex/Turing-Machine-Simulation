#!/bin/bash

TASK1_DIR="./tests/Task1"
TASK2_DIR="./tests/Task2"

INPUT_DIR="input"
REF_DIR="ref"

SOLUTION_FILE="task2"

TASK1_SCORE=5
TASK2_SCORE=4

TIMEOUT=20s

INPUT_FILE="task1.in"
OUTPUT_FILE="task1.out"
TEMP_FILE="task1.temp"
TEMP2_FILE="temp"

task1_score=0
task2_score=0
total_score=0

TASK1_ENDING="TASK1_SCORE"
len1=${#TASK1_ENDING}
TASK2_ENDING="TASK2_SCORE"
len2=${#TASK2_ENDING}
CHECKER_ENDING="FINAL_SCORE"
len3=${#CHECKER_ENDING}

arch=$(uname -i)
EXEC="./simulator_32bit"
if [[ $arch == "x86_64" ]]; then
	EXEC="./simulator_64bit"
fi

make clean &> /dev/null
exit_code=$?
if [[ $exit_code == 2 ]]; then
	printf "No rule to make target clean\n"
	exit 1
fi

printf "\n                 ******* Testing first task *******\n"
for test_file in $(ls "$TASK1_DIR/$INPUT_DIR")
do
	rm -rf "./$OUTPUT_FILE"

	len=${#test_file}
	diff_len=$(( 60 - len ))
	
	printf "$test_file " 
	printf "%"$diff_len"s" "." | tr " " "."
	printf " "
	
	score=0
	cp "$TASK1_DIR/$INPUT_DIR/$test_file" "./$INPUT_FILE"
	REF_FILE="$TASK1_DIR/$REF_DIR/"$test_file".ref"
	
	make build &> /dev/null
	exit_code=$?
	if [[ $exit_code == 2 ]]; then
		printf "[ BUILD ERROR ]\n"
		continue
	fi

	timeout $TIMEOUT make run &> /dev/null
	exit_code=$?
	if [[ $exit_code == 2 ]]; then
		printf "[ RUN ERROR ]\n"
		continue
	fi

	if [[ $exit_code == 0 && -f "./$OUTPUT_FILE" ]]; then
		cat "./$OUTPUT_FILE" | sed 's/#//g' > $TEMP_FILE
		diff "./$TEMP_FILE" $REF_FILE &> /dev/null
		if [[ $? == 0 ]]; then
			score=$TASK1_SCORE
		fi
	fi

	task1_score=$(( task1_score + score ))
	if [[ $exit_code == 124 ]]; then
		printf "[ TLE ]\n"
	else
		printf "[ %d/%d ]\n" $score $TASK1_SCORE
	fi
done

rm -rf "./$INPUT_FILE"
rm -rf "./$OUTPUT_FILE"
rm -rf "./$TEMP_FILE"

make clean &> /dev/null

printf "\n"$TASK1_ENDING"_%"$(( 60 - len1 ))"s_[_%d/%d_]\n" "." $task1_score 60 | tr " " "." | tr "_" " "

printf "\n\n"

printf "                ******* Testing second task *******\n"

INPUT_FILE="input"
OUTPUT_FILE="output"

if [[ ! -f "./$SOLUTION_FILE" ]]; then
	printf "\n.................Missing second task solution file..........."
	printf "\n"
else
	for test_file in $(ls "$TASK2_DIR/$INPUT_DIR")
	do
		rm -rf "./$OUTPUT_FILE"

		len=${#test_file}
		diff_len=$(( 60 - len ))
	
		printf "$test_file " 
		printf "%"$diff_len"s" "." | tr " " "."
		printf " "
	
		score=0
		cat "./$SOLUTION_FILE" > "./$INPUT_FILE"
		printf "\n" >> "./$INPUT_FILE"
		cat "$TASK2_DIR/$INPUT_DIR/$test_file" >> "./$INPUT_FILE"
		REF_FILE="$TASK2_DIR/$REF_DIR/"$test_file".ref"

		timeout $TIMEOUT $EXEC &> /dev/null
		exit_code=$?
		if [[ $exit_code == 0 && -f "./$OUTPUT_FILE" ]]; then
			diff "./$OUTPUT_FILE" $REF_FILE &> /dev/null
			if [[ $? == 0 ]]; then
				score=$TASK2_SCORE
			fi
		fi

		task2_score=$(( task2_score + score ))
		if [[ $exit_code == 124 ]]; then
			printf "[ TLE ]\n"
		else
			printf "[ %d/%d ]\n" $score $TASK2_SCORE
		fi
	done
fi

rm -rf "./$INPUT_FILE"
rm -rf "./$OUTPUT_FILE"

printf "\n"$TASK2_ENDING"_%"$(( 60 - len2 ))"s_[_%d/%d_]\n" "." $task2_score 40 | tr " " "." | tr "_" " "

total_score=$(( task1_score + task2_score ))

printf "\n"$CHECKER_ENDING"_%"$(( 60 - len3 ))"s_[_%d/%d_]\n" "." $total_score 100 | tr " " "." | tr "_" " "

