#!/bin/bash
###------------------------------------------------------------------------------------------------
# alias:   cid.npm.generate_prerelease_version
# script:  generate_prerelease_version.sh
# purpose: Generates Prerelease Version for NPM Package
# version: 1.0.0
###------------------------------------------------------------------------------------------------

###------------------------------------------------------------------------------------------------
## Config
declare -r SELF_IDENTITY="prerelease_version"
declare -r SELF_IDENTITY_H="NPM (Generate Prerelease Version)"
declare -a ARGUMENTS_DESCRIPTION=(
    '-t : <tag>'
    '-p : <prerelease_id> (optional, defaults to "SNAPSHOT")'
    '-d : <package_directory>'
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
VARIABLES_REQUIRED=(
    'release_candidate_tag'
    'prerelease_id'
    'directory_package'
)

PACKAGE_VERSION=""
RELEASE_CANDIDATE_VERSION=""
TMP_FILE_LOG=""
TMP_ERROR_MSG=""

###------------------------------------------------------------------------------------------------
## Main
# Process Arguments
while getopts "hVt:p:d:" OPTION; do
    case $OPTION in
        t) RELEASE_CANDIDATE_TAG=$OPTARG;;
        p) PRERELEASE_ID=$OPTARG;;
        d) DIRECTORY_PACKAGE=$OPTARG;;
        h) usage; exit 0;;
        V) echo "$(return_script_version "${0}")"; exit 0;;
        *) echo "ERROR: There is an error with one or more of the arguments"; usage; exit $E_BAD_ARGS;;
        ?) echo "ERROR: There is an error with one or more of the arguments"; usage; exit $E_BAD_ARGS;;
    esac
done
start_logic
log "${SELF_IDENTITY_H}: Started"

if(is_empty "${PRERELEASE_ID}"); then
    PRERELEASE_ID="SNAPSHOT"
fi
DIRECTORY_PACKAGE="$(echo "${DIRECTORY_PACKAGE}" | sed 's:/*$::')"

verify_array_key_values "VARIABLES_REQUIRED[@]"
RETURNVAL="$?"; if [ ${RETURNVAL} -ne 0 ]; then exit_logic $E_BAD_ARGS; fi

if [ ! -f "${DIRECTORY_PACKAGE}/package.json" ]; then
    TMP_ERROR_MSG="package.json file not found [${DIRECTORY_PACKAGE}/package.json]"
    log_error "- ${TMP_ERROR_MSG}"
    exit_logic $E_OBJECT_NOT_FOUND "${TMP_ERROR_MSG}"
fi

PACKAGE_VERSION="$(cat ${DIRECTORY_PACKAGE}/package.json | grep '"version"' | head -1 | awk -F: '{ print $2 }' | sed 's/[",]//g' | tr -d '[:space:]')"
if(is_empty "${PACKAGE_VERSION}"); then
    TMP_ERROR_MSG="Package Version not found"
    log_error "- ${TMP_ERROR_MSG}"
    exit_logic $E_OBJECT_NOT_FOUND "${TMP_ERROR_MSG}"
fi

RELEASE_CANDIDATE_VERSION="${PACKAGE_VERSION}-${PRERELEASE_ID}.${RELEASE_CANDIDATE_TAG}"

line_break
log "- Package Version:           [${PACKAGE_VERSION}]"
log "- Release Candidate Version: [${RELEASE_CANDIDATE_VERSION}]"
line_break

generate_temp_file TMP_FILE_LOG "log output"

cd "${DIRECTORY_PACKAGE}"
$(which npm) version "${RELEASE_CANDIDATE_VERSION}" >>${TMP_FILE_LOG} 2>&1
RETURNVAL="$?"
log_add_from_file "${TMP_FILE_LOG}" "${SELF_IDENTITY_H}: NPM Version Output"
if [ ${RETURNVAL} -ne 0 ]; then
    TMP_ERROR_MSG="Failed to generate Release Candidate version"
    log_error "- ${TMP_ERROR_MSG}"
    exit_logic $E_NPM_FAILURE "${TMP_ERROR_MSG}"
fi

log_success "${SELF_IDENTITY_H}: Finished"
exit_logic 0
