#!/bin/bash
###------------------------------------------------------------------------------------------------
# alias:   cid.load_vars_from_file
# script:  load_vars_from_file.sh
# purpose: Loads Variables from specified Key Value file into Azure DevOps Pipeline
# version: 1.0.0
###------------------------------------------------------------------------------------------------

###------------------------------------------------------------------------------------------------
## Config
declare -r SELF_IDENTITY="load_vars_from_file"
declare -r SELF_IDENTITY_H="Load Variables from File"
declare -a ARGUMENTS_DESCRIPTION=(
    '-f : <key_value_file>'
)

###------------------------------------------------------------------------------------------------
## Load Defaults
declare -r GLOBAL_CONFIG_FILE="${DIRECTORY_CID:-/opt/cid}/config/global.config"
source "${GLOBAL_CONFIG_FILE}" || exit 3

###------------------------------------------------------------------------------------------------
## Declare dependencies
REQUIRED_SOURCE_FILES+=()

###------------------------------------------------------------------------------------------------
## Variables
FILE_KEY_VALUE=""
TMP_ALIAS=""
TMP_ERROR_MSG=""
TMP_KEY=""
TMP_VALUE=""
KEY_MAX_LENGTH=0

###------------------------------------------------------------------------------------------------
## Main
# Process Arguments
while getopts "hVf:" OPTION; do
    case $OPTION in
        f) FILE_KEY_VALUE=$OPTARG;;
        h) usage; exit 0;;
        V) echo "$(return_script_version "${0}")"; exit 0;;
        *) echo "ERROR: There is an error with one or more of the arguments"; usage; exit $E_BAD_ARGS;;
        ?) echo "ERROR: There is an error with one or more of the arguments"; usage; exit $E_BAD_ARGS;;
    esac
done
start_logic
log "${SELF_IDENTITY_H}: Started"

if(is_empty "${FILE_KEY_VALUE}"); then
    TMP_ERROR_MSG="Key Value file not specfied (-f)"
    log_error "- ${TMP_ERROR_MSG}"
    exit_logic $E_BAD_ARGS "${TMP_ERROR_MSG}"
fi

if [ ! -f "${FILE_KEY_VALUE}" ]; then
    TMP_ERROR_MSG="Key Value file does not exist [${FILE_KEY_VALUE}]"
    log_error "- ${TMP_ERROR_MSG}"
    exit_logic $E_OBJECT_NOT_FOUND "${TMP_ERROR_MSG}"
fi

line_break
log_notice "${SELF_IDENTITY_H}: Loading Variables from Key Value file [${FILE_KEY_VALUE}]"
OLD_IFS="${IFS}"; IFS=$'\n'
for CURRENT_LINE in $(cat "${FILE_KEY_VALUE}" 2>/dev/null | grep -v "^#\|^;" | sed -e 's/[ \t]*$//' -e '/^$/d'); do
    TMP_KEY="$(echo "${CURRENT_LINE}" | awk -F = '{print $1}' | sed -e 's/^ *//g' -e 's/ *$//g')"
    if(! is_empty "${TMP_KEY}"); then
        if [ ${#TMP_KEY} -gt $KEY_MAX_LENGTH ]; then
            KEY_MAX_LENGTH=${#TMP_KEY}
        fi
    fi
done
KEY_MAX_LENGTH=$((${KEY_MAX_LENGTH}+1))
for CURRENT_LINE in $(cat "${FILE_KEY_VALUE}" 2>/dev/null | grep -v "^#\|^;" | sed -e 's/[ \t]*$//' -e '/^$/d'); do
    TMP_KEY="$(echo "${CURRENT_LINE}" | awk -F = '{print $1}' | sed -e 's/^ *//g' -e 's/ *$//g')"
    TMP_VALUE="$(echo "${CURRENT_LINE}" | awk -F = '{print $2}' | sed -e 's/^ *//g' -e 's/ *$//g')"
    if(! is_empty "${TMP_KEY}"); then
        log "$(printf "%-1s %-${KEY_MAX_LENGTH}s %s" "-" "${TMP_KEY}:" "[${TMP_VALUE}]")"
        echo "##vso[task.setvariable variable=${TMP_KEY}]${TMP_VALUE}"
        echo "##vso[task.setvariable variable=${TMP_KEY};isOutput=true]${TMP_VALUE}"
    fi
done
IFS="${OLD_IFS}"
line_break

log_success "${SELF_IDENTITY_H}: Finished"
exit_logic 0
