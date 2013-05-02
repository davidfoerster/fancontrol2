#!/bin/bash
set -e
[ "${0%/*}" = "$0" ] || cd "${0%/*}"
cd Profile

generate_profile() {
	./fancontrol2.prof ../fancontrol2.prof.yaml <&- &
	sleep 1
	if ! kill -s 0 `jobs -p %1`; then
		echo 'Error: Could not run fancontrol2 for profile generation. Does it have read/write access to all configured inputs and PWMs?' >&2
		return 2
	fi
	
	echo 'Profiling idle phase ...'
	sleep 10

	echo 'Profiling high load phase ...'
	stress --cpu "$(getconf _NPROCESSORS_ONLN)" --timeout 20

	echo 'Profiling return to idle phase ...'
	sleep 20
	
	echo 'Profiling successful!'
	kill %1
	wait %1 
}

cleanup() {
	find -name \*.gcda -delete
}

cleanup
generate_profile || { r=$?; cleanup; exit $r; }
#rm src/config.gcda
