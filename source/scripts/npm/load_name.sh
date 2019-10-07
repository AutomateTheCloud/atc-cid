#!/bin/bash
###------------------------------------------------------------------------------------------------
# alias:   cid.npm.load_name
# script:  load_name.sh
# purpose: Loads Package Name from specified package.json file
# version: 1.0.0
###------------------------------------------------------------------------------------------------

###------------------------------------------------------------------------------------------------
## Config
declare -r SELF_IDENTITY="npm_load_name"
declare -r SELF_IDENTITY_H="NPM (Load Name from package.json)"
declare -a ARGUMENTS_DESCRIPTION=(
    '-f : <file>'
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
VARIABLE_NAME=""
TMP_ERROR_MSG=""
TMP_VALUE=""

###------------------------------------------------------------------------------------------------
## Main
# Process Arguments
while getopts "hVf:v:" OPTION; do
    case $OPTION in
        f) FILE_TARGET=$OPTARG;;
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
    TMP_ERROR_MSG="Target file not specified (-f)"
    log_error "- ${TMP_ERROR_MSG}"
    exit_logic $E_BAD_ARGS "${TMP_ERROR_MSG}"
fi
if(is_empty "${VARIABLE_NAME}"); then
    TMP_ERROR_MSG="Variable Name not specified (-v)"
    log_error "- ${TMP_ERROR_MSG}"
    exit_logic $E_BAD_ARGS "${TMP_ERROR_MSG}"
fi

if [ ! -f "${FILE_TARGET}" ]; then
    TMP_ERROR_MSG="Target file does not exist [${FILE_TARGET}]"
    log_error "- ${TMP_ERROR_MSG}"
    exit_logic $E_OBJECT_NOT_FOUND "${TMP_ERROR_MSG}"
fi

line_break
log_notice "${SELF_IDENTITY_H}: Started"
log "- Target File:       [${FILE_TARGET}]"
log "- Variable Name:     [${VARIABLE_NAME}]"
line_break

TMP_VALUE="$(cat ${FILE_TARGET} | grep '"name"' | head -1 | awk -F: '{ print $2 }' | sed 's/[",]//g' | tr -d '[:space:]')"
if(! is_empty "${TMP_VALUE}"); then
    log "- Package Name:      [${TMP_VALUE}]"
    echo "##vso[task.setvariable variable=${VARIABLE_NAME}]${TMP_VALUE}"
    echo "##vso[task.setvariable variable=${VARIABLE_NAME};isOutput=true]${TMP_VALUE}"
else
    TMP_ERROR_MSG="Package Name not found"
    log_error "- ${TMP_ERROR_MSG}"
    exit_logic $E_OBJECT_NOT_FOUND "${TMP_ERROR_MSG}"
fi

log_success "${SELF_IDENTITY_H}: Finished"
exit_logic 0
