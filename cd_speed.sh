#!/bin/bash

main()
{
    local cds_backend="/usr/local/bin/cd-speed"

    # If no args passed, print the manual and exit
    local result="$($cds_backend "$@")"
    if [[ "$result" == cds:* ]]; then
        local dest_dir="${result#*:}"
        if [[ ! -d "$dest_dir" ]]; then
            echo "ERROR: output is not a directory: $dest_dir. Aborting."
            return 1
        fi

        cd "$dest_dir" && ls -G
        return
    fi
    echo "$result"
}

main "$@"
