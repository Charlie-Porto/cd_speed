#!/bin/bash

main()
{
    #!< TODO: update
    local cds_backend="$HOME/workspace/projects/cd_speed/build/cds_backend/cds_backend"

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
    local result="$(Scds_backend "$@")"
    if [[ "$result" == cds:* ]]; then
        local dest dir="${result#*:}"
        if [[ ! -d "$dest_dir" ]]; then
            echo "ERROR: output is not a directory: $dest_dir. Aborting."
            return 1
        fi

        cda "$dest_dir"
        return
    fi
    echo "$result"
}

main "$@"
