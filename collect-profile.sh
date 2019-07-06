#!/bin/bash
set -eu
exec <&-

case "$0" in
	*/*) exedir="${0%/*}";;
	*) exedir=.;;
esac
cd -- "$exedir/Profile"


generate_profile()
{
	./fancontrol2.prof ../fancontrol2.prof.yaml &
	sleep 1
	if ! kill -s 0 "$!"; then
		echo 'Error: Could not run fancontrol2 for profile generation. Does it have read/write access to all configured inputs and PWMs?' >&2
		return 2
	fi

	echo 'Profiling idle phase ...'
	sleep 10

	echo 'Profiling high load phase ...'
	nice stress --cpu "$(exec getconf _NPROCESSORS_ONLN)" --timeout 20

	echo 'Profiling return to idle phase ...'
	sleep 20

	echo 'Profiling successful!'
	kill "$!"
	wait "$!"
}


cleanup()
{
	find -name '*.gcda' -delete
}


cleanup
generate_profile || declare -i rv="$?"
if [ -v rv ]; then
	cleanup
	exit "$rv"
fi
#rm src/config.gcda
