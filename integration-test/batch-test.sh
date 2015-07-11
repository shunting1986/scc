#!/bin/bash

TEST_ROOT="integration-test"
WS_ROOT="temp"
CC="./scc"

red() {
	echo -en "\033[31m"
	echo $*
	echo -en "\033[0m"	
}

blue() {
	echo -en "\033[34m"
	echo $*
	echo -en "\033[0m"	
}

green() {
	echo -en "\033[32m"
	echo $*
	echo -en "\033[0m"	
}

runonetest() {
	test_dir=$1
	prog=$test_dir/main.c
	if [ ! -f $prog ] 
	then
		red $test_dir prog not found
		return
	fi
	asm_file=main.s
	err_file=output.err
	exe_file=prog
	$CC $prog > $WS_ROOT/$asm_file 2> $WS_ROOT/$err_file
	if gcc -m32 $WS_ROOT/$asm_file -o $WS_ROOT/$exe_file 
	then
		true
	else
		red $test_dir asm file can not compile
		return
	fi

	i=1 # go over all the test cases
	act_out=real_out
	blue $test_dir
	while true
	do
		in_file=in.$i
		exp_out=out.$i

		if [ ! -f $test_dir/$in_file ]
		then
			break
		fi

		if $WS_ROOT/$exe_file < $test_dir/$in_file | tee $WS_ROOT/$act_out | diff -u - $test_dir/$exp_out 
		then
			green "  pass $in_file"
		else
			red "  fail $in_file"
		fi

		i=$(expr $i + 1)
	done
}

if [ $# -gt 0 ]
then
	TEST_DIR_LIST=$*
else
	TEST_DIR_LIST=`ls $TEST_ROOT` # will filter non-dir later
fi

for test_dir in $TEST_DIR_LIST
do
	test_dir="$TEST_ROOT/$test_dir"
	if [ ! -d $test_dir ]
	then 
		continue
	fi

	runonetest $test_dir
done


