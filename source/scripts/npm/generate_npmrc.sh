#!/bin/bash
###------------------------------------------------------------------------------------------------
# alias:   cid.npm.generate_npmrc
# script:  generate_npmrc.sh
# purpose: Generates Temporary Npmrc file
# version: 1.0.0
###------------------------------------------------------------------------------------------------

###------------------------------------------------------------------------------------------------
## Config
declare -r SELF_IDENTITY="generate_npmrc"
declare -r SELF_IDENTITY_H="NPM (Generate Npmrc)"
declare -a ARGUMENTS_DESCRIPTION=(
    '-r : <registry_url>'
    '-d : <directory>'
    '-f : <filename_override> (optional, if not specified, defaults to ".npmrc")'
)

###------------------------------------------------------------------------------------------------
## Load Defaults
declare -r GLOBAL_CONFIG_FILE="${DIRECTORY_CID:-/opt/cid}/config/global.config"
source "${GLOBAL_CONFIG_FILE}" || exit 3

###------------------------------------------------------------------------------------------------
## Declare dependencies
REQUIRED_SOURCE_FILES+=("${LIB_FUNCTIONS_TOOLS_NPM}")

###------------------------------------------------------------------------------------------------
## Variables
REGISTRY_URL=""
DIRECTORY_NPMRC=""
FILENAME_NPMRC=""
TMP_ERROR_MSG=""

###------------------------------------------------------------------------------------------------
## Main
# Process Arguments
while getopts "hVr:d:f:" OPTION; do
    case $OPTION in
        r) REGISTRY_URL=$OPTARG;;
        d) DIRECTORY_NPMRC=$OPTARG;;
        f) FILENAME_NPMRC=$OPTARG;;
        h) usage; exit 0;;
        V) echo "$(return_script_version "${0}")"; exit 0;;
        *) echo "ERROR: There is an error with one or more of the arguments"; usage; exit $E_BAD_ARGS;;
        ?) echo "ERROR: There is an error with one or more of the arguments"; usage; exit $E_BAD_ARGS;;
    esac
done
start_logic
log "${SELF_IDENTITY_H}: Started"

npm_generate_npmrc "${REGISTRY_URL}" "${DIRECTORY_NPMRC}" "${FILENAME_NPMRC}"
RETURNVAL="$?"; if [ ${RETURNVAL} -ne 0 ]; then exit_logic $RETURNVAL; fi

log_success "${SELF_IDENTITY_H}: Finished"
exit_logic 0
