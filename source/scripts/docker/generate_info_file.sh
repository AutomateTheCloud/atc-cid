#!/bin/bash
###------------------------------------------------------------------------------------------------
# alias:   cid.docker.generate_info_file
# script:  generate_info_file.sh
# purpose: Generates Docker Info file for use in CI Pipelines
# version: 1.0.0
###------------------------------------------------------------------------------------------------

###------------------------------------------------------------------------------------------------
## Config
declare -r SELF_IDENTITY="generate_info_file"
declare -r SELF_IDENTITY_H="Docker (Generate Info File)"
declare -a ARGUMENTS_DESCRIPTION=(
    '-I <docker_info_file> : Docker Info file'
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
    'registry_url'            # Passed as Global Variable
    'organization'            # Passed as Global Variable
    'image_name'              # Passed as Global Variable
    'image_tag'               # Passed as Global Variable
)

FILE_DOCKER_INFO=""

###------------------------------------------------------------------------------------------------
## Main
# Process Arguments
while getopts "hVI:" OPTION; do
    case $OPTION in
        I) FILE_DOCKER_INFO=$OPTARG;;
        h) usage; exit 0;;
        V) echo "$(return_script_version "${0}")"; exit 0;;
        *) echo "ERROR: There is an error with one or more of the arguments"; usage; exit $E_BAD_ARGS;;
        ?) echo "ERROR: There is an error with one or more of the arguments"; usage; exit $E_BAD_ARGS;;
    esac
done
start_logic
log "${SELF_IDENTITY_H}: Started"

if(is_empty "${FILE_DOCKER_INFO}"); then
    TMP_ERROR_MSG="Docker Info File not specfied (-I)"
    log_error "- ${TMP_ERROR_MSG}"
    exit_logic $E_BAD_ARGS "${TMP_ERROR_MSG}"
fi

verify_array_key_values "VARIABLES_REQUIRED[@]" "DOCKER"
RETURNVAL="$?"; if [ ${RETURNVAL} -ne 0 ]; then exit_logic $E_BAD_ARGS; fi

line_break
log "- Info File:     [${FILE_DOCKER_INFO}]"
line_break

if ! $(which touch) "${FILE_DOCKER_INFO}"  > /dev/null 2>&1; then
    TMP_ERROR_MSG="Cannot write to specified Output file [${FILE_DOCKER_INFO}]"
    log_error "${SELF_IDENTITY_H}: ${TMP_ERROR_MSG}"
    exit_logic $E_OBJECT_FAILED_TO_CREATE "${TMP_ERROR_MSG}"
fi

log_notice "${SELF_IDENTITY_H}: Generating Docker Info file"
cat > ${FILE_DOCKER_INFO} << ZZEOF
docker.registry_url=${DOCKER_REGISTRY_URL}
docker.organization=${DOCKER_ORGANIZATION}
docker.image.tag=${DOCKER_IMAGE_TAG}
docker.image.name=${DOCKER_IMAGE_NAME}
docker.image.full=${DOCKER_REGISTRY_URL}/${DOCKER_ORGANIZATION}/${DOCKER_IMAGE_NAME}:${DOCKER_IMAGE_TAG}
docker.image=${DOCKER_ORGANIZATION}/${DOCKER_IMAGE_NAME}:${DOCKER_IMAGE_TAG}
ZZEOF

log_add_from_file "${FILE_DOCKER_INFO}" "Docker Info file"

log_success "${SELF_IDENTITY_H}: Finished"
exit_logic 0
