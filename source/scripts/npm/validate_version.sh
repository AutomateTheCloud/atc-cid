#!/bin/bash
###------------------------------------------------------------------------------------------------
# alias:   cid.npm.validate_version
# script:  validate_version.sh
# purpose: Validates Package Version to see if the current version has been published to the Repository
# version: 1.0.0
###------------------------------------------------------------------------------------------------

###------------------------------------------------------------------------------------------------
## Config
declare -r SELF_IDENTITY="validate_version"
declare -r SELF_IDENTITY_H="NPM (Validate Version)"
declare -a ARGUMENTS_DESCRIPTION=(
    '-p : <package_name>'
    '-v : <package_version>'
    '-d : <working_directory>'
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
PACKAGE_NAME=""
PACKAGE_VERSION=""
DIRECTORY_WORKING=""
TMP_ERROR_MSG=""

###------------------------------------------------------------------------------------------------
## Main
# Process Arguments
while getopts "hVp:v:d:" OPTION; do
    case $OPTION in
        p) PACKAGE_NAME=$OPTARG;;
        v) PACKAGE_VERSION=$OPTARG;;
        d) DIRECTORY_WORKING=$OPTARG;;
        h) usage; exit 0;;
        V) echo "$(return_script_version "${0}")"; exit 0;;
        *) echo "ERROR: There is an error with one or more of the arguments"; usage; exit $E_BAD_ARGS;;
        ?) echo "ERROR: There is an error with one or more of the arguments"; usage; exit $E_BAD_ARGS;;
    esac
done
start_logic
log "${SELF_IDENTITY_H}: Started"

npm_does_version_exist "${PACKAGE_NAME}" "${PACKAGE_VERSION}" "${DIRECTORY_WORKING}"
RETURNVAL="$?"; if [ ${RETURNVAL} -eq 0 ]; then
    TMP_ERROR_MSG="Production package has already been deployed and exists in the registry. Please increment Version value and try again."
    log_error "- ${TMP_ERROR_MSG}"
    exit_logic $E_NPM_VERSION_VALIDATION_FAILURE "${TMP_ERROR_MSG}"
fi

log_success "${SELF_IDENTITY_H}: Finished"
exit_logic 0
