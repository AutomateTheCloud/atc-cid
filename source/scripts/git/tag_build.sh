#!/bin/bash
###------------------------------------------------------------------------------------------------
# alias:   cid.git.tag_build
# script:  tag_build.sh
# purpose: Tags Git Repository with specified Tag
# version: 1.0.0
###------------------------------------------------------------------------------------------------

###------------------------------------------------------------------------------------------------
## Config
declare -r SELF_IDENTITY="tag_build"
declare -r SELF_IDENTITY_H="Git (Tag Build)"
declare -a ARGUMENTS_DESCRIPTION=(
    '-t <tag> : Tag String'
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
VARIABLES_REQUIRED=(
    'build_sourcesdirectory'   # Passed as Global Variable
    'build_buildnumber'        # Passed as Global Variable
    'build_sourceversion'      # Passed as Global Variable
    'build_requestedfor'       # Passed as Global Variable
    'build_requestedforemail'  # Passed as Global Variable
    'system_accesstoken'       # Passed as Global Variable
    'string_tag'
)
TMP_FILE_LOG=""

###------------------------------------------------------------------------------------------------
## Main
# Process Arguments
while getopts "hVt:" OPTION; do
    case $OPTION in
        t) STRING_TAG=$OPTARG;;
        h) usage; exit 0;;
        V) echo "$(return_script_version "${0}")"; exit 0;;
        *) echo "ERROR: There is an error with one or more of the arguments"; usage; exit $E_BAD_ARGS;;
        ?) echo "ERROR: There is an error with one or more of the arguments"; usage; exit $E_BAD_ARGS;;
    esac
done
start_logic
log "${SELF_IDENTITY_H}: Started"

verify_array_key_values "VARIABLES_REQUIRED[@]"
RETURNVAL="$?"; if [ ${RETURNVAL} -ne 0 ]; then exit_logic $E_BAD_ARGS; fi

generate_temp_file TMP_FILE_LOG "log output"

line_break
log "- Tag:              [${STRING_TAG}]"
log "- Build:            [${BUILD_BUILDNUMBER}]"
log "- Source Directory: [${BUILD_SOURCESDIRECTORY}]"
log "- Source Version:   [${BUILD_SOURCEVERSION}]"
log "- Submitted By:     [${BUILD_REQUESTEDFOR} / ${BUILD_REQUESTEDFOREMAIL}]"
line_break

log_notice "${SELF_IDENTITY_H}: Tagging Git Commit"
cd ${BUILD_SOURCESDIRECTORY}
$(which git) config user.name "${BUILD_REQUESTEDFOR}" >/dev/null 2>&1
$(which git) config user.email "${BUILD_REQUESTEDFOREMAIL}" >/dev/null 2>&1
$(which git) -c http.extraheader="AUTHORIZATION: bearer ${SYSTEM_ACCESSTOKEN}" tag -fa ${STRING_TAG} ${BUILD_SOURCEVERSION} -m "${BUILD_BUILDNUMBER}" >${TMP_FILE_LOG} 2>&1
$(which git) -c http.extraheader="AUTHORIZATION: bearer ${SYSTEM_ACCESSTOKEN}" push origin ${STRING_TAG} -f >>${TMP_FILE_LOG} 2>&1
RETURNVAL="$?"
log_add_from_file "${TMP_FILE_LOG}" "${SELF_IDENTITY_H}: Git Tag Output"
if [ ${RETURNVAL} -ne 0 ]; then
    TMP_ERROR_MSG="Failed to Tag Git Commit"
    log_error "- ${TMP_ERROR_MSG}"
    exit_logic $E_GIT_FAULURE "${TMP_ERROR_MSG}"
fi

log_success "${SELF_IDENTITY_H}: Finished"
exit_logic 0
