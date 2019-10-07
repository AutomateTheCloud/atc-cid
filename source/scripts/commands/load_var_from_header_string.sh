#!/bin/bash
###------------------------------------------------------------------------------------------------
# alias:   cid.load_var_from_header_string
# script:  load_var_from_header_string.sh
# purpose: Loads Variable from Comment Header in specified file
# version: 1.0.0
###------------------------------------------------------------------------------------------------

###------------------------------------------------------------------------------------------------
## Config
declare -r SELF_IDENTITY="load_var_from_header_string"
declare -r SELF_IDENTITY_H="Load Variable from Header"
declare -a ARGUMENTS_DESCRIPTION=(
    '-f : <file>'
    '-s : <string_identifier>'
    '-v : <variable_name>'
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
FILE_TARGET=""
STRING_IDENTIFIER=""
VARIABLE_NAME=""
TMP_ERROR_MSG=""
TMP_VALUE=""

###------------------------------------------------------------------------------------------------
## Main
# Process Arguments
while getopts "hVf:s:v:" OPTION; do
    case $OPTION in
        f) FILE_TARGET=$OPTARG;;
        s) STRING_IDENTIFIER=$OPTARG;;
        v) VARIABLE_NAME=$OPTARG;;
        h) usage; exit 0;;
        V) echo "$(return_script_version "${0}")"; exit 0;;
        *) echo "ERROR: There is an error with one or more of the arguments"; usage; exit $E_BAD_ARGS;;
        ?) echo "ERROR: There is an error with one or more of the arguments"; usage; exit $E_BAD_ARGS;;
    esac
done
start_logic
log "${SELF_IDENTITY_H}: Started"

if(is_empty "${FILE_TARGET}"); then
    TMP_ERROR_MSG="Target file not specfied (-f)"
    log_error "- ${TMP_ERROR_MSG}"
    exit_logic $E_BAD_ARGS "${TMP_ERROR_MSG}"
fi
if(is_empty "${STRING_IDENTIFIER}"); then
    TMP_ERROR_MSG="String Identifer not specfied (-s)"
    log_error "- ${TMP_ERROR_MSG}"
    exit_logic $E_BAD_ARGS "${TMP_ERROR_MSG}"
fi
if(is_empty "${VARIABLE_NAME}"); then
    TMP_ERROR_MSG="Variable Name not specfied (-v)"
    log_error "- ${TMP_ERROR_MSG}"
    exit_logic $E_BAD_ARGS "${TMP_ERROR_MSG}"
fi

if [ ! -f "${FILE_TARGET}" ]; then
    TMP_ERROR_MSG="Target file does not exist [${FILE_TARGET}]"
    log_error "- ${TMP_ERROR_MSG}"
    exit_logic $E_OBJECT_NOT_FOUND "${TMP_ERROR_MSG}"
fi

line_break
log_notice "${SELF_IDENTITY_H}: Loading Variable from specified file"
log "- Target File:       [${FILE_TARGET}]"
log "- String Identifier: [${STRING_IDENTIFIER}]"
log "- Variable Name:     [${VARIABLE_NAME}]"

return_header_string "${STRING_IDENTIFIER}" "${FILE_TARGET}" TMP_VALUE
if(! is_empty "${TMP_VALUE}"); then
    log "- Value:             [${TMP_VALUE}]"
    echo "##vso[task.setvariable variable=${VARIABLE_NAME}]${TMP_VALUE}"
    echo "##vso[task.setvariable variable=${VARIABLE_NAME};isOutput=true]${TMP_VALUE}"
else
    log_warning "- No value for specified Target, skipping Variable Declaration"
fi

log_success "${SELF_IDENTITY_H}: Finished"
exit_logic 0
