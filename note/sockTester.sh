#!/bin/bash

# Default variables
SHOW_HEADERS=false
SHOW_FULL=false
DELIMITER=""

# 1. Parse options
# -h = Show headers only
# -v = Show full raw response
# -t = "Heredoc" mode with a custom delimiter (e.g., -t "END")
while getopts "hvt:" opt; do
  case $opt in
    h) SHOW_HEADERS=true ;;
    v) SHOW_FULL=true ;;
    t) DELIMITER="$OPTARG" ;;
    *) echo "Usage: $0 [-h] [-v] [-t delimiter] <payload>" >&2; exit 1 ;;
  esac
done

# 2. Determine Payload Source
if [ -n "$DELIMITER" ]; then
    # --- Interactive / Heredoc Mode ---
    echo "Creating payload... Type '$DELIMITER' on a new line to send."
    PAYLOAD=""
    while IFS= read -r line; do
        # Stop if the specific delimiter is typed
        if [ "$line" == "$DELIMITER" ]; then
            break
        fi
        # Append the input line + \r\n (CRLF) for correct HTTP formatting
        # We escape it as \\r\\n so printf interprets it correctly later
        PAYLOAD="${PAYLOAD}${line}\r\n"
    done
else
    # --- Standard Argument Mode ---
    shift $((OPTIND-1))
    if [ -z "$1" ]; then
        echo "Error: Missing payload argument."
		echo "-h to show header"
		echo "-v to show complete respense"
		echo "-t to work with heredoc"
		echo "you must provide header as arg or with -t"
        exit 1
    fi
    PAYLOAD="$1"
fi

RAW_FILE="raw_response.txt"

# 3. Run netcat
# We use printf to interpret the \r\n characters correctly
printf "$PAYLOAD" | nc 127.0.0.1 1025 > "$RAW_FILE"

# 4. Separate Headers and Body
# 'sed' creates the HTML file by deleting the headers (lines 1 to first empty line)
sed '1,/^\r\{0,1\}$/d' "$RAW_FILE" > result.html

# 5. Handle Terminal Output
if [ "$SHOW_FULL" = true ]; then
    echo "------ FULL RESPONSE ------"
    cat "$RAW_FILE"
elif [ "$SHOW_HEADERS" = true ]; then
    echo "------ HEADERS ONLY ------"
    # This sed command PRINTS lines 1 to the first empty line
    sed -n '1,/^\r\{0,1\}$/p' "$RAW_FILE"
else
    echo "Output saved. Opening Firefox..."
fi

# 6. Open Firefox
firefox result.html &