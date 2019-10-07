###------------------------------------------------------------------------------------------------
# script:  mail.inc
# purpose: Collection of functions related to mail
# version: 1.0.0
#
# function list:
#          - send_mail
###------------------------------------------------------------------------------------------------
REQUIRED_EXECUTABLES+=('mail')

###------------------------------------------------------------------------------------------------
## FUNCTION: send_mail()
## - Sends email
## - Arguments
##   - $1: Recipients
##   - $2: Subject
##   - $3: Message
##   - $4: Message File (optional, if message specified)
##   - $5: Attachment File (optional)
##   - $6: Sender Address (optional)
function send_mail() {
    local FUNCTION_DESCRIPTION="Mail (Send)"
    local CMD_SM_FILE_ATTACHMENT=""
    local CMD_SM_SENDER=""
    local SM_RECIPIENT="${1}"
    if(is_empty "${SM_RECIPIENT}"); then
        log_error "${FUNCTION_DESCRIPTION}: Recipient not specified"
        return $E_BAD_ARGS
    fi
    local SM_SUBJECT="${2}"
    if(is_empty "${SM_SUBJECT}"); then
        log_error "${FUNCTION_DESCRIPTION}: Subject not specified"
        return $E_BAD_ARGS
    fi
    local SM_MESSAGE="${3}"
    local SM_MESSAGE_FILE="${4}"
    local SM_FILE_ATTACHMENT="${5}"
    local SM_SENDER="${6}"

    generate_temp_file DISCARDABLE_VARIABLE "email message body"
    local TMP_MESSAGE_BODY="${DISCARDABLE_VARIABLE}"
    generate_temp_file DISCARDABLE_VARIABLE "mail execution log"
    local TMP_MAIL_EXECUTION_LOG="${DISCARDABLE_VARIABLE}"

    if(! is_empty "${SM_MESSAGE}"); then
cat << ZZEOF > ${TMP_MESSAGE_BODY}
${SM_MESSAGE}

ZZEOF
    fi
    if(! is_empty "${SM_MESSAGE_FILE}"); then
cat << ZZEOF > ${TMP_MESSAGE_BODY}
$(cat ${SM_MESSAGE_FILE})
ZZEOF
    fi
    log "${FUNCTION_DESCRIPTION}: Sending email"
    log "- Recipient:  [${SM_RECIPIENT}]"
    log "- Subject:    [${SM_SUBJECT}]"
    if(! is_empty "${SM_SENDER}"); then
        CMD_SM_SENDER="-r ${SM_SENDER}"
        log "- Sender:     [${SM_SENDER}]"
    fi
    if(! is_empty "${SM_FILE_ATTACHMENT}"); then
        CMD_SM_FILE_ATTACHMENT="-a ${SM_FILE_ATTACHMENT}"
        log "- Attachment: [${SM_FILE_ATTACHMENT}]"
    fi
    log_add_from_file "${TMP_MESSAGE_BODY}" "message body"

    $(which mail) ${CMD_SM_SENDER} ${CMD_SM_FILE_ATTACHMENT} -s "${SM_SUBJECT}" ${SM_RECIPIENT} < ${TMP_MESSAGE_BODY} > ${TMP_MAIL_EXECUTION_LOG} 2>&1
    RETURNVAL="$?"
    if [ ${RETURNVAL} -ne 0 ]; then
        TMP_ERROR_MSG="Failed to send email [returned::${RETURNVAL}]"
        log_error "- ${TMP_ERROR_MSG}"
        log_add_from_file "${TMP_MAIL_EXECUTION_LOG}" "output of mail operation"
        return $E_MAIL_FAILURE
    fi
    log_success "${FUNCTION_DESCRIPTION}: Message sent"
    return 0
}
