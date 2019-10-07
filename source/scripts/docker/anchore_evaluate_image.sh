#!/bin/bash
###------------------------------------------------------------------------------------------------
# alias:   cid.docker.anchore_evaluate_image
# script:  anchore_evaluate_image.sh
# purpose: Performs Security Evaluation using Anchore for specified Docker Image
# version: 1.0.0
###------------------------------------------------------------------------------------------------

###------------------------------------------------------------------------------------------------
## Config
declare -r SELF_IDENTITY="evaluate_image"
declare -r SELF_IDENTITY_H="Anchore (Evaluate Image)"
declare -a ARGUMENTS_DESCRIPTION=(
    '-d <docker_image> : Docker Image'
    '-r <file_report> : Report File (optional)'
    '-f : Force Evaluation Pass (sets exit code to 0 even if there is a failure)'
    '-U <anchore_api_url> : Anchore API URL'
    '-u <anchore_user> : Anchore User'
    '-p <anchore_password> : Anchore Password'
)

###------------------------------------------------------------------------------------------------
## Load Defaults
declare -r GLOBAL_CONFIG_FILE="${DIRECTORY_CID:-/opt/cid}/config/global.config"
source "${GLOBAL_CONFIG_FILE}" || exit 3

###------------------------------------------------------------------------------------------------
## Declare dependencies
REQUIRED_SOURCE_FILES+=(
    "${LIB_FUNCTIONS_TOOLS_ANCHORE}"
)

###------------------------------------------------------------------------------------------------
## Variables
VARIABLES_REQUIRED=(
    'docker_image'
    'file_report'
    'anchore_cli_url'
    'anchore_cli_user'
    'anchore_cli_pass'
)

EVALUATION_STATUS=""

###------------------------------------------------------------------------------------------------
## Main
# Process Arguments
while getopts "hVd:r:U:u:p:f" OPTION; do
    case $OPTION in
        d) DOCKER_IMAGE=$OPTARG;;
        r) FILE_REPORT=$OPTARG;;
        U) ANCHORE_CLI_URL=$OPTARG;;
        u) ANCHORE_CLI_USER=$OPTARG;;
        p) ANCHORE_CLI_PASS=$OPTARG;;
        f) FORCE_EVALUATION_PASS=true;;
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

if(is_empty "${FILE_REPORT}"); then
    generate_temp_file FILE_REPORT "Anchore Image Report"
fi

if(is_empty "${FORCE_EVALUATION_PASS}"); then
    FORCE_EVALUATION_PASS=false
fi

verify_array_key_values "VARIABLES_REQUIRED[@]"
RETURNVAL="$?"; if [ ${RETURNVAL} -ne 0 ]; then exit_logic $E_BAD_ARGS; fi

line_break
log "- Docker Image:               [${DOCKER_IMAGE}]"
log "- Report File:                [${FILE_REPORT}]"
log "- Anchore API:                [${ANCHORE_CLI_URL}]"
if option_enabled FORCE_EVALUATION_PASS; then
    log "- Force Evaluation Pass:      [${FORCE_EVALUATION_PASS}]"
fi

## END Initialize
###------------------------------------------------------------------------------------------------

###------------------------------------------------------------------------------------------------
## START Execution
line_break
log_highlight "Execution"

log_notice "${SELF_IDENTITY_H}: Evaluating Image"
anchore_image_evaluate "${DOCKER_IMAGE}"
EVALUATION_STATUS="$?"

log_notice "${SELF_IDENTITY_H}: Generating Report"
anchore_report "${DOCKER_IMAGE}" "${FILE_REPORT}"

if [ ${EVALUATION_STATUS} -ne 0 ]; then
    if option_enabled FORCE_EVALUATION_PASS; then
        log_warning "${SELF_IDENTITY_H}: Docker Image FAILED evaluation (force evaluation PASS specified)"
    else
        log_error "${SELF_IDENTITY_H}: Docker Image FAILED evaluation"
        exit_logic $E_ANCHORE_EVALUATION_FAIL
    fi
fi

## END Execution
###------------------------------------------------------------------------------------------------

log_success "${SELF_IDENTITY_H}: Finished"
exit_logic 0
