#pragma once
#ifndef TOM_COMMANDS_STUBS_INTEGRATESTUBS_HPP
#define TOM_COMMANDS_STUBS_INTEGRATESTUBS_HPP

#include <orm/macros/systemheader.hpp>
TINY_SYSTEM_HEADER

#include <orm/macros/commonnamespace.hpp>

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Tom::Commands::Stubs
{

    /*! Register-ArgumentCompleter command content. */
    inline const auto *const RegisterArgumentCompleter =
R"(
# TinyORM tom tab-completion
# ---

Register-ArgumentCompleter -Native -CommandName tom -ScriptBlock {
    param($wordToComplete, $commandAst, $cursorPosition)
    [Console]::InputEncoding = [Console]::OutputEncoding = $OutputEncoding = [System.Text.Utf8Encoding]::new()
    $Local:word = $wordToComplete.Replace('"', '""')
    $Local:ast = $commandAst.ToString().Replace('"', '""')
    tom complete --word="$Local:word" --commandline="$Local:ast" --position=$cursorPosition
        | ForEach-Object {
            $completionText, $listText, $toolTip = $_ -split ';', 3
            $listText ??= $completionText
            $toolTip ??= $completionText
            [System.Management.Automation.CompletionResult]::new(
                $completionText, $listText, 'ParameterValue', $toolTip)
        }
}

)";

#ifdef __linux__
/*! TinyORM tom bash completion file content. */
inline const auto *const TomBashCompletionContent =
R"TTT(# bash completion for tom                                -*- shell-script -*-

# Provide file/dir completion for the following commands
__tom_filedir()
{
    if [[ ${1-} == @(make:migration|make:seeder) && ${2-} -eq 2 ]]; then
        _filedir
    fi
}

_tom()
{
    local cur prev words cword

    _init_completion || return

    COMP_WORDBREAKS=${COMP_WORDBREAKS//:}

    # No completion after the following options
    case $prev in
        --help | --version | -!(-*)[hV])
            return
            ;;
    esac

    # Count the number of arguments excluding options
    # Default is 1 and it only count arguments before current word!
    local args
    _count_args

    # Find currently active tom command
    local tom_command i
    for ((i = 1; i < ${#words[@]}; i++)); do
        if [[ ${words[i]} != -* ]]; then
            # Command found but is actually completing now
            if [[ $i -ne $cword ]]; then
                tom_command=${words[i]}
            fi
            break
        fi
    done

    # Accurate completion using the tom complete command
    if _have tom; then
        # Completion for positional arguments and for long and short options
        if [[ $args -eq 1 || $cur == -* ||
            (-v tom_command && $args -eq 2 && $tom_command == @(help|list|integrate)) ]];
        then
            COMPREPLY=($(compgen -W "$(tom complete --word="$cur" \
                --commandline="${words[*]}" --cword=$cword 2>/dev/null)" -- "$cur"))
        fi

        # Provide file/dir completion for the following commands
        __tom_filedir "$tom_command" $args

        return
    fi

    # Inaccurate completion if the tom command is not on the system path, it doesn't
    # provide all options
    commands='env help inspire integrate list migrate db:seed db:wipe
        make:migration make:seeder migrate:fresh migrate:install
        migrate:refresh migrate:reset migrate:rollback migrate:status'

    namespaces='global db make migrate namespaced all'

    common_options='--ansi --no-ansi --env= --help --no-interaction --quiet
        --version --verbose'

    shells='bash pwsh'

    # Complete positional arguments for tom commands
    if [[ -v tom_command && $args -eq 2 && $cur != -* ]]; then
        case $tom_command in
            integrate)
                COMPREPLY=($(compgen -W "$shells" -- "$cur"))
                return
                ;;
            help)
                COMPREPLY=($(compgen -W "$commands" -- "$cur"))
                return
                ;;
            list)
                COMPREPLY=($(compgen -W "$namespaces" -- "$cur"))
                return
                ;;
        esac
    fi

    # Complete long and short options
    if [[ $cur == -* ]]; then
        COMPREPLY=($(compgen -W "$common_options" -- "$cur"))
        return
    fi

    # Complete tom commands
    if [[ ! -v tom_command && $cur != -* ]]; then
        COMPREPLY=($(compgen -W "$commands" -- "$cur"))
    fi

    # Provide file/dir completion for the following commands
    __tom_filedir $tom_command $args

} &&
    complete -F _tom tom

# ex: filetype=sh

)TTT";
#endif

} // namespace Tom::Commands::Stubs

TINYORM_END_COMMON_NAMESPACE

#endif // TOM_COMMANDS_STUBS_INTEGRATESTUBS_HPP
