show_instances()
{
   local cur="$1"
   local bufs=$(trace-cmd list -B)
   if [ "$bufs" == "No buffer instances defined" ]; then
	return 0
   fi
   COMPREPLY=( $(compgen -W "${bufs}" -- "${cur}") )
   return 0
}

show_options()
{
   local cur="$1"
   local options=$(trace-cmd list -o | sed -e 's/^\(no\)*\(.*\)/\2 no\2/')
   COMPREPLY=( $(compgen -W "${options}" -- "${cur}") )
   return 0
}

__show_files()
{
    COMPREPLY=( $(compgen -f -- "$cur") )
    if [ ${#COMPREPLY[@]} -gt 1 ]; then
	    return 0;
    fi
    # directories get '/' instead of space
    DIRS=( $(compgen -d -- "$cur"))
    if [ ${#DIRS[@]} -eq 1 ]; then
	compopt -o nospace
	COMPREPLY="$DIRS/"
	return 0;
    fi
    return 0
}

cmd_options()
{
    local type="$1"
    local cur="$2"
    local cmds=$(trace-cmd $type -h 2>/dev/null|grep "^ *-" | \
				 sed -e 's/ *\(-[^ ]*\).*/\1/')
    COMPREPLY=( $(compgen -W "${cmds}" -- "${cur}") )
    if [ ${#COMPREPLY[@]} -eq 0 ]; then
	__show_files "$cur"
    fi
}

plugin_options()
{
    local cur="$1"

    local opts=$(trace-cmd list -O | sed -ne 's/option://p')
    COMPREPLY=( $(compgen -W "${opts}" -- "${cur}") )
}

__trace_cmd_list_complete()
{
    local prev=$1
    local cur=$2
    shift 2
    local words=("$@")

    case "$prev" in
	list)
	    local cmds=$(trace-cmd list -h |egrep "^ {10}-" | \
				 sed -e 's/.*\(-.\).*/\1/')
	    COMPREPLY=( $(compgen -W "${cmds}" -- "${cur}") )
	    ;;
	*)
	    size=${#words[@]}
	    if [ $size -gt 3 ]; then
		if [ "$cur" == "-" ]; then
		    let size=$size-3
		else
		    let size=$size-2
		fi
		local w="${words[$size]}"
		if [ "$w" == "-e" ]; then
		    local cmds=$(trace-cmd list -h |egrep "^ {12}-" | \
				 sed -e 's/.*\(-.\).*/\1/')
		    COMPREPLY=( $(compgen -W "${cmds}" -- "${cur}") )
		fi
	    fi
	    ;;
    esac
}

__trace_cmd_show_complete()
{
    local prev=$1
    local cur=$2
    shift 2
    local words=("$@")

    case "$prev" in
	-B)
	    show_instances "$cur"
	    ;;
	*)
	    cmd_options show "$cur"
	    ;;
    esac
}

__trace_cmd_extract_complete()
{
    local prev=$1
    local cur=$2
    shift 2
    local words=("$@")

    case "$prev" in
	extract)
	    cmd_options "$prev" "$cur"
	    ;;
	-B)
	    show_instances "$cur"
	    ;;
	*)
	    __show_files
	    ;;
    esac
}

__trace_cmd_record_complete()
{
    local prev=$1
    local cur=$2
    shift 2
    local words=("$@")

    case "$prev" in
        -e)
            local events=$(trace-cmd list -e)
            local prefix=${cur%%:*}

            COMPREPLY=( $(compgen -W "${events}" -- "${cur}") )

            # This is still to handle the "*:*" special case
            if [[ -n "$prefix" ]]; then
                local reply_n=${#COMPREPLY[*]}
                for (( i = 0; i < $reply_n; i++)); do
                    COMPREPLY[$i]=${COMPREPLY[i]##${prefix}:}
                done
            fi
            ;;
        -p)
            local plugins=$(trace-cmd list -p)
	    COMPREPLY=( $(compgen -W "${plugins}" -- "${cur}" ) )
            ;;
        -l|-n|-g)
            # This is extremely slow still (may take >1sec).
            local funcs=$(trace-cmd list -f | sed 's/ .*//')
            COMPREPLY=( $(compgen -W "${funcs}" -- "${cur}") )
            ;;
	-B)
	    show_instances "$cur"
	    ;;
	-O)
	    show_options "$cur"
	    ;;
        *)
	    # stream start and profile do not show all options
	    cmd_options record "$cur"
	    ;;
    esac
}

__trace_cmd_report_complete()
{
    local prev=$1
    local cur=$2
    shift 2
    local words=("$@")

    case "$prev" in
	-O)
	    plugin_options "$cur"
	    ;;
        *)
	    cmd_options report "$cur"
	    ;;
    esac
}

__trace_cmd_dump_complete()
{
    local prev=$1
    local cur=$2
    shift 2
    local words=("$@")

    case "$prev" in
	-i)
	    __show_files
	    ;;
	*)
	    cmd_options dump "$cur"
	    ;;
    esac
}

__show_command_options()
{
    local command="$1"
    local cur="$2"
    local cmds=( $(trace-cmd --help 2>/dev/null | \
		    grep " - " | sed 's/^ *//; s/ -.*//') )

    for cmd in ${cmds[@]}; do
	if [ $cmd == "$command" ]; then
	    local opts=$(trace-cmd $cmd -h 2>/dev/null|grep "^ *-" | \
				 sed -e 's/ *\(-[^ ]*\).*/\1/')
	    COMPREPLY=( $(compgen -W "${opts}" -- "$cur") )
	    return 0
	fi
    done
    __show_files "$cur"
}

_trace_cmd_complete()
{
    local cur=""
    local prev=""
    local words=()

    # Not to use COMP_WORDS to avoid buggy behavior of Bash when
    # handling with words including ":", like:
    #
    # prev="${COMP_WORDS[COMP_CWORD-1]}"
    # cur="${COMP_WORDS[COMP_CWORD]}"
    #
    # Instead, we use _get_comp_words_by_ref() magic.
    _get_comp_words_by_ref -n : cur prev words

    if [ "$prev" == "trace-cmd" ]; then
            local cmds=$(trace-cmd --help 2>/dev/null | \
                                grep " - " | sed 's/^ *//; s/ -.*//')
            COMPREPLY=( $(compgen -W "${cmds}" -- "${cur}") )
	    return;
    fi

    local w="${words[1]}"

    case "$w" in
	list)
	    __trace_cmd_list_complete "${prev}" "${cur}" ${words[@]}
	    return 0
	    ;;
	show)
	    __trace_cmd_show_complete "${prev}" "${cur}" ${words[@]}
	    return 0
	    ;;
	extract)
	    __trace_cmd_extract_complete "${prev}" "${cur}" ${words[@]}
	    return 0
	    ;;
	record|stream|start|profile)
	    __trace_cmd_record_complete "${prev}" "${cur}" ${words[@]}
	    return 0
	    ;;
	report)
	    __trace_cmd_report_complete "${prev}" "${cur}" ${words[@]}
	    return 0
	    ;;
	dump)
	    __trace_cmd_dump_complete "${prev}" "${cur}" ${words[@]}
	    return 0
	    ;;
        *)
	    __show_command_options "$w" "${cur}"
            ;;
    esac
}
complete -F _trace_cmd_complete trace-cmd
