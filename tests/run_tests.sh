#!/bin/sh
# Regression tests for TSCOS.
#  1. The scripted demo must reproduce the execution log in docs/execution_log.txt.
#  2. Interactive commands must produce the expected scheduler behaviour.
#  3. At no point may two approaches be GREEN at the same time (mutual exclusion).
set -e
cd "$(dirname "$0")/.."
make -s
fail=0

# 1. scripted demo
./tscos > /tmp/tscos_demo.txt
sed 's/[[:space:]]*$//' docs/execution_log.txt > /tmp/tscos_expected.txt
sed 's/[[:space:]]*$//' /tmp/tscos_demo.txt > /tmp/tscos_actual.txt
if diff /tmp/tscos_expected.txt /tmp/tscos_actual.txt > /dev/null; then
    echo "PASS  demo output matches docs/execution_log.txt"
else
    echo "FAIL  demo output differs from docs/execution_log.txt"; fail=1
fi

# 2. interactive behaviour
./tscos -i < tests/interactive_input.txt > /tmp/tscos_interactive.txt
for expect in "EMERGENCY_PREEMPT" "WALK (15s)" "ROUND_ROBIN" "locked by operator" "Unknown or malformed"; do
    if grep -q "$expect" /tmp/tscos_interactive.txt; then
        echo "PASS  interactive output contains: $expect"
    else
        echo "FAIL  interactive output missing: $expect"; fail=1
    fi
done

# 3. mutual exclusion: every signal line shows at most one GREEN lamp
if grep "N:" /tmp/tscos_demo.txt /tmp/tscos_interactive.txt | awk -F'GREEN' 'NF>2 {bad=1} END {exit bad}'; then
    echo "PASS  never more than one approach GREEN at a time"
else
    echo "FAIL  two approaches GREEN at the same time"; fail=1
fi

exit $fail
