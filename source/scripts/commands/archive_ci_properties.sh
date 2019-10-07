#!/bin/bash
###------------------------------------------------------------------------------------------------
# alias:   cid.archive_ci_properties
# script:  archive_ci_properties.sh
# purpose: Archives CI Properties file to Workspace Artifact Directory (if file exists)
# version: 1.0.0
###------------------------------------------------------------------------------------------------

###------------------------------------------------------------------------------------------------
## Config
declare -r SELF_IDENTITY="archive_ci_properties"
declare -r SELF_IDENTITY_H="Archive (CI Properties)"
declare -a ARGUMENTS_DESCRIPTION=()

###------------------------------------------------------------------------------------------------
## Load Defaults
declare -r GLOBAL_CONFIG_FILE="${DIRECTORY_CID:-/opt/cid}/config/global.config"
source "${GLOBAL_CONFIG_FILE}" || exit 3

###------------------------------------------------------------------------------------------------
## Declare dependencies
REQUIRED_SOURCE_FILES+=(
    "${LIB_FUNCTIONS_CORE_CID}"
)

###------------------------------------------------------------------------------------------------
## Variables
RETURNVAL=""
TMP_ERROR_MSG=""

###------------------------------------------------------------------------------------------------
## Main
# Process Arguments
while getopts "hV" OPTION; do
    case $OPTION in
        h) usage; exit 0;;
        V) echo "$(return_script_version "${0}")"; exit 0;;
        *) echo "ERROR: There is an error with one or more of the arguments"; usage; exit $E_BAD_ARGS;;
        ?) echo "ERROR: There is an error with one or more of the arguments"; usage; exit $E_BAD_ARGS;;
    esac
done
start_logic
log "${SELF_IDENTITY_H}: Started"

FILE_CI_PROPERTIES="$($(which find) ${DIRECTORY_BASE_AGENT} -not -path '*/\.*' -name ci.properties | head -1 | sed -e 's/^ *//g' -e 's/ *$//g')"
if(! is_empty "${FILE_CI_PROPERTIES}"); then
    log "- CI Properties file found [${FILE_CI_PROPERTIES}], copying to artifacts location [${DIRECTORY_WORKSPACE_ARTIFACTS}/]"
    /bin/cp -f ${FILE_CI_PROPERTIES} ${DIRECTORY_WORKSPACE_ARTIFACTS}/ >/dev/null 2>&1
else
    log_warning "- No CI Properties file found, skipping"
fi

log_success "${SELF_IDENTITY_H}: Finished"
exit_logic 0
