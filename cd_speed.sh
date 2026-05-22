#!/bin/bash

main()
{
    #!< TODO: update
    local cds_backend="/usr/local/bin/cd-speed"

    # TODO: implement ability to CRUD dr keywords (*including nested)
    local flags_map=(
        -a --add
        -rm --remove
        -rl --relink
    )
    # If no args passed to backend, it prints the manual and extts
    if [[ -z "$1" ]]; then
        $cds_backend
        return
    fi
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
