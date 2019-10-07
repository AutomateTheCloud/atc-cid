#!/bin/bash
###------------------------------------------------------------------------------------------------
# alias:   cid.artifact.nexus3_validate_version
# script:  nexus3_validate_version.sh
# purpose: Validates Artifact Version from Nexus 3 (Raw Format)
# version: 1.0.0
###------------------------------------------------------------------------------------------------

###------------------------------------------------------------------------------------------------
## Config
declare -r SELF_IDENTITY="nexus3_validate_version"
declare -r SELF_IDENTITY_H="Artifact (Nexus 3 Validate Version)"
declare -a ARGUMENTS_DESCRIPTION=(
    '-I <artifact_info_file> : Artifact Info file (optional)'
    '-r <artifact_repository> : Repo'
    '-g <artifact_group_id> : Group ID'
    '-i <artifact_id> : ID (Name)'
    '-v <artifact_version> : Version'
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
    'repository_url'            # Passed as Global Variable
    'repository_username'       # Passed as Global Variable
    'repository_password'       # Passed as Global Variable
    'artifact_repository'
    'artifact_group_id'
    'artifact_id'
    'artifact_version'
)
FILE_ARTIFACT_INFO=""
TMP_FILE_LOG=""
TMP_STRING=""

###------------------------------------------------------------------------------------------------
## Main
# Process Arguments
while getopts "hVI:r:g:i:v:f:c:o:" OPTION; do
    case $OPTION in
        I) FILE_ARTIFACT_INFO=$OPTARG;;
        r) ARTIFACT_REPOSITORY=$OPTARG;;
        g) ARTIFACT_GROUP_ID=$OPTARG;;
        i) ARTIFACT_ID=$OPTARG;;
        v) ARTIFACT_VERSION=$OPTARG;;
        h) usage; exit 0;;
        V) echo "$(return_script_version "${0}")"; exit 0;;
        *) echo "ERROR: There is an error with one or more of the arguments"; usage; exit $E_BAD_ARGS;;
        ?) echo "ERROR: There is an error with one or more of the arguments"; usage; exit $E_BAD_ARGS;;
    esac
done
start_logic
log "${SELF_IDENTITY_H}: Started"

generate_temp_file TMP_FILE_LOG "log output"

if(! is_empty "${FILE_ARTIFACT_INFO}"); then
    if [ ! -f "${FILE_ARTIFACT_INFO}" ]; then
        log_error "- Artifact Info file was specified but does not exist [${FILE_ARTIFACT_INFO}]"
        exit_logic $E_OBJECT_NOT_FOUND
    fi
    log_notice "${SELF_IDENTITY_H}: Loading values from Artifact Info file [${FILE_ARTIFACT_INFO}]"
    load_array_properties_from_file "VARIABLES_REQUIRED[@]" "${FILE_ARTIFACT_INFO}"
fi

verify_array_key_values "VARIABLES_REQUIRED[@]"
RETURNVAL="$?"; if [ ${RETURNVAL} -ne 0 ]; then exit_logic $E_BAD_ARGS; fi

line_break
log "- Base URL:    [https://${REPOSITORY_URL}/]"
log "- Repository:  [${ARTIFACT_REPOSITORY}]"
log "- Group ID:    [${ARTIFACT_GROUP_ID}]"
log "- Artifact ID: [${ARTIFACT_ID}]"
log "- Version:     [${ARTIFACT_VERSION}]"
line_break

log_notice "${SELF_IDENTITY_H}: Validating Artifact Version"
$(which curl) -sSfLk --connect-timeout 30 --speed-time 30 --max-time 7200 -G -u "${REPOSITORY_USERNAME}:${REPOSITORY_PASSWORD}" --data-urlencode "repository=${ARTIFACT_REPOSITORY}" --data-urlencode "group=/${ARTIFACT_GROUP_ID}/${ARTIFACT_ID}/${ARTIFACT_VERSION}" https://${REPOSITORY_URL}/service/rest/v1/search >${TMP_FILE_LOG} 2>&1
RETURNVAL="$?"
if [ ${RETURNVAL} -ne 0 ]; then
    log_error "- Failed to Query Validation [curl returned: ${RETURNVAL}]"
    log_add_from_file "${TMP_FILE_LOG}" "${SELF_IDENTITY_H}: Error"
    exit_logic $E_CURL_FAILURE
fi

TMP_STRING="$(cat ${TMP_FILE_LOG} | grep '"downloadUrl" :' | head -1)"
if(! is_empty "${TMP_STRING}"); then
    log_error "- Artifact Version has already been deployed and exists in the registry. Please increment Version value and try again."
    exit_logic $E_ARTIFACT_VERSION_VALIDATION_FAILURE
fi

log "- Artifact Version does not exist"

log_success "${SELF_IDENTITY_H}: Finished"
exit_logic 0
