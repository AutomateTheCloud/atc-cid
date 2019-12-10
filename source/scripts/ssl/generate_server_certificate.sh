#!/bin/bash
###------------------------------------------------------------------------------------------------
# alias:   cid.ssl.generate_server_certificate
# script:  generate_server_certificate.sh
# purpose: SSL - Generate Server Certificate
# version: 1.0.0
###------------------------------------------------------------------------------------------------

###------------------------------------------------------------------------------------------------
## Config
declare -r SELF_IDENTITY="generate_server_certificate"
declare -r SELF_IDENTITY_H="SSL (Generate Server Certificate)"
declare -a ARGUMENTS_DESCRIPTION=(
    '-e <directory> : SSL Export Directory'
    '-d <domain_name> : Domain Name'
    '-D <days> : Days Valid'
    '-o <org_name> : Organization Name'
    '-K <file> : Root CA Key'
    '-C <file> : Root CA Certificate'
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
    'file_root_ca_key'
    'file_root_ca_crt'
)
DIRECTORY_SSL_EXPORT_BASE=""
DIRECTORY_SSL_EXPORT=""
FILE_SSL_KEY=""
FILE_SSL_CSR=""
FILE_SSL_CRT=""
FILE_ROOT_CA_KEY=""
FILE_ROOT_CA_CRT=""
SSL_DOMAIN_NAME=""
SSL_DAYS_VALID="365"
SSL_SUBJECT_ORGANIZATION=""

TMP_FILE_LOG=""

###------------------------------------------------------------------------------------------------
## Main
# Process Arguments
while getopts "hVe:d:D:o:K:C:" OPTION; do
    case $OPTION in
        e) DIRECTORY_SSL_EXPORT_BASE=$OPTARG;;
        d) SSL_DOMAIN_NAME=$OPTARG;;
        D) SSL_DAYS_VALID=$OPTARG;;
        o) SSL_SUBJECT_ORGANIZATION=$OPTARG;;
        K) FILE_ROOT_CA_KEY=$OPTARG;;
        C) FILE_ROOT_CA_CRT=$OPTARG;;
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
if [ ! -f "${FILE_ROOT_CA_KEY}" ]; then
    TMP_ERROR_MSG="Root CA Key does not exist [${FILE_ROOT_CA_KEY}]"
    log_error "- ${TMP_ERROR_MSG}"
    exit_logic $E_OBJECT_NOT_FOUND "${TMP_ERROR_MSG}"
fi
if [ ! -f "${FILE_ROOT_CA_CRT}" ]; then
    TMP_ERROR_MSG="Root CA Certificate does not exist [${FILE_ROOT_CA_CRT}]"
    log_error "- ${TMP_ERROR_MSG}"
    exit_logic $E_OBJECT_NOT_FOUND "${TMP_ERROR_MSG}"
fi

DIRECTORY_SSL_EXPORT="${DIRECTORY_SSL_EXPORT_BASE}/${SSL_DOMAIN_NAME}"
FILE_SSL_KEY="${DIRECTORY_SSL_EXPORT}/${SSL_DOMAIN_NAME}.key"
FILE_SSL_CSR="${DIRECTORY_SSL_EXPORT}/${SSL_DOMAIN_NAME}.csr"
FILE_SSL_CRT="${DIRECTORY_SSL_EXPORT}/${SSL_DOMAIN_NAME}.crt"

line_break
log "- Domain:                [${SSL_DOMAIN_NAME}]"
log "- Days Valid:            [${SSL_DAYS_VALID}]"
log "- Organization:          [${SSL_SUBJECT_ORGANIZATION}]"
log "- File (key):            [${FILE_SSL_KEY}]"
log "- File (csr):            [${FILE_SSL_CSR}]"
log "- File (certificate):    [${FILE_SSL_CRT}]"
log "- Root CA (key):         [${FILE_ROOT_CA_KEY}]"
log "- Root CA (certificate): [${FILE_ROOT_CA_CRT}]"
## END Initialize
###------------------------------------------------------------------------------------------------

###------------------------------------------------------------------------------------------------
## START Execution
line_break
log_highlight "Execution"

$(which mkdir) -p ${DIRECTORY_SSL_EXPORT} 2>/dev/null

log_notice "${SELF_IDENTITY_H}: Generating Key"
log "- [${FILE_SSL_KEY}]"
> ${TMP_FILE_LOG}
$(which openssl) genrsa -out ${FILE_SSL_KEY} 2048 >${TMP_FILE_LOG} 2>&1
RETURNVAL="$?"
if [ ${RETURNVAL} -ne 0 ]; then
    TMP_ERROR_MSG="Failed to Generate Key [openssl returned: ${RETURNVAL}]"
    log_error "- ${TMP_ERROR_MSG}"
    log_add_from_file "${TMP_FILE_LOG}" "${SELF_IDENTITY_H}: Error"
    exit_logic $E_OPENSSL_FAILURE
fi

log_notice "${SELF_IDENTITY_H}: Generating CSR"
log "- [${FILE_SSL_CSR}]"
> ${TMP_FILE_LOG}
$(which openssl) req -new -sha256 -key ${FILE_SSL_KEY} -subj "/O=M${SSL_SUBJECT_ORGANIZATION}/CN=${SSL_DOMAIN_NAME}" -out ${FILE_SSL_CSR} >${TMP_FILE_LOG} 2>&1
RETURNVAL="$?"
if [ ${RETURNVAL} -ne 0 ]; then
    TMP_ERROR_MSG="Failed to Generate CSR [openssl returned: ${RETURNVAL}]"
    log_error "- ${TMP_ERROR_MSG}"
    log_add_from_file "${TMP_FILE_LOG}" "${SELF_IDENTITY_H}: Error"
    exit_logic $E_OPENSSL_FAILURE
fi

log_notice "${SELF_IDENTITY_H}: Generating Certificate"
log "- [${FILE_SSL_CRT}]"
> ${TMP_FILE_LOG}
$(which openssl) x509 -req -in ${FILE_SSL_CSR} -CA ${FILE_ROOT_CA_CRT} -CAkey ${FILE_ROOT_CA_KEY} -CAcreateserial -out ${FILE_SSL_CRT} -days ${SSL_DAYS_VALID} -sha256 >${TMP_FILE_LOG} 2>&1
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
