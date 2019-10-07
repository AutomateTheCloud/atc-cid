#!/bin/bash
###------------------------------------------------------------------------------------------------
# alias:   cid.artifact.nexus3_download_raw
# script:  nexus3_download_raw.sh
# purpose: Download Artifact from Nexus 3 (Raw Format)
# version: 1.0.0
###------------------------------------------------------------------------------------------------

###------------------------------------------------------------------------------------------------
## Config
declare -r SELF_IDENTITY="nexus3_download_raw"
declare -r SELF_IDENTITY_H="Artifact (Nexus 3 Download Raw)"
declare -a ARGUMENTS_DESCRIPTION=(
    '-I <artifact_info_file> : Artifact Info file (optional)'
    '-r <artifact_repository> : Repo'
    '-g <artifact_group_id> : Group ID'
    '-i <artifact_id> : ID (Name)'
    '-v <artifact_version> : Version'
    '-f <artifact_filename> : Filename'
    '-c <artifact_md5_checksum> : MD5 Checksum (optional, if not specified, skips validation)'
    '-o <file_output> : Output File'
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
    'artifact_filename'
    'file_output'
)
VARIABLES_OPTIONAL=(
    'artifact_md5'
)
FILE_ARTIFACT_INFO=""
ARTIFACT_LINK=""
TMP_MD5=""
TMP_FILE_LOG=""

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
        f) ARTIFACT_FILENAME=$OPTARG;;
        c) ARTIFACT_MD5=$OPTARG;;
        o) FILE_OUTPUT=$OPTARG;;
        h) usage; exit 0;;
        V) echo "$(return_script_version "${0}")"; exit 0;;
        *) echo "ERROR: There is an error with one or more of the arguments"; usage; exit $E_BAD_ARGS;;
        ?) echo "ERROR: There is an error with one or more of the arguments"; usage; exit $E_BAD_ARGS;;
    esac
done
start_logic
log "${SELF_IDENTITY_H}: Started"

###------------------------------------------------------------------------------------------------
## START Initialize
line_break
log_highlight "Initialize"

generate_temp_file TMP_FILE_LOG "log output"

if(! is_empty "${FILE_ARTIFACT_INFO}"); then
    if [ ! -f "${FILE_ARTIFACT_INFO}" ]; then
        TMP_ERROR_MSG="Artifact Info file was specified but does not exist [${FILE_ARTIFACT_INFO}]"
        log_error "- ${TMP_ERROR_MSG}"
        exit_logic $E_OBJECT_NOT_FOUND "${TMP_ERROR_MSG}"
    fi
    log_notice "${SELF_IDENTITY_H}: Loading values from Artifact Info file [${FILE_ARTIFACT_INFO}]"
    load_array_properties_from_file "VARIABLES_REQUIRED[@]" "${FILE_ARTIFACT_INFO}"
    load_array_properties_from_file "VARIABLES_OPTIONAL[@]" "${FILE_ARTIFACT_INFO}"
fi

verify_array_key_values "VARIABLES_REQUIRED[@]"
RETURNVAL="$?"; if [ ${RETURNVAL} -ne 0 ]; then exit_logic $E_BAD_ARGS; fi

ARTIFACT_LINK="https://${REPOSITORY_URL}/repository/${ARTIFACT_REPOSITORY}/${ARTIFACT_GROUP_ID}/${ARTIFACT_ID}/${ARTIFACT_VERSION}/${ARTIFACT_FILENAME}"

line_break
log "- Base URL:    [https://${REPOSITORY_URL}/]"
log "- Repository:  [${ARTIFACT_REPOSITORY}]"
log "- Group ID:    [${ARTIFACT_GROUP_ID}]"
log "- Artifact ID: [${ARTIFACT_ID}]"
log "- Version:     [${ARTIFACT_VERSION}]"
log "- Filename:    [${ARTIFACT_FILENAME}]"
log "- File Output: [${FILE_OUTPUT}]"
line_break

log_notice "${SELF_IDENTITY_H}: Downloading Artifact"
log "- [${ARTIFACT_LINK}]"

$(which curl) -sSfLk --connect-timeout 30 --speed-time 30 --max-time 7200 -u "${REPOSITORY_USERNAME}:${REPOSITORY_PASSWORD}" -o "${FILE_OUTPUT}" "${ARTIFACT_LINK}" >${TMP_FILE_LOG} 2>&1
RETURNVAL="$?"
if [ ${RETURNVAL} -ne 0 ]; then
    TMP_ERROR_MSG="Failed to download file [curl returned: ${RETURNVAL}]"
    log_error "- ${TMP_ERROR_MSG}"
    log_add_from_file "${TMP_FILE_LOG}" "${SELF_IDENTITY_H}: Error"
    exit_logic $E_CURL_FAILURE
fi

if(! is_empty "${ARTIFACT_MD5}"); then
    log_notice "${SELF_IDENTITY_H}: Verifying Checksum"
    return_file_md5sum "${FILE_OUTPUT}" TMP_MD5
    log "- Target MD5 Checksum: [${ARTIFACT_MD5}]"
    log "- Actual MD5 Checksum: [${TMP_MD5}]"
    if [[ "ZZ_${ARTIFACT_MD5}" != "ZZ_${TMP_MD5}" ]]; then
        log_error "- Checksum Mismatch"
        exit_logic $E_CHECKSUM_MISMATCH
    fi
fi

log_success "${SELF_IDENTITY_H}: Finished"
exit_logic 0
