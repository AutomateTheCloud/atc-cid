#!/bin/bash
###------------------------------------------------------------------------------------------------
# alias:   cid.ssl.generate_root_ca
# script:  generate_root_ca.sh
# purpose: SSL - Generate Root CA
# version: 1.0.0
###------------------------------------------------------------------------------------------------

###------------------------------------------------------------------------------------------------
## Config
declare -r SELF_IDENTITY="generate_root_ca"
declare -r SELF_IDENTITY_H="SSL (Generate Root CA)"
declare -a ARGUMENTS_DESCRIPTION=(
    '-e <directory> : SSL Export Directory'
    '-d <domain_name> : Domain Name'
    '-D <days> : Days Valid'
    '-o <org_name> : Organization Name'
)

###------------------------------------------------------------------------------------------------
## Load Defaults
declare -r GLOBAL_CONFIG_FILE="${DIRECTORY_CID:-/opt/cid}/config/global.config"
source "${GLOBAL_CONFIG_FILE}" || exit 3

###------------------------------------------------------------------------------------------------
## Declare dependencies
REQUIRED_SOURCE_FILES+=()
REQUIRED_EXECUTABLES+=('openssl')

###------------------------------------------------------------------------------------------------
## Variables
VARIABLES_REQUIRED=(
    'directory_ssl_export_base'
    'ssl_domain_name'
    'ssl_days_valid'
    'ssl_subject_organization'
)
DIRECTORY_SSL_EXPORT_BASE=""
DIRECTORY_SSL_EXPORT=""
FILE_ROOT_CA_KEY=""
FILE_ROOT_CA_CRT=""
SSL_DOMAIN_NAME=""
SSL_DAYS_VALID="365"
SSL_SUBJECT_ORGANIZATION=""

TMP_FILE_LOG=""

###------------------------------------------------------------------------------------------------
## Main
# Process Arguments
while getopts "hVe:d:D:o:" OPTION; do
    case $OPTION in
        e) DIRECTORY_SSL_EXPORT_BASE=$OPTARG;;
        d) SSL_DOMAIN_NAME=$OPTARG;;
        D) SSL_DAYS_VALID=$OPTARG;;
        o) SSL_SUBJECT_ORGANIZATION=$OPTARG;;
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

DIRECTORY_SSL_EXPORT_BASE="$(echo "${DIRECTORY_SSL_EXPORT_BASE}" | sed 's:/*$::')"

generate_temp_file TMP_FILE_LOG "log output"

verify_array_key_values "VARIABLES_REQUIRED[@]"
RETURNVAL="$?"; if [ ${RETURNVAL} -ne 0 ]; then exit_logic $E_BAD_ARGS; fi

if [ ! -d "${DIRECTORY_SSL_EXPORT_BASE}" ]; then
    TMP_ERROR_MSG="SSL Export Directory does not exist [${DIRECTORY_SSL_EXPORT_BASE}]"
    log_error "- ${TMP_ERROR_MSG}"
    exit_logic $E_OBJECT_NOT_FOUND "${TMP_ERROR_MSG}"
fi

DIRECTORY_SSL_EXPORT="${DIRECTORY_SSL_EXPORT_BASE}/rootCA_${SSL_DOMAIN_NAME}"
FILE_ROOT_CA_KEY="${DIRECTORY_SSL_EXPORT}/rootCA_${SSL_DOMAIN_NAME}.key"
FILE_ROOT_CA_CRT="${DIRECTORY_SSL_EXPORT}/rootCA_${SSL_DOMAIN_NAME}.crt"

line_break
log "- Domain:             [${SSL_DOMAIN_NAME}]"
log "- Days Valid:         [${SSL_DAYS_VALID}]"
log "- Organization:       [${SSL_SUBJECT_ORGANIZATION}]"
log "- File (key):         [${FILE_ROOT_CA_KEY}]"
log "- File (certificate): [${FILE_ROOT_CA_CRT}]"
## END Initialize
###------------------------------------------------------------------------------------------------

###------------------------------------------------------------------------------------------------
## START Execution
line_break
log_highlight "Execution"

$(which mkdir) -p ${DIRECTORY_SSL_EXPORT} 2>/dev/null

log_notice "${SELF_IDENTITY_H}: Generating Key"
log "- [${FILE_ROOT_CA_KEY}]"
> ${TMP_FILE_LOG}
$(which openssl) genrsa -out ${FILE_ROOT_CA_KEY} 4096 >${TMP_FILE_LOG} 2>&1
RETURNVAL="$?"
if [ ${RETURNVAL} -ne 0 ]; then
    TMP_ERROR_MSG="Failed to Generate Key [openssl returned: ${RETURNVAL}]"
    log_error "- ${TMP_ERROR_MSG}"
    log_add_from_file "${TMP_FILE_LOG}" "${SELF_IDENTITY_H}: Error"
    exit_logic $E_OPENSSL_FAILURE
fi

log_notice "${SELF_IDENTITY_H}: Generating Certificate"
log "- [${FILE_ROOT_CA_CRT}]"
> ${TMP_FILE_LOG}
$(which openssl) req -x509 -new -nodes -key ${FILE_ROOT_CA_KEY} -sha256 -days ${SSL_DAYS_VALID} -subj "/O=M${SSL_SUBJECT_ORGANIZATION}/CN=${SSL_DOMAIN_NAME}" -out ${FILE_ROOT_CA_CRT}  >${TMP_FILE_LOG} 2>&1
RETURNVAL="$?"
if [ ${RETURNVAL} -ne 0 ]; then
    TMP_ERROR_MSG="Failed to Generate Certificate [openssl returned: ${RETURNVAL}]"
    log_error "- ${TMP_ERROR_MSG}"
    log_add_from_file "${TMP_FILE_LOG}" "${SELF_IDENTITY_H}: Error"
    exit_logic $E_OPENSSL_FAILURE
fi
## END Execution
###------------------------------------------------------------------------------------------------

log_success "${SELF_IDENTITY_H}: Finished"
exit_logic 0
