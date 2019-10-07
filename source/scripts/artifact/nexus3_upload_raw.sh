#!/bin/bash
###------------------------------------------------------------------------------------------------
# alias:   cid.artifact.nexus3_upload_raw
# script:  nexus3_upload_raw.sh
# purpose: Upload Artifact to Nexus 3 (Raw Format) and generates optional artifact.info file
# version: 1.0.0
###------------------------------------------------------------------------------------------------

###------------------------------------------------------------------------------------------------
## Config
declare -r SELF_IDENTITY="nexus3_upload_raw"
declare -r SELF_IDENTITY_H="Artifact (Nexus 3 Upload Raw)"
declare -a ARGUMENTS_DESCRIPTION=(
    '-I <artifact_info_file> : Artifact Info file (optional)'
    '-F <artifact_file_to_upload> : File to upload'
    '-r <artifact_repository> : Repo'
    '-g <artifact_group_id> : Group ID'
    '-i <artifact_id> : ID (Name)'
    '-v <artifact_version> : Version'
    '-f <artifact_filename> : Filename'
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
    'file_artifact'
)
FILE_ARTIFACT_INFO=""
ARTIFACT_LINK=""
TMP_MD5=""
TMP_SHA1=""
TMP_SHA256=""
TMP_FILE_LOG=""
TMP_FILE_MD5=""

###------------------------------------------------------------------------------------------------
## Main
# Process Arguments
while getopts "hVI:F:r:g:i:v:f:" OPTION; do
    case $OPTION in
        I) FILE_ARTIFACT_INFO=$OPTARG;;
        F) FILE_ARTIFACT=$OPTARG;;
        r) ARTIFACT_REPOSITORY=$OPTARG;;
        g) ARTIFACT_GROUP_ID=$OPTARG;;
        i) ARTIFACT_ID=$OPTARG;;
        v) ARTIFACT_VERSION=$OPTARG;;
        f) ARTIFACT_FILENAME=$OPTARG;;
        h) usage; exit 0;;
        V) echo "$(return_script_version "${0}")"; exit 0;;
        *) echo "ERROR: There is an error with one or more of the arguments"; usage; exit $E_BAD_ARGS;;
        ?) echo "ERROR: There is an error with one or more of the arguments"; usage; exit $E_BAD_ARGS;;
    esac
done
start_logic
log "${SELF_IDENTITY_H}: Started"

generate_temp_file TMP_FILE_LOG "log output"
generate_temp_file TMP_FILE_MD5 "checksum - MD5"

verify_array_key_values "VARIABLES_REQUIRED[@]"
RETURNVAL="$?"; if [ ${RETURNVAL} -ne 0 ]; then exit_logic $E_BAD_ARGS; fi

if [ ! -f "${FILE_ARTIFACT}" ]; then
    TMP_ERROR_MSG="Artifact file does not exist [${FILE_ARTIFACT}]"
    log_error "- ${TMP_ERROR_MSG}"
    exit_logic $E_OBJECT_NOT_FOUND "${TMP_ERROR_MSG}"
fi

ARTIFACT_LINK="https://${REPOSITORY_URL}/repository/${ARTIFACT_REPOSITORY}/${ARTIFACT_GROUP_ID}/${ARTIFACT_ID}/${ARTIFACT_VERSION}/${ARTIFACT_FILENAME}"
return_file_md5sum "${FILE_ARTIFACT}" TMP_MD5
return_file_sha1sum "${FILE_ARTIFACT}" TMP_SHA1
return_file_sha256sum "${FILE_ARTIFACT}" TMP_SHA256
echo -n "${TMP_MD5}" > ${TMP_FILE_MD5}

line_break
log "- File:        [${FILE_ARTIFACT}]"
log "- Base URL:    [https://${REPOSITORY_URL}/]"
log "- Repository:  [${ARTIFACT_REPOSITORY}]"
log "- Group ID:    [${ARTIFACT_GROUP_ID}]"
log "- Artifact ID: [${ARTIFACT_ID}]"
log "- Version:     [${ARTIFACT_VERSION}]"
log "- Filename:    [${ARTIFACT_FILENAME}]"
log "- MD5:         [${TMP_MD5}]"
log "- SHA1:        [${TMP_SHA1}]"
log "- SHA256:      [${TMP_SHA256}]"
line_break

log_notice "${SELF_IDENTITY_H}: Uploading Artifact"
log "- [${ARTIFACT_LINK}]"
>${TMP_FILE_LOG}
$(which curl) -sSfLk --connect-timeout 30 --speed-time 30 --max-time 7200 -u "${REPOSITORY_USERNAME}:${REPOSITORY_PASSWORD}" --upload-file "${FILE_ARTIFACT}" "${ARTIFACT_LINK}" >${TMP_FILE_LOG} 2>&1
RETURNVAL="$?"
if [ ${RETURNVAL} -ne 0 ]; then
    TMP_ERROR_MSG="Failed to upload file [curl returned: ${RETURNVAL}]"
    log_error "- ${TMP_ERROR_MSG}"
    log_add_from_file "${TMP_FILE_LOG}" "${SELF_IDENTITY_H}: Error"
    exit_logic $E_CURL_FAILURE
fi

log_notice "${SELF_IDENTITY_H}: Uploading Checksum [MD5]"
log "- [${ARTIFACT_LINK}.md5]"
>${TMP_FILE_LOG}
$(which curl) -sSfLk --connect-timeout 30 --speed-time 30 --max-time 7200 -u "${REPOSITORY_USERNAME}:${REPOSITORY_PASSWORD}" --upload-file "${TMP_FILE_MD5}" "${ARTIFACT_LINK}.md5" >${TMP_FILE_LOG} 2>&1
RETURNVAL="$?"
if [ ${RETURNVAL} -ne 0 ]; then
    TMP_ERROR_MSG="Failed to upload file [curl returned: ${RETURNVAL}]"
    log_error "- ${TMP_ERROR_MSG}"
    log_add_from_file "${TMP_FILE_LOG}" "${SELF_IDENTITY_H}: Error"
    exit_logic $E_CURL_FAILURE
fi

if(! is_empty "${FILE_ARTIFACT_INFO}"); then
    log_notice "${SELF_IDENTITY_H}: Generating Artifact Info file"
    log "- [${FILE_ARTIFACT_INFO}]"
cat > ${FILE_ARTIFACT_INFO} << ZZEOF
repository.url=${REPOSITORY_URL}
artifact.repository=${ARTIFACT_REPOSITORY}
artifact.group_id=${ARTIFACT_GROUP_ID}
artifact.id=${ARTIFACT_ID}
artifact.version=${ARTIFACT_VERSION}
artifact.filename=${ARTIFACT_FILENAME}
artifact.link=${ARTIFACT_LINK}
artifact.md5=${TMP_MD5}
artifact.sha1=${TMP_SHA1}
artifact.sha256=${TMP_SHA256}
ZZEOF
fi

log_success "${SELF_IDENTITY_H}: Finished"
exit_logic 0
