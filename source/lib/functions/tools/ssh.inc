#--------------------------------------------------------------------------------
# script:  ssh.inc
# purpose: Collection of functions related to SSH
# version: 1.0.0
#
# function list:
#          - prepare_ssh_response_file_for_log
#          - use_ssh
#--------------------------------------------------------------------------------
REQUIRED_EXECUTABLES+=('ssh' 'timeout')

###------------------------------------------------------------------------------------------------
# Variables
SSH_DEFAULT_RETRY_COUNT=5
SSH_DEFAULT_TIMEOUT=15
SSH_DEFAULT_RETRY_TIMER_MAX_SEC=15
SSH_DEFAULT_OPERATION_TIMEOUT=60

###--------------------------------------------------------------------------------
## FUNCTION: prepare_ssh_response_file_for_log()
## - Cleans up the response file to remove entries that we dont need or want
## - Arguments
##   - $1: Response file
function prepare_ssh_response_file_for_log() {
    local RESPONSE_FILE="${1}"
    if [ -f ${RESPONSE_FILE} ]; then
        sed -i '/^Warning: Permanently added/d' "${RESPONSE_FILE}"
        sed -i '/^unicode_start skipped/d' "${RESPONSE_FILE}"
    fi
}

###--------------------------------------------------------------------------------
## FUNCTION: use_ssh()
# - Wrapper function for making SSH calls
# - Performs retries if enabled and appropriate
# - Attempts to catch all SSH / timeout specific return codes and attempts to handle the return code appropriately
# - Returns exit code of the whatever the remote command returned
## - Arguments
##   - $1: Remote IP
##   - $2: Command to execute
##   - $3: Response file (optional, if not supplied, response output will be sent to a temporary file)
##   - $4: Log Response (optional, defaults to yes if not supplied. Useful for preventing log spam, yes/no)
##   - $5: Identity file (optional)
##   - $6: SSH retry count (optional, defaults to SSH_DEFAULT_RETRY_COUNT)
##   - $7: SSH Timeout (optional, defaults to SSH_DEFAULT_TIMEOUT)
##   - $8: SSH Retry Timer max value (optional, defaults to SSH_DEFAULT_RETRY_TIMER_MAX_SEC)
##   - $9: SSH Operation timeout (optional, defaults to SSH_DEFAULT_OPERATION_TIMEOUT), used to kill the script if it runs for too long
##   - $10: Suppress command display in the logs (optional, yes/no)
function use_ssh() {
    local REMOTE_IP="${1}"
    local COMMAND="${2}"
    local RESPONSE_FILE="${3}"
    if(is_empty "${RESPONSE_FILE}"); then
        generate_temp_file DISCARDABLE_VARIABLE
        RESPONSE_FILE="${DISCARDABLE_VARIABLE}"
    fi
    local LOG_RESPONSE="${4}"
    if(is_empty "${LOG_RESPONSE}"); then
        LOG_RESPONSE=yes
    fi
    local IDENTITY_FILE="${5}"
    if(! is_empty "${IDENTITY_FILE}"); then
        if [ ! -f ${IDENTITY_FILE} ]; then
            log_warning "SSH: Specified identity file does not exist, attempting to connect without it (${IDENTITY_FILE})"
            IDENTITY_FILE=""
        fi
    fi
    local SSH_RETRY_COUNT="${6}"
    if(is_empty "${SSH_RETRY_COUNT}"); then
        SSH_RETRY_COUNT=${SSH_DEFAULT_RETRY_COUNT}
    fi
    local SSH_TIMEOUT="${7}"
    if(is_empty "${SSH_TIMEOUT}"); then
        SSH_TIMEOUT=${SSH_DEFAULT_TIMEOUT}
    fi
    local SSH_RETRY_TIMER_MAX_SEC="${8}"
    if(is_empty "${SSH_RETRY_TIMER_MAX_SEC}"); then
        SSH_RETRY_TIMER_MAX_SEC=${SSH_DEFAULT_RETRY_TIMER_MAX_SEC}
    fi
    local SSH_OPERATION_TIMEOUT="${9}"
    if(is_empty "${SSH_OPERATION_TIMEOUT}"); then
        SSH_OPERATION_TIMEOUT=${SSH_DEFAULT_OPERATION_TIMEOUT}
    fi
    local SSH_SUPPRESS_COMMAND_DISPLAY="${10}"
    local COMMAND_DISPLAY=""
    if option_enabled SSH_SUPPRESS_COMMAND_DISPLAY; then
        COMMAND_DISPLAY=""
    else
        COMMAND_DISPLAY="set -x;"
    fi
    local RUN=1
    local COUNTER=1
    local RETURNVAL=""
    while [ ${RUN} == 1 ]; do
        if [ ${COUNTER} -gt ${SSH_RETRY_COUNT} ]; then
            log_error "SSH: retry count exceeded, aborting operation"
            return $E_SSH_CONNECTION_FAILURE
        fi
        if(is_empty "${IDENTITY_FILE}"); then
            log "SSH: Connecting [Attempt::${COUNTER} / IP::${REMOTE_IP} / Command::${COMMAND}]"
            $(which timeout) ${SSH_OPERATION_TIMEOUT} $(which ssh) -o UserKnownHostsFile=/dev/null -o PasswordAuthentication=no -o StrictHostKeyChecking=no -o ConnectTimeout=${SSH_TIMEOUT} -o ServerAliveInterval=5 -o LogLevel=Error ${REMOTE_IP} "${COMMAND_DISPLAY}${COMMAND}" &> ${RESPONSE_FILE}
        else
            # Using identity file
            log "SSH: Connecting [Attempt::${COUNTER} / IP::${REMOTE_IP} / Identity_File::${IDENTITY_FILE} / Command::${COMMAND}]"
            $(which timeout) ${SSH_OPERATION_TIMEOUT} $(which ssh) -o UserKnownHostsFile=/dev/null -o PasswordAuthentication=no -o StrictHostKeyChecking=no -o ConnectTimeout=${SSH_TIMEOUT} -o ServerAliveInterval=5 -o LogLevel=Error -i "${IDENTITY_FILE}" ${REMOTE_IP} "${COMMAND_DISPLAY}${COMMAND}" &> ${RESPONSE_FILE}
        fi
        RETURNVAL="$?"
        if option_enabled LOG_RESPONSE; then
            prepare_ssh_response_file_for_log "${RESPONSE_FILE}"
            log_add_from_file_content "${RESPONSE_FILE}" "SSH: Connection output [Attempt::${COUNTER}]"
        else
            log "SSH: Connection output saved: ${RESPONSE_FILE} [Attempt::${COUNTER}]"
        fi
        case $RETURNVAL in
            0)   RUN=0; log "SSH: connection succeeded, remote command returned 0 [OK]";;
            124)  log_warning "SSH: returned 124 [E_OPERATION_TIMEOUT - Requested operation timed out.]. Retrying"; call_sleep_random ${SSH_RETRY_TIMER_MAX_SEC};;
            255)  log_warning "SSH: returned 255 [E_SSH_CONNECTION_FAILURE - An error occurred while processing request via SSH session.]. Retrying"; call_sleep_random ${SSH_RETRY_TIMER_MAX_SEC};;
            *)   log_error "SSH: connection succeeded, however the remote command returned $RETURNVAL [$(lookup_exit_code $RETURNVAL)]"; return $RETURNVAL;;
            ?)   log_error "SSH: connection succeeded, however the remote command returned $RETURNVAL [$(lookup_exit_code $RETURNVAL)]"; return $RETURNVAL;;
        esac
        COUNTER=$((${COUNTER}+1))
    done
    return 0
}
