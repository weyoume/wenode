find . -name "*ezira*" -exec bash -c 'mv "$1" "${1/ezira/eznode}"' - '{}' \;