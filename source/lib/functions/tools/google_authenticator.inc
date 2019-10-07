###--------------------------------------------------------------------------------
# script:  google_authenticator.inc
# purpose: Collection of functions related to Google Authenticator
# version: 1.0.0
#
# function list:
#          - ga_generate_token
#          - ga_generate_user_config
#          - ga_get_user_token
#          - ga_load_from_file
#          - ga_put_user_token
###--------------------------------------------------------------------------------
REQUIRED_EXECUTABLES+=()

###--------------------------------------------------------------------------------
# Variables
GOOGLE_AUTHENTICATOR_REQUIRED=(
    'username'
    'secret_key'
    'qr_code_url'
)
GOOGLE_AUTHENTICATOR_USERNAME=""
GOOGLE_AUTHENTICATOR_SECRET_KEY=""
GOOGLE_AUTHENTICATOR_QR_CODE_URL=""

GOOGLE_AUTHENTICATOR_RATE_LIMIT="3 30"
GOOGLE_AUTHENTICATOR_WINDOW_SIZE="7"

SSM_PARAMETER_SECURITY_BASE="/security"
SSM_PARAMETER_SECURITY_USERS_BASE="${SSM_PARAMETER_SECURITY_BASE}/users"

SSM_KEYNAME_GOOGLE_AUTHENTICATOR="google_authenticator_token"

###--------------------------------------------------------------------------------
## FUNCTION: ga_generate_token()
## - Generates Google Authenticator Token for specified user and outputs details to specified Key-Value file
## - Arguments
##   - $1: File to dump key-value data to
##   - $2: Username
##   - $3: Identifier (example: [SSH: Amazon])
function ga_generate_token() {
    local FUNCTION_DESCRIPTION="Google Authenticator (Generate Token)"
    local TMP_FILE_GA_KEYVALUE_OUTPUT="${1}"
    local TMP_USERNAME="${2}"
    local TMP_IDENTIFIER="${3}"
    local TMP_FILE_GA_EXECUTION_OUTPUT=""
    local TMP_FILE_GA_EXECUTION_ERROR=""
    local TMP_FILE_GA_CONFIG=""

    local TMP_SECRET_KEY=""
    local TMP_QR_CODE_URL=""

    local RETURNVAL=""

    if(is_empty "${TMP_FILE_GA_KEYVALUE_OUTPUT}"); then
        log_error "${FUNCTION_DESCRIPTION}: Google Authenticator Key-Value Output file not specified"
        return $E_BAD_ARGS
    fi
    if [ ! -f "${TMP_FILE_GA_KEYVALUE_OUTPUT}" ]; then
        log_error "${FUNCTION_DESCRIPTION}: Google Authenticator Key-Value Output file does not exist [${TMP_FILE_GA_KEYVALUE_OUTPUT}]"
        return $E_OBJECT_NOT_FOUND
    fi
    if(is_empty "${TMP_USERNAME}"); then
        log_error "${FUNCTION_DESCRIPTION}: Username not specified"
        return $E_BAD_ARGS
    fi
    if(is_empty "${TMP_IDENTIFIER}"); then
        log_error "${FUNCTION_DESCRIPTION}: Identifier not specified"
        return $E_BAD_ARGS
    fi

    generate_temp_file DISCARDABLE_VARIABLE "ga execution output file"
    TMP_FILE_GA_EXECUTION_OUTPUT="${DISCARDABLE_VARIABLE}"
    generate_temp_file DISCARDABLE_VARIABLE "ga execution error file"
    TMP_FILE_GA_EXECUTION_ERROR="${DISCARDABLE_VARIABLE}"
    generate_temp_file DISCARDABLE_VARIABLE "ga config file"
    TMP_FILE_GA_CONFIG="${DISCARDABLE_VARIABLE}"

    log_notice "${FUNCTION_DESCRIPTION}: Generating token"
    log "- Username:   [${TMP_USERNAME}]"
    log "- Identifier: [${TMP_IDENTIFIER}]"

    $(which google-authenticator) -f -d -t -i "${TMP_IDENTIFIER} (${TMP_USERNAME})" -l "${TMP_IDENTIFIER} (${TMP_USERNAME})" -u -S 30 -w 7 -e 1 -s ${TMP_FILE_GA_CONFIG} >${TMP_FILE_GA_EXECUTION_OUTPUT} 2>${TMP_FILE_GA_EXECUTION_ERROR}
    RETURNVAL="$?"
    if [ ${RETURNVAL} -ne 0 ]; then
        log_add_from_file_content "${TMP_FILE_GA_EXECUTION_OUTPUT}" "${FUNCTION_DESCRIPTION}: Output"
        log_add_from_file_content "${TMP_FILE_GA_EXECUTION_ERROR}" "${FUNCTION_DESCRIPTION}: Error"
        log_error "- Failed to create Google Authenticator Token, aborting operation"
        return $E_GOOGLE_AUTHENTICATOR_FAILURE
    fi

    TMP_SECRET_KEY="$(cat ${TMP_FILE_GA_EXECUTION_OUTPUT} | grep 'new secret key' | awk -F: '{print $2}' | sed -e 's/^ *//g' -e 's/ *$//g')"
    TMP_QR_CODE_URL="$(cat ${TMP_FILE_GA_EXECUTION_OUTPUT} | grep 'https://www.google.com' | sed -e 's/^ *//g' -e 's/ *$//g')"

    if(is_empty "${TMP_SECRET_KEY}"); then
        log_error "${FUNCTION_DESCRIPTION}: failed to retrieve Secret Key"
        return $E_GOOGLE_AUTHENTICATOR_FAILURE
    fi
    if(is_empty "${TMP_QR_CODE_URL}"); then
        log_error "${FUNCTION_DESCRIPTION}: failed to retrieve QR Code URL"
        return $E_GOOGLE_AUTHENTICATOR_FAILURE
    fi
    log "${FUNCTION_DESCRIPTION}: Token successfully generated"
    log "- Secret Key:  [${TMP_SECRET_KEY}]"
    log "- QR Code URL: [${TMP_QR_CODE_URL}]"
cat > ${TMP_FILE_GA_KEYVALUE_OUTPUT} << ZZEOF
[username]=${TMP_USERNAME}
[secret_key]=${TMP_SECRET_KEY}
[qr_code_url]=${TMP_QR_CODE_URL}
ZZEOF

    return 0
}

###------------------------------------------------------------------------------------------------
## FUNCTION: ga_generate_user_config()
## - Generates Google Authenticator Config for specified user
## - Arguments
##   - $1: Username
##   - $2: AWS Region
function ga_generate_user_config() {
    local FUNCTION_DESCRIPTION="Google Authenticator (create config)"
    local TMP_USERNAME="${1}"
    local TMP_AWS_REGION="${2}"
    local TMP_FILE_GA="/home/${TMP_USERNAME}/.google_authenticator"
    local TMP_GA_TOKEN=""
    log_notice "${FUNCTION_DESCRIPTION}: started"
    log "- Username: [${TMP_USERNAME}]"
    ga_get_user_token TMP_GA_TOKEN "${TMP_USERNAME}" "${TMP_AWS_REGION}"
    if (is_empty "${TMP_GA_TOKEN}"); then
        log_warning "- Failed to retrieve GA token for specified user, skipping GA configuration generation"
        return 0
    fi
cat << ZZ_EOF > ${TMP_FILE_GA}
${TMP_GA_TOKEN}
"RATE_LIMIT ${GOOGLE_AUTHENTICATOR_RATE_LIMIT}
" WINDOW_SIZE ${GOOGLE_AUTHENTICATOR_WINDOW_SIZE}
" DISALLOW_REUSE
" TOTP_AUTH
ZZ_EOF
    $(which chmod) 0400 ${TMP_FILE_GA} >/dev/null 2>&1
    $(which chown) ${TMP_USERNAME}:${TMP_USERNAME} ${TMP_FILE_GA} >/dev/null 2>&1
    return 0
}

###------------------------------------------------------------------------------------------------
## FUNCTION: ga_get_user_token()
## - Retrieves Google Authenticator Token for specified user from SSM Stored Parameters
## - Arguments
##   - $1: Variable name to pass info to (optional)
##   - $2: Username
##   - $3: AWS Region
function ga_get_user_token() {
    local __RESULTVAR="${1}"
    local TMP_USERNAME="$(sanitize_username "${2}")"
    local TMP_AWS_REGION="${3}"
    local AWS_REGION_STRING=""
    if(! is_empty "${TMP_AWS_REGION}"); then
        AWS_REGION_STRING="--region ${TMP_AWS_REGION}"
    fi
    local TMP_STRING_GA_TOKEN=""

    TMP_STRING_GA_TOKEN="$($(which aws) ${AWS_REGION_STRING} ssm get-parameter --name "${SSM_PARAMETER_SECURITY_USERS_BASE}/${TMP_USERNAME}/${SSM_KEYNAME_GOOGLE_AUTHENTICATOR}" --with-decryption --query "Parameter.[Value]" --output text 2>/dev/null)"
    sleep 0.25
    if (is_empty "${__RESULTVAR}"); then
        printf "${TMP_STRING_GA_TOKEN}" | sed -e 's/^ *//g' -e 's/ *$//g'
    else
        eval $__RESULTVAR="'$(echo "$(trim "${TMP_STRING_GA_TOKEN}")" | sed -e 's/^ *//g' -e 's/ *$//g')'"
    fi
}

###--------------------------------------------------------------------------------
## FUNCTION: ga_load_from_file()
## - Arguments
##   - $1: File to read values from
function ga_load_from_file() {
    local FUNCTION_DESCRIPTION="Google Authenticator (Load)"
    local TMP_FILE_GA_KEYVALUE="${1}"

    local RETURNVAL=""

    if [ ! -f "${TMP_FILE_GA_KEYVALUE}" ]; then
        log_error "${FUNCTION_DESCRIPTION}: Google Authenticator Key-Value file does not exist [${TMP_FILE_GA_KEYVALUE}]"
        return $E_OBJECT_NOT_FOUND
    fi

    log_notice "${FUNCTION_DESCRIPTION}: loading GA related variables"
    load_array_key_values_from_file "GOOGLE_AUTHENTICATOR_REQUIRED[@]" "${TMP_FILE_GA_KEYVALUE}" "GOOGLE_AUTHENTICATOR"
    verify_array_key_values "GOOGLE_AUTHENTICATOR_REQUIRED[@]" "GOOGLE_AUTHENTICATOR"
    RETURNVAL="$?"; if [ ${RETURNVAL} -ne 0 ]; then return $RETURNVAL; fi

    return 0
}

###------------------------------------------------------------------------------------------------
## FUNCTION: ga_put_user_token()
## - Puts Google Authenticator Token for specified user in SSM Stored Parameters (Secrets)
## - Arguments
##   - $1: Username
##   - $2: Google Authenticator Token
##   - $3: AWS Region
function ga_put_user_token() {
    local FUNCTION_DESCRIPTION="Google Authenticator (Secrets - Put User Token)"
    local TMP_USERNAME="$(sanitize_username "${1}")"
    local TMP_GA_TOKEN="${2}"
    local TMP_AWS_REGION="${3}"

    local TMP_AWS_ACCOUNT_NUMBER=""
    local TMP_ARN_KMS_KEY=""

    local RETURNVAL=""

    aws_metadata_account_id_from_cli TMP_AWS_ACCOUNT_NUMBER

    if(is_empty "${TMP_USERNAME}"); then
        log_error "${FUNCTION_DESCRIPTION}: Username not specified"
        return $E_BAD_ARGS
    fi
    if(is_empty "${TMP_GA_TOKEN}"); then
        log_error "${FUNCTION_DESCRIPTION}: Google Authenticator Token not specified"
        return $E_BAD_ARGS
    fi
    if(is_empty "${TMP_AWS_REGION}"); then
        log_error "${FUNCTION_DESCRIPTION}: AWS Region not specified"
        return $E_BAD_ARGS
    fi
    if(is_empty "${TMP_AWS_ACCOUNT_NUMBER}"); then
        log_error "${FUNCTION_DESCRIPTION}: AWS Account Number lookup failed"
        return $E_BAD_ARGS
    fi

    TMP_ARN_KMS_KEY="arn:aws:kms:${TMP_AWS_REGION}:${TMP_AWS_ACCOUNT_NUMBER}:alias/security-${TMP_AWS_REGION}"

    log_notice "${FUNCTION_DESCRIPTION}: started"
    log "- Username:         [${TMP_USERNAME}]"
    log "- GA Token:         [${TMP_GA_TOKEN}]"
    log "- Region:           [${TMP_AWS_REGION}]"

    parameter_put "${SSM_PARAMETER_SECURITY_USERS_BASE}/${TMP_USERNAME}/${SSM_KEYNAME_GOOGLE_AUTHENTICATOR}" "${TMP_GA_TOKEN}" "${TMP_ARN_KMS_KEY}" "Google Authenticator token for User: ${TMP_USERNAME}" yes "${TMP_AWS_REGION}"
    RETURNVAL="$?"
    if [ ${RETURNVAL} -ne 0 ]; then
        log_error "- Failed to put Google Authenticator Token for user"
        return $RETURNVAL
    fi

    log "${FUNCTION_DESCRIPTION}: successfully put Google Authenticator Token for user"
    return 0
}
