#!/bin/bash

# Guardrails: enforce correct usage
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <start> <end>"
    exit 1
fi

start=$1
end=$2

# Hard stop if inputs are not integers
if ! [[ "$start" =~ ^[0-9]+$ && "$end" =~ ^[0-9]+$ ]]; then
    echo "Error: start and end must be integers"
    exit 1
fi

# Business logic
for ((i=start; i<=end; i++)); do
    touch "invalid${i}.conf"
done

echo "Created invalid${start}.conf to invalid${end}.conf"
