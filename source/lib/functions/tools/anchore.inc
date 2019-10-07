###------------------------------------------------------------------------------------------------
# script:  anchore.inc
# purpose: Collection of functions related to Anchore
# version: 1.0.0
#
# function list:
#          - anchore_image_analysis_check
#          - anchore_image_analysis_queue
#          - anchore_image_analysis_wait
#          - anchore_image_evaluate
#          - anchore_image_list
#          - anchore_image_validate
#          - anchore_report
#          - anchore_report_image_vulnerability
###------------------------------------------------------------------------------------------------
REQUIRED_EXECUTABLES+=('anchore-cli')

###------------------------------------------------------------------------------------------------
# Variables
ANCHORE_CLI_VARIABLES=(
    'anchore_cli_url'       # Passed as Global Variable
    'anchore_cli_user'      # Passed as Global Variable
    'anchore_cli_pass'      # Passed as Global Variable
)

###------------------------------------------------------------------------------------------------
# Process Variables
verify_array_key_values "ANCHORE_CLI_VARIABLES[@]"
RETURNVAL="$?"; if [ ${RETURNVAL} -ne 0 ]; then log_error "- This script requires credentials for Anchore CLI usage"; exit_logic $E_BAD_ARGS; fi

export ANCHORE_CLI_URL="${ANCHORE_CLI_URL}"
export ANCHORE_CLI_USER="${ANCHORE_CLI_USER}"
export ANCHORE_CLI_PASS="${ANCHORE_CLI_PASS}"

###--------------------------------------------------------------------------------
## FUNCTION: anchore_image_analysis_check()
## - Checks Analysis status of Docker Image from Anchore
## - This process may take some time. It does not report to STDOUT until wait cycle is complete
## - Arguments
##   - $1: Docker Image
function anchore_image_analysis_check() {
    local FUNCTION_DESCRIPTION="Anchore - Image (Check Analysis)"
    local TMP_DOCKER_IMAGE="${1}"

    local TMP_FILE_ANCHORE_OUTPUT=""

    if(is_empty "${TMP_DOCKER_IMAGE}"); then
        log_error "${FUNCTION_DESCRIPTION}: Docker Image not specified"
        return $E_BAD_ARGS
    fi

    generate_temp_file TMP_FILE_ANCHORE_OUTPUT "Anchore Output"

    log_notice "${FUNCTION_DESCRIPTION}: started"
    log "- Docker Image: [${TMP_DOCKER_IMAGE}]"
    log "- Anchore API:  [${ANCHORE_CLI_URL}]"

    $(which anchore-cli) evaluate check "${TMP_DOCKER_IMAGE}" >${TMP_FILE_ANCHORE_OUTPUT} 2>&1
    RETURNVAL="$?"
    log_add_from_file "${TMP_FILE_ANCHORE_OUTPUT}" "${FUNCTION_DESCRIPTION}: Operation output"
    if [ ${RETURNVAL} -ne 0 ]; then
        log_error "${FUNCTION_DESCRIPTION}: Analysis Status [FAIL]"
        return $E_ANCHORE_EVALUATION_FAIL
    fi

    log_notice "${FUNCTION_DESCRIPTION}: finished"
    return 0
}

###--------------------------------------------------------------------------------
## FUNCTION: anchore_image_analysis_queue()
## - Queue Docker Image for Anchore Analysis
## - This process may take some time. It does not report to STDOUT until wait cycle is complete
## - Arguments
##   - $1: Docker Image
function anchore_image_analysis_queue() {
    local FUNCTION_DESCRIPTION="Anchore - Image (Queue Analysis)"
    local TMP_DOCKER_IMAGE="${1}"

    local TMP_FILE_ANCHORE_OUTPUT=""
    local TMP_FILE_ANCHORE_ERROR=""

    if(is_empty "${TMP_DOCKER_IMAGE}"); then
        log_error "${FUNCTION_DESCRIPTION}: Docker Image not specified"
        return $E_BAD_ARGS
    fi

    generate_temp_file TMP_FILE_ANCHORE_OUTPUT "Anchore Output"
    generate_temp_file TMP_FILE_ANCHORE_ERROR "Anchore Error"

    log_notice "${FUNCTION_DESCRIPTION}: started"
    log "- Docker Image: [${TMP_DOCKER_IMAGE}]"
    log "- Anchore API:  [${ANCHORE_CLI_URL}]"

    $(which anchore-cli) image add "${TMP_DOCKER_IMAGE}" --force >${TMP_FILE_ANCHORE_OUTPUT} 2>${TMP_FILE_ANCHORE_ERROR}
    RETURNVAL="$?"
    log_add_from_file "${TMP_FILE_ANCHORE_OUTPUT}" "${FUNCTION_DESCRIPTION}: Operation output"
    if [ ${RETURNVAL} -ne 0 ]; then
        log_add_from_file "${TMP_FILE_ANCHORE_ERROR}" "${FUNCTION_DESCRIPTION}: Operation error"
        log_error "${FUNCTION_DESCRIPTION}: operation failed (anchore-cli_exitcode::${RETURNVAL}])"
        return $E_ANCHORE_FAILURE
    fi

    log_notice "${FUNCTION_DESCRIPTION}: finished"
    return 0
}

###--------------------------------------------------------------------------------
## FUNCTION: anchore_image_analysis_wait()
## - Wait for Docker Image Anchore Analysis to complete
## - Arguments
##   - $1: Docker Image
function anchore_image_analysis_wait() {
    local FUNCTION_DESCRIPTION="Anchore - Image (Wait for Analysis)"
    local TMP_DOCKER_IMAGE="${1}"

    local TMP_FILE_ANCHORE_OUTPUT=""
    local TMP_FILE_ANCHORE_ERROR=""

    if(is_empty "${TMP_DOCKER_IMAGE}"); then
        log_error "${FUNCTION_DESCRIPTION}: Docker Image not specified"
        return $E_BAD_ARGS
    fi

    generate_temp_file TMP_FILE_ANCHORE_OUTPUT "Anchore Output"
    generate_temp_file TMP_FILE_ANCHORE_ERROR "Anchore Error"

    log_notice "${FUNCTION_DESCRIPTION}: started"
    log "- Docker Image: [${TMP_DOCKER_IMAGE}]"
    log "- Anchore API:  [${ANCHORE_CLI_URL}]"

    $(which anchore-cli) image wait "${TMP_DOCKER_IMAGE}" >${TMP_FILE_ANCHORE_OUTPUT} 2>${TMP_FILE_ANCHORE_ERROR}
    RETURNVAL="$?"
    if [ ${RETURNVAL} -ne 0 ]; then
        log_add_from_file "${TMP_FILE_ANCHORE_ERROR}" "${FUNCTION_DESCRIPTION}: Operation error"
        log_error "${FUNCTION_DESCRIPTION}: operation failed (anchore-cli_exitcode::${RETURNVAL}])"
        return $E_ANCHORE_FAILURE
    fi

    log_notice "${FUNCTION_DESCRIPTION}: finished"
    return 0
}

###--------------------------------------------------------------------------------
## FUNCTION: anchore_image_evaluate()
## - Performs security evaluation of specified Docker Image in Anchore
## - Arguments
##   - $1: Docker Image
function anchore_image_evaluate() {
    local FUNCTION_DESCRIPTION="Anchore - Image (Evaluate)"
    local TMP_DOCKER_IMAGE="${1}"

    if(is_empty "${TMP_DOCKER_IMAGE}"); then
        log_error "${FUNCTION_DESCRIPTION}: Docker Image not specified"
        return $E_BAD_ARGS
    fi

    log_notice "${FUNCTION_DESCRIPTION}: started"
    log "- Docker Image: [${TMP_DOCKER_IMAGE}]"
    
    log_notice "${FUNCTION_DESCRIPTION}: started"
    
    anchore_image_analysis_queue "${TMP_DOCKER_IMAGE}"
    RETURNVAL="$?"; if [ ${RETURNVAL} -ne 0 ]; then return ${RETURNVAL}; fi
    
    call_sleep 30 "allowing Anchore time to warmup"
    
    anchore_image_analysis_wait "${TMP_DOCKER_IMAGE}"
    RETURNVAL="$?"; if [ ${RETURNVAL} -ne 0 ]; then return ${RETURNVAL}; fi
    
    call_sleep 1 "allowing Anchore time to cooldown"
    
    anchore_image_analysis_check "${TMP_DOCKER_IMAGE}"
    RETURNVAL="$?"; if [ ${RETURNVAL} -ne 0 ]; then return ${RETURNVAL}; fi

    log_notice "${FUNCTION_DESCRIPTION}: finished"
    return 0
}

###--------------------------------------------------------------------------------
## FUNCTION: anchore_image_list()
## - Lists Docker images loaded into Anchore
function anchore_image_list() {
    local FUNCTION_DESCRIPTION="Anchore - Image (List)"
    local TMP_FILE_ANCHORE_OUTPUT=""

    generate_temp_file TMP_FILE_ANCHORE_OUTPUT "Anchore Output"

    log_notice "${FUNCTION_DESCRIPTION}: Retrieving Image List"
    log "- Anchore API: [${ANCHORE_CLI_URL}]"

    $(which anchore-cli) image list >${TMP_FILE_ANCHORE_OUTPUT} 2>&1
    RETURNVAL="$?"
    log_add_from_file "${TMP_FILE_ANCHORE_OUTPUT}" "${FUNCTION_DESCRIPTION}: Operation output"
    if [ ${RETURNVAL} -ne 0 ]; then
        log_error "${FUNCTION_DESCRIPTION}: operation failed (anchore-cli_exitcode::${RETURNVAL}])"
        return $E_ANCHORE_FAILURE
    fi
    return 0
}

###--------------------------------------------------------------------------------
## FUNCTION: anchore_report()
## - Report Generation - Full Report
function anchore_report() {
    local FUNCTION_DESCRIPTION="Anchore - Report"
    local TMP_DOCKER_IMAGE="${1}"
    local TMP_FILE_REPORT="${2}"

    local TMP_FILE_REPORT_IMAGE_EVALUATION=""
    local TMP_FILE_REPORT_IMAGE_VULNERABILITY=""
    
    if(is_empty "${TMP_DOCKER_IMAGE}"); then
        log_error "${FUNCTION_DESCRIPTION}: Docker Image not specified"
        return $E_BAD_ARGS
    fi
    if(is_empty "${TMP_FILE_REPORT}"); then
        log_error "${FUNCTION_DESCRIPTION}: Report file not specified"
        return $E_BAD_ARGS
    fi
    if ! $(which touch) ${TMP_FILE_REPORT} >/dev/null 2>&1; then
        log_error "${FUNCTION_DESCRIPTION}: Report file cannot be created [${TMP_FILE_REPORT}]"
        return $E_OBJECT_FAILED_TO_CREATE
    fi

    generate_temp_file TMP_FILE_REPORT_IMAGE_EVALUATION "Anchore Image Evaluation Report"
    generate_temp_file TMP_FILE_REPORT_IMAGE_VULNERABILITY "Anchore Image Vulnerability Report"

    log_notice "${FUNCTION_DESCRIPTION}: Generating Report"
    log "- Docker Image: [${TMP_DOCKER_IMAGE}]"
    log "- Anchore API:  [${ANCHORE_CLI_URL}]"
    log "- Report File:  [${TMP_FILE_REPORT}]"
    
    anchore_report_image_evaluation "${TMP_DOCKER_IMAGE}" "${TMP_FILE_REPORT_IMAGE_EVALUATION}"
    RETURNVAL="$?"; if [ ${RETURNVAL} -ne 0 ]; then return ${RETURNVAL}; fi

    anchore_report_image_vulnerability "${TMP_DOCKER_IMAGE}" "${TMP_FILE_REPORT_IMAGE_VULNERABILITY}"
    RETURNVAL="$?"; if [ ${RETURNVAL} -ne 0 ]; then return ${RETURNVAL}; fi

    log_notice "${SELF_IDENTITY_H}: Generating Report"
cat > ${TMP_FILE_REPORT} << ZZEOF
Docker Image: [${DOCKER_IMAGE}]
Generated:    [$($(which date) +'%Y-%m-%d %H:%M:%S.%3N %Z')]

----------------------------------------------------------------------------------------------------------------------------------------------------------------
[Evaluation]
$(cat "${TMP_FILE_REPORT_IMAGE_EVALUATION}")

----------------------------------------------------------------------------------------------------------------------------------------------------------------
[Vulnerabilities]
$(cat "${TMP_FILE_REPORT_IMAGE_VULNERABILITY}")

ZZEOF

    log_add_from_file "${TMP_FILE_REPORT}" "${SELF_IDENTITY_H}: Report"
    return 0
}

###--------------------------------------------------------------------------------
## FUNCTION: anchore_report_image_evaluation()
## - Report Generation - Image Evaluation
## - Arguments
##   - $1: Docker Image
##   - $2: Output Report File
function anchore_report_image_evaluation() {
    local FUNCTION_DESCRIPTION="Anchore - Report (Image Evaluation)"
    local TMP_DOCKER_IMAGE="${1}"
    local TMP_FILE_REPORT="${2}"

    local TMP_FILE_ANCHORE_OUTPUT=""
    local TMP_FILE_ANCHORE_ERROR=""

    if(is_empty "${TMP_DOCKER_IMAGE}"); then
        log_error "${FUNCTION_DESCRIPTION}: Docker Image not specified"
        return $E_BAD_ARGS
    fi
    if(is_empty "${TMP_FILE_REPORT}"); then
        log_error "${FUNCTION_DESCRIPTION}: Report file not specified"
        return $E_BAD_ARGS
    fi
    if ! $(which touch) ${TMP_FILE_REPORT} >/dev/null 2>&1; then
        log_error "${FUNCTION_DESCRIPTION}: Report file cannot be created [${TMP_FILE_REPORT}]"
        return $E_OBJECT_FAILED_TO_CREATE
    fi

    generate_temp_file TMP_FILE_ANCHORE_OUTPUT "Anchore Output"
    generate_temp_file TMP_FILE_ANCHORE_ERROR "Anchore Error"

    log_notice "${FUNCTION_DESCRIPTION}: Retrieving Report"
    log "- Docker Image: [${TMP_DOCKER_IMAGE}]"
    log "- Anchore API:  [${ANCHORE_CLI_URL}]"
    log "- Report File:  [${TMP_FILE_REPORT}]"

    $(which anchore-cli) evaluate check "${TMP_DOCKER_IMAGE}" --detail >${TMP_FILE_ANCHORE_OUTPUT} 2>/dev/null

    cat ${TMP_FILE_ANCHORE_OUTPUT} > ${TMP_FILE_REPORT}
    return 0
}

###--------------------------------------------------------------------------------
## FUNCTION: anchore_report_image_vulnerability()
## - Report Generation - Image Vulnerability
function anchore_report_image_vulnerability() {
    local FUNCTION_DESCRIPTION="Anchore - Report (Image Vulnerability)"
    local TMP_DOCKER_IMAGE="${1}"
    local TMP_FILE_REPORT="${2}"

    local TMP_FILE_ANCHORE_OUTPUT=""
    local TMP_FILE_ANCHORE_ERROR=""

    if(is_empty "${TMP_DOCKER_IMAGE}"); then
        log_error "${FUNCTION_DESCRIPTION}: Docker Image not specified"
        return $E_BAD_ARGS
    fi
    if(is_empty "${TMP_FILE_REPORT}"); then
        log_error "${FUNCTION_DESCRIPTION}: Report file not specified"
        return $E_BAD_ARGS
    fi
    if ! $(which touch) ${TMP_FILE_REPORT} >/dev/null 2>&1; then
        log_error "${FUNCTION_DESCRIPTION}: Report file cannot be created [${TMP_FILE_REPORT}]"
        return $E_OBJECT_FAILED_TO_CREATE
    fi

    generate_temp_file TMP_FILE_ANCHORE_OUTPUT "Anchore Output"
    generate_temp_file TMP_FILE_ANCHORE_ERROR "Anchore Error"

    log_notice "${FUNCTION_DESCRIPTION}: Retrieving Report"
    log "- Docker Image: [${TMP_DOCKER_IMAGE}]"
    log "- Anchore API:  [${ANCHORE_CLI_URL}]"
    log "- Report File:  [${TMP_FILE_REPORT}]"

    $(which anchore-cli) image vuln "${TMP_DOCKER_IMAGE}" all >${TMP_FILE_ANCHORE_OUTPUT} 2>${TMP_FILE_ANCHORE_ERROR}
    RETURNVAL="$?"
    if [ ${RETURNVAL} -ne 0 ]; then
        log_add_from_file "${TMP_FILE_ANCHORE_ERROR}" "${FUNCTION_DESCRIPTION}: Operation error"
        log_error "${FUNCTION_DESCRIPTION}: operation failed (anchore-cli_exitcode::${RETURNVAL}])"
        return $E_ANCHORE_FAILURE
    fi

    cat ${TMP_FILE_ANCHORE_OUTPUT} > ${TMP_FILE_REPORT}
    return 0
}