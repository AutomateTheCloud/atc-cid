###------------------------------------------------------------------------------------------------
# script:  curl.inc
# purpose: Collection of functions related to curl
# version: 1.0.0
#
# function list:
#          - use_curl
###------------------------------------------------------------------------------------------------
REQUIRED_EXECUTABLES+=('curl')

###------------------------------------------------------------------------------------------------
# Variables
CURL_DEFAULT_LIMIT_RATE=0
CURL_DEFAULT_MAX_TIMEOUT=7200 # 2 hours
CURL_DEFAULT_CONNECT_TIMEOUT=30
CURL_DEFAULT_SPEED_TIMEOUT=30
CURL_DEFAULT_RETRY_COUNT=3
CURL_DEFAULT_RETRY_TIMER_MAX_SEC=30

###------------------------------------------------------------------------------------------------
## FUNCTION: use_curl()
## - wrapper function for making curl calls
## - performs retries if enabled and appropriate
## - attempts to catch all curl specific return codes and attempts to handle the return code appropriately"<max_timeout>"
## - Arguments
##   - $1: URL
##   - $2: Output File (optional, <filename>, if not specified, file/request will save to a temporary file)
##   - $3: Display Data File (optional, yes/no, defaults to no if not specified. Useful for when downloading files, as not to spam the log with binary data)
##   - $4: Post Data (optional)
##   - $5: Authentication - Username (optional, if not specified, no Authentication credentials will be sent)
##   - $6: Authentication - Password (optional)
##   - $7: Authentication - Client Certificate File (optional, <filename>, if not specified, no certificate will be sent)
##   - $8: Resume Download (optional, yes/no, defaults to yes if not specified)
##   - $9: Rate Limit (optional, <value, ie: 1k, 1m>, defaults to CURL_DEFAULT_LIMIT_RATE [0] if not specified)
##   - $10: Curl Speed Timeout (optional, defaults to CURL_DEFAULT_SPEED_TIMEOUT - this is used to prevent stalled downloads, ie: less than 1 byte/sec for SPEED_TIMEOUT seconds)
##   - $11: Curl Max Timeout (optional, defaults to CURL_DEFAULT_MAX_TIMEOUT - this is used to prevent ridiculously long downloads)
function use_curl() {
    local FUNCTION_DESCRIPTION="Curl"
    local TMP_URL="${1}"
    local TMP_FILE_OUTPUT="${2}"
    local TMP_DISPLAY_OUTPUT="${3}"
    local TMP_POST_DATA="${4}"
    local TMP_AUTH_USERNAME="${5}"
    local TMP_AUTH_PASSWORD="${6}"
    local TMP_AUTH_CLIENT_CERT_FILE="${7}"
    local TMP_RESUME_DOWNLOAD="${8}"
    local TMP_CURL_LIMIT_RATE="${9}"
    local TMP_CURL_SPEED_TIMEOUT="${10}"
    local TMP_CURL_MAX_TIMEOUT="${11}"

    local TMP_CURL_HOST="$(echo "${TMP_URL}" | awk -F[/:] '{print $4}')"

    local ARG_CURL_CONNECT_TIMEOUT=""
    local ARG_CURL_SPEED_TIME=""
    local ARG_CURL_MAX_TIME=""
    local ARG_CURL_RATE_LIMIT=""
    local ARG_CURL_AUTH_CLIENT_CERT=""
    local ARG_CURL_AUTH_CREDENTIALS=""
    local ARG_CURL_RESUME_DOWNLOAD=""

    local TMP_CURL_HTTP_CODE=""
    local TMP_CURL_SIZE_DOWNLOAD=""
    local TMP_CURL_SPEED_DOWNLOAD=""
    local TMP_CURL_SIZE_UPLOAD=""
    local TMP_CURL_SPEED_UPLOAD=""

    local FILE_TMP_CURL_RESPONSE=""
    local FILE_TMP_CURL_ERROR=""
    local FILE_TMP_CURL_AUTH_CREDENTIALS=""

    local RUN=1
    local COUNTER=0
    local RETRY_ENABLED=no
    local HAS_ERROR=no

    generate_temp_file FILE_TMP_CURL_RESPONSE "curl response log"
    generate_temp_file FILE_TMP_CURL_ERROR "curl error log"

    if(is_empty "${TMP_URL}"); then
        log_error "${FUNCTION_DESCRIPTION}: URL not specified]"
        return $E_BAD_ARGS
    fi
    if(is_empty "${TMP_FILE_OUTPUT}"); then
        generate_temp_file TMP_FILE_OUTPUT "curl output"
    fi
    if(is_empty "${TMP_DISPLAY_OUTPUT}"); then
        TMP_DISPLAY_OUTPUT=no
    fi
    if(! is_empty "${TMP_AUTH_CLIENT_CERT_FILE}"); then
        if [ ! -f "${TMP_AUTH_CLIENT_CERT_FILE}" ]; then
            log_error "${FUNCTION_DESCRIPTION}: Specified Client Certificate not found [${TMP_AUTH_CLIENT_CERT_FILE}]"
            return $E_OBJECT_NOT_FOUND
        fi
    fi
    if(is_empty "${TMP_RESUME_DOWNLOAD}"); then
        TMP_RESUME_DOWNLOAD=yes
    fi
    if(is_empty "${TMP_CURL_LIMIT_RATE}"); then
        TMP_CURL_LIMIT_RATE="${CURL_DEFAULT_LIMIT_RATE}"
    fi
    if(is_empty "${TMP_CURL_SPEED_TIMEOUT}"); then
        TMP_CURL_SPEED_TIMEOUT="${CURL_DEFAULT_SPEED_TIMEOUT}"
    fi
    if(is_empty "${TMP_CURL_MAX_TIMEOUT}"); then
        TMP_CURL_MAX_TIMEOUT="${CURL_DEFAULT_MAX_TIMEOUT}"
    fi

    log_notice "${FUNCTION_DESCRIPTION}: started"
    line_break
    log "- URL:              [${TMP_URL}]"
    log "- Output File:      [${TMP_FILE_OUTPUT}]"
    log "- Display Output    [${TMP_DISPLAY_OUTPUT}]"
    # Auth - User / Pass
    if(! is_empty "${TMP_AUTH_USERNAME}"); then
        log "- Auth Credentials: [${TMP_AUTH_USERNAME}:${TMP_AUTH_PASSWORD//?/*}]"
    fi
    # Auth - Client Cert
    if(! is_empty "${TMP_AUTH_CLIENT_CERT_FILE}"); then
        log "- Auth Client Cert: [${TMP_AUTH_CLIENT_CERT_FILE}]"
    fi
    log "- Resume Download   [${TMP_RESUME_DOWNLOAD}]"
    log "- Rate Limit:       [${TMP_CURL_LIMIT_RATE}]"
    log "- Speed Timeout:    [${TMP_CURL_SPEED_TIMEOUT}]"
    log "- Max Timeout:      [${TMP_CURL_MAX_TIMEOUT}]"
    line_break

    ARG_CURL_CONNECT_TIMEOUT="--connect-timeout ${CURL_DEFAULT_CONNECT_TIMEOUT}"
    ARG_CURL_SPEED_TIME="--speed-time ${TMP_CURL_SPEED_TIMEOUT}"
    ARG_CURL_MAX_TIME="--max-time ${TMP_CURL_MAX_TIMEOUT}"
    ARG_CURL_LIMIT_RATE="--limit-rate ${TMP_CURL_LIMIT_RATE}"

    if(! is_empty "${TMP_AUTH_USERNAME}"); then
        generate_temp_file FILE_TMP_CURL_AUTH_CREDENTIALS "curl auth credentials"
        echo "machine ${TMP_CURL_HOST} login ${TMP_AUTH_USERNAME} password ${TMP_AUTH_PASSWORD}" > ${FILE_TMP_CURL_AUTH_CREDENTIALS}
        ARG_CURL_AUTH_CREDENTIALS="--netrc-file ${FILE_TMP_CURL_AUTH_CREDENTIALS}"
    fi
    if(! is_empty "${TMP_AUTH_CLIENT_CERT_FILE}"); then
        ARG_CURL_AUTH_CLIENT_CERT="--cert-type pem --cert ${TMP_AUTH_CLIENT_CERT_FILE}"
    fi
    if option_enabled TMP_RESUME_DOWNLOAD; then
        ARG_CURL_RESUME_DOWNLOAD="-C -"
    fi

    while [ ${RUN} == 1 ]; do
        COUNTER=$((${COUNTER}+1))
        if [ ${COUNTER} -gt ${CURL_DEFAULT_RETRY_COUNT} ]; then
            log_error "${FUNCTION_DESCRIPTION}: retry count [${CURL_DEFAULT_RETRY_COUNT}] exceeded, aborting operation"
            return $E_CURL_FAILURE
        fi
        if option_enabled RETRY_ENABLED; then
            call_sleep_random ${CURL_DEFAULT_RETRY_TIMER_MAX_SEC}
        fi

        # Reset Temporary Variables for the current aws run
        HAS_ERROR=no
        RETURNVAL=""
        RETRY_ENABLED=no
        TMP_CURL_HTTP_CODE=""
        TMP_CURL_SIZE_DOWNLOAD=""
        TMP_CURL_SPEED_DOWNLOAD=""
        TMP_CURL_SIZE_UPLOAD=""
        TMP_CURL_SPEED_UPLOAD=""

        log "${FUNCTION_DESCRIPTION}: making call to URL (Attempt::${COUNTER} of ${CURL_DEFAULT_RETRY_COUNT})"
        $(which curl) -sSfLk -w "@${LIB_DEPENDENCIES_CURL_OUTPUT_FORMAT}" ${ARG_CURL_CONNECT_TIMEOUT} ${ARG_CURL_SPEED_TIME} ${ARG_CURL_MAX_TIME} ${ARG_CURL_LIMIT_RATE} ${ARG_CURL_AUTH_CREDENTIALS} ${ARG_CURL_AUTH_CLIENT_CERT} ${TMP_POST_DATA} -o ${TMP_FILE_OUTPUT} ${TMP_URL} >${FILE_TMP_CURL_RESPONSE} 2>${FILE_TMP_CURL_ERROR}
        RETURNVAL="$?"
        load_key_value_from_file TMP_CURL_HTTP_CODE "http_code" "${FILE_TMP_CURL_RESPONSE}"
        load_key_value_from_file TMP_CURL_SIZE_DOWNLOAD "size_download" "${FILE_TMP_CURL_RESPONSE}"
        load_key_value_from_file TMP_CURL_SPEED_DOWNLOAD "speed_download" "${FILE_TMP_CURL_RESPONSE}"
        load_key_value_from_file TMP_CURL_SIZE_UPLOAD "size_upload" "${FILE_TMP_CURL_RESPONSE}"
        load_key_value_from_file TMP_CURL_SPEED_UPLOAD "speed_upload" "${FILE_TMP_CURL_RESPONSE}"

        log "- Data saved:  [${TMP_FILE_OUTPUT}]"
        if option_enabled TMP_DISPLAY_OUTPUT; then
            log_add_from_file "${TMP_FILE_OUTPUT}" "Data Output"
        fi
        if [ "${TMP_CURL_SIZE_DOWNLOAD}" -gt 0 ]; then
            log "- Download size: [$(filesize_bytes_to_human_readable "${TMP_CURL_SIZE_DOWNLOAD}")] / Download speed: [$(filesize_bytes_to_human_readable "${TMP_CURL_SPEED_DOWNLOAD}") / sec]"
        fi
        if [ "${TMP_CURL_SIZE_UPLOAD}" -gt 0 ]; then
            log "- Upload size: [$(filesize_bytes_to_human_readable "${TMP_CURL_SIZE_UPLOAD}")] / Upload speed: [$(filesize_bytes_to_human_readable "${TMP_CURL_SPEED_UPLOAD}") / sec]"
        fi
        if(is_empty "${TMP_CURL_HTTP_CODE}"); then
            log_warning "- HTTP Status Code unknown [UNKNOWN]"
            HAS_ERROR=yes
        elif [ "${TMP_CURL_HTTP_CODE}" == "000" ]; then
            log_warning "- HTTP Status Code unknown [UNKNOWN]"
            HAS_ERROR=yes
        elif [ "${TMP_CURL_HTTP_CODE}" -ge 200 -a "${TMP_CURL_HTTP_CODE}" -le 299 ]; then
            log "- HTTP Status Code ${TMP_CURL_HTTP_CODE} [OK]"
        else
            log_warning "- HTTP Status Code ${TMP_CURL_HTTP_CODE} [NON_2XX_STATUS]"
            HAS_ERROR=yes
        fi

        case $RETURNVAL in
            0)   RUN=0; log "- Curl returned: 0 [OK]";;
            1)   log_warning "- Curl returned: 1 [CURLE_UNSUPPORTED_PROTOCOL - The URL you passed to libcurl used a protocol that this libcurl does not support. The support might be a compile-time option that you didn't use, it can be a misspelled protocol string or just a protocol libcurl has no code for.]"; RETRY_ENABLED=yes;;
            2)   log_warning "- Curl returned: 2 [CURLE_FAILED_INIT - Very early initialization code failed. This is likely to be an internal error or problem, or a resource problem where something fundamental couldn't get done at init time.]"; RETRY_ENABLED=yes;;
            3)   log_error "- Curl returned: 3 [CURLE_URL_MALFORMAT - The URL was not properly formatted.]"; RETRY_ENABLED=no;;
            4)   log_error "- Curl returned: 4 [CURLE_NOT_BUILT_IN - A requested feature, protocol or option was not found built-in in this libcurl due to a build-time decision. This means that a feature or option was not enabled or explicitly disabled when libcurl was built and in order to get it to function you have to get a rebuilt libcurl.]"; RETRY_ENABLED=no;;
            5)   log_warning "- Curl returned: 5 [CURLE_COULDNT_RESOLVE_PROXY - Couldn't resolve proxy. The given proxy host could not be resolved.]"; RETRY_ENABLED=yes;;
            6)   log_warning "- Curl returned: 6 [CURLE_COULDNT_RESOLVE_HOST - Couldn't resolve host. The given remote host was not resolved.]"; RETRY_ENABLED=yes;;
            7)   log_warning "- Curl returned: 7 [CURLE_COULDNT_CONNECT - Failed to connect() to host or proxy.]"; RETRY_ENABLED=yes;;
            8)   log_warning "- Curl returned: 8 [CURLE_FTP_WEIRD_SERVER_REPLY - After connecting to a FTP server, libcurl expects to get a certain reply back. This error code implies that it got a strange or bad reply. The given remote server is probably not an OK FTP server.]"; RETRY_ENABLED=yes;;
            9)   log_warning "- Curl returned: 9 [CURLE_REMOTE_ACCESS_DENIED - We were denied access to the resource given in the URL. For FTP, this occurs while trying to change to the remote directory.]"; RETRY_ENABLED=yes;;
            10)  log_warning "- Curl returned: 10 [CURLE_FTP_ACCEPT_FAILED - While waiting for the server to connect back when an active FTP session is used, an error code was sent over the control connection or similar.]"; RETRY_ENABLED=yes;;
            11)  log_warning "- Curl returned: 11 [CURLE_FTP_WEIRD_PASS_REPLY - After having sent the FTP password to the server, libcurl expects a proper reply. This error code indicates that an unexpected code was returned.]"; RETRY_ENABLED=yes;;
            12)  log_warning "- Curl returned: 12 [CURLE_FTP_ACCEPT_TIMEOUT - During an active FTP session while waiting for the server to connect, the CURLOPT_ACCEPTTIMOUT_MS (or the internal default) timeout expired.]"; RETRY_ENABLED=yes;;
            13)  log_warning "- Curl returned: 13 [CURLE_FTP_WEIRD_PASV_REPLY - libcurl failed to get a sensible result back from the server as a response to either a PASV or a EPSV command. The server is flawed.]"; RETRY_ENABLED=yes;;
            14)  log_error "- Curl returned: 14 [CURLE_FTP_WEIRD_227_FORMAT - FTP servers return a 227-line as a response to a PASV command. If libcurl fails to parse that line, this return code is passed back.]"; RETRY_ENABLED=no;;
            15)  log_warning "- Curl returned: 15 [CURLE_FTP_CANT_GET_HOST - An internal failure to lookup the host used for the new connection.]"; RETRY_ENABLED=yes;;
            17)  log_warning "- Curl returned: 17 [CURLE_FTP_COULDNT_SET_TYPE - Received an error when trying to set the transfer mode to binary or ASCII.]"; RETRY_ENABLED=yes;;
            18)  log_warning "- Curl returned: 18 [CURLE_PARTIAL_FILE - A file transfer was shorter or larger than expected. This happens when the server first reports an expected transfer size, and then delivers data that doesn't match the previously given size.]"; RETRY_ENABLED=yes;;
            19)  log_warning "- Curl returned: 19 [CURLE_FTP_COULDNT_RETR_FILE - This was either a weird reply to a 'RETR' command or a zero byte transfer complete.]"; RETRY_ENABLED=yes;;
            21)  log_error "- Curl returned: 21 [CURLE_QUOTE_ERROR - When sending custom QUOTE commands to the remote server, one of the commands returned an error code that was 400 or higher (for FTP) or otherwise indicated unsuccessful completion of the command.]"; RETRY_ENABLED=no;;
            22)  log_warning "- Curl returned: 22 [CURLE_HTTP_RETURNED_ERROR - This is returned if CURLOPT_FAILONERROR is set TRUE and the HTTP server returns an error code that is >= 400.]"; RETRY_ENABLED=yes;;
            23)  log_error "- Curl returned: 23 [CURLE_WRITE_ERROR - An error occurred when writing received data to a local file, or an error was returned to libcurl from a write callback.]"; RETRY_ENABLED=no;;
            25)  log_warning "- Curl returned: 25 [CURLE_UPLOAD_FAILED - Failed starting the upload. For FTP, the server typically denied the STOR command. The error buffer usually contains the server's explanation for this.]"; RETRY_ENABLED=yes;;
            26)  log_warning "- Curl returned: 26 [CURLE_READ_ERROR - There was a problem reading a local file or an error returned by the read callback.]"; RETRY_ENABLED=yes;;
            27)  log_error "- Curl returned: 27 [CURLE_OUT_OF_MEMORY - A memory allocation request failed. This is serious badness and things are severely screwed up if this ever occurs.]"; RETRY_ENABLED=no;;
            28)  log_warning "- Curl returned: 28 [CURLE_OPERATION_TIMEDOUT - Operation timeout. The specified time-out period was reached according to the conditions.]"; RETRY_ENABLED=yes;;
            30)  log_error "- Curl returned: 30 [CURLE_FTP_PORT_FAILED - The FTP PORT command returned error. This mostly happens when you haven't specified a good enough address for libcurl to use. See CURLOPT_FTPPORT.]"; RETRY_ENABLED=no;;
            31)  log_error "- Curl returned: 31 [CURLE_FTP_COULDNT_USE_REST - The FTP REST command returned error. This should never happen if the server is sane.]"; RETRY_ENABLED=no;;
            33)  log_error "- Curl returned: 33 [CURLE_RANGE_ERROR - The server does not support or accept range requests.]"; RETRY_ENABLED=yes;;
            34)  log_warning "- Curl returned: 34 [CURLE_HTTP_POST_ERROR - This is an odd error that mainly occurs due to internal confusion.]"; RETRY_ENABLED=yes;;
            35)  log_warning "- Curl returned: 35 [CURLE_SSL_CONNECT_ERROR - A problem occurred somewhere in the SSL/TLS handshake. You really want the error buffer and read the message there as it pinpoints the problem slightly more. Could be certificates (file formats, paths, permissions), passwords, and others.]"; RETRY_ENABLED=yes;;
            36)  log_error "- Curl returned: 36 [CURLE_BAD_DOWNLOAD_RESUME - The download could not be resumed because the specified offset was out of the file boundary.]"; RETRY_ENABLED=no;;
            37)  log_error "- Curl returned: 37 [CURLE_FILE_COULDNT_READ_FILE - A file given with FILE:// couldn't be opened. Most likely because the file path doesn't identify an existing file. Did you check file permissions?]"; RETRY_ENABLED=no;;
            38)  log_error "- Curl returned: 38 [CURLE_LDAP_CANNOT_BIND - LDAP cannot bind. LDAP bind operation failed.]"; RETRY_ENABLED=no;;
            39)  log_warning "- Curl returned: 39 [CURLE_LDAP_SEARCH_FAILED - LDAP search failed.]"; RETRY_ENABLED=yes;;
            41)  log_error "- Curl returned: 41 [CURLE_FUNCTION_NOT_FOUND - Function not found. A required zlib function was not found.]"; RETRY_ENABLED=no;;
            42)  log_warning "- Curl returned: 42 [CURLE_ABORTED_BY_CALLBACK - Aborted by callback. A callback returned ABORT to libcurl.]"; RETRY_ENABLED=yes;;
            43)  log_error "- Curl returned: 43 [CURLE_BAD_FUNCTION_ARGUMENT - Internal error. A function was called with a bad parameter.]"; RETRY_ENABLED=no;;
            45)  log_error "- Curl returned: 45 [CURLE_INTERFACE_FAILED - Interface error. A specified outgoing interface could not be used. Set which interface to use for outgoing connections' source IP address with CURLOPT_INTERFACE.]"; RETRY_ENABLED=no;;
            47)  log_warning "- Curl returned: 47 [CURLE_TOO_MANY_REDIRECTS - Too many redirects. When following redirects, libcurl hit the maximum amount. Set your limit with CURLOPT_MAXREDIRS.]"; RETRY_ENABLED=yes;;
            48)  log_error "- Curl returned: 48 [CURLE_UNKNOWN_OPTION - An option passed to libcurl is not recognized/known. Refer to the appropriate documentation. This is most likely a problem in the program that uses libcurl. The error buffer might contain more specific information about which exact option it concerns.]"; RETRY_ENABLED=no;;
            49)  log_error "- Curl returned: 49 [CURLE_TELNET_OPTION_SYNTAX - A telnet option string was Illegally formatted.]"; RETRY_ENABLED=no;;
            51)  log_warning "- Curl returned: 51 [CURLE_PEER_FAILED_VERIFICATION - The remote server's SSL certificate or SSH md5 fingerprint was deemed not OK.]"; RETRY_ENABLED=yes;;
            52)  log_warning "- Curl returned: 52 [CURLE_GOT_NOTHING - Nothing was returned from the server, and under the circumstances, getting nothing is considered an error.]"; RETRY_ENABLED=yes;;
            53)  log_error "- Curl returned: 53 [CURLE_SSL_ENGINE_NOTFOUND - The specified crypto engine wasn't found.]"; RETRY_ENABLED=no;;
            54)  log_error "- Curl returned: 54 [CURLE_SSL_ENGINE_SETFAILED - Failed setting the selected SSL crypto engine as default!]"; RETRY_ENABLED=no;;
            55)  log_warning "- Curl returned: 55 [CURLE_SEND_ERROR - Failed sending network data.]"; RETRY_ENABLED=yes;;
            56)  log_warning "- Curl returned: 56 [CURLE_RECV_ERROR - Failure with receiving network data.]"; RETRY_ENABLED=yes;;
            58)  log_error "- Curl returned: 58 [CURLE_SSL_CERTPROBLEM - problem with the local client certificate.]"; RETRY_ENABLED=no;;
            59)  log_error "- Curl returned: 59 [CURLE_SSL_CIPHER - Couldn't use specified cipher.]"; RETRY_ENABLED=no;;
            60)  log_warning "- Curl returned: 60 [CURLE_SSL_CACERT - Peer certificate cannot be authenticated with known CA certificates.]"; RETRY_ENABLED=yes;;
            61)  log_error "- Curl returned: 61 [CURLE_BAD_CONTENT_ENCODING - Unrecognized transfer encoding.]"; RETRY_ENABLED=no;;
            62)  log_error "- Curl returned: 62 [CURLE_LDAP_INVALID_URL - Invalid LDAP URL.]"; RETRY_ENABLED=no;;
            63)  log_warning "- Curl returned: 63 [CURLE_FILESIZE_EXCEEDED - Maximum file size exceeded.]"; RETRY_ENABLED=yes;;
            64)  log_error "- Curl returned: 64 [CURLE_USE_SSL_FAILED - Requested FTP SSL level failed.]"; RETRY_ENABLED=no;;
            65)  log_warning "- Curl returned: 65 [CURLE_SEND_FAIL_REWIND - When doing a send operation curl had to rewind the data to retransmit, but the rewinding operation failed.]"; RETRY_ENABLED=yes;;
            66)  log_error "- Curl returned: 66 [CURLE_SSL_ENGINE_INITFAILED - Initiating the SSL Engine failed.]"; RETRY_ENABLED=no;;
            67)  log_warning "- Curl returned: 67 [CURLE_LOGIN_DENIED - The remote server denied curl to login]"; RETRY_ENABLED=yes;;
            68)  log_error "- Curl returned: 68 [CURLE_TFTP_NOTFOUND - File not found on TFTP server.]"; RETRY_ENABLED=no;;
            69)  log_error "- Curl returned: 69 [CURLE_TFTP_PERM - Permission problem on TFTP server.]"; RETRY_ENABLED=no;;
            70)  log_warning "- Curl returned: 70 [CURLE_REMOTE_DISK_FULL - Out of disk space on the server.]"; RETRY_ENABLED=yes;;
            71)  log_error "- Curl returned: 71 [CURLE_TFTP_ILLEGAL - Illegal TFTP operation.]"; RETRY_ENABLED=no;;
            72)  log_error "- Curl returned: 72 [CURLE_TFTP_UNKNOWNID - Unknown TFTP transfer ID.]"; RETRY_ENABLED=no;;
            73)  log_error "- Curl returned: 73 [CURLE_REMOTE_FILE_EXISTS - File already exists and will not be overwritten.]"; RETRY_ENABLED=no;;
            74)  log_error "- Curl returned: 74 [CURLE_TFTP_NOSUCHUSER - This error should never be returned by a properly functioning TFTP server.]"; RETRY_ENABLED=no;;
            75)  log_error "- Curl returned: 75 [CURLE_CONV_FAILED - Character conversion failed.]"; RETRY_ENABLED=no;;
            76)  log_error "- Curl returned: 76 [CURLE_CONV_REQD - Caller must register conversion callbacks.]"; RETRY_ENABLED=no;;
            77)  log_error "- Curl returned: 77 [CURLE_SSL_CACERT_BADFILE - Problem with reading the SSL CA cert (path? access rights?)]"; RETRY_ENABLED=no;;
            78)  log_error "- Curl returned: 78 [CURLE_REMOTE_FILE_NOT_FOUND - The resource referenced in the URL does not exist.]"; RETRY_ENABLED=no;;
            79)  log_warning "- Curl returned: 79 [CURLE_SSH - An unspecified error occurred during the SSH session.]"; RETRY_ENABLED=yes;;
            80)  log_warning "- Curl returned: 80 [CURLE_SSL_SHUTDOWN_FAILED - Failed to shut down the SSL connection.]"; RETRY_ENABLED=yes;;
            81)  log_warning "- Curl returned: 81 [CURLE_AGAIN - Socket is not ready for send/recv wait till it's ready and try again. This return code is only returned from curl_easy_recv(3) and curl_easy_send(3)]"; RETRY_ENABLED=yes;;
            82)  log_error "- Curl returned: 82 [CURLE_SSL_CRL_BADFILE - Failed to load CRL file]"; RETRY_ENABLED=no;;
            83)  log_error "- Curl returned: 83 [CURLE_SSL_ISSUER_ERROR - Issuer check failed]"; RETRY_ENABLED=no;;
            84)  log_error "- Curl returned: 84 [CURLE_FTP_PRET_FAILED - The FTP server does not understand the PRET command at all or does not support the given argument. Be careful when using CURLOPT_CUSTOMREQUEST, a custom LIST command will be sent with PRET CMD before PASV as well.]"; RETRY_ENABLED=no;;
            85)  log_warning "- Curl returned: 85 [CURLE_RTSP_CSEQ_ERROR - Mismatch of RTSP CSeq numbers.]"; RETRY_ENABLED=yes;;
            86)  log_warning "- Curl returned: 86 [CURLE_RTSP_SESSION_ERROR - Mismatch of RTSP Session Identifiers.]"; RETRY_ENABLED=yes;;
            87)  log_warning "- Curl returned: 87 [CURLE_FTP_BAD_FILE_LIST - Unable to parse FTP file list (during FTP wildcard downloading).]"; RETRY_ENABLED=yes;;
            88)  log_error "- Curl returned: 88 [CURLE_CHUNK_FAILED - Chunk callback reported error.]"; RETRY_ENABLED=no;;
            *)   log_error "- Curl returned: $RETURNVAL [Unknown - This result code is not listed by Curl, it is possible the result codes have changed]. Aborting operation"; RETRY_ENABLED=no;;
            ?)   log_error "- Curl returned: $RETURNVAL [Unknown - This result code is not listed by Curl, it is possible the result codes have changed]. Aborting operation"; RETRY_ENABLED=no;;
        esac
        # NGINX responds with HTTP code 416 if there is a problem with file resume related items, this is here to catch that condition to force a retry
        if [ "${TMP_CURL_HTTP_CODE}" -eq 416 ] && option_enabled TMP_RESUME_DOWNLOAD; then
            RUN=1
            RETRY_ENABLED=yes
        fi
        if option_enabled HAS_ERROR; then
            if option_enabled RETRY_ENABLED; then
                # There was an error via HTTP response code, however we might retry the call, forcing reset of TMP_FILE_OUTPUT
                > ${TMP_FILE_OUTPUT}
            fi
        fi
        if [ ${RETURNVAL} -ne 0 ]; then
            log_add_from_file "${FILE_TMP_CURL_ERROR}" "error"
            # NGINX responds with HTTP code 400 if a certificate is supplied and is rejected, so, if we send a client cert and get 400, abort call here to prevent retries
            if [ "${TMP_CURL_HTTP_CODE}" -eq 400 ]; then
                if(! is_empty "${TMP_AUTH_CLIENT_CERT_FILE}"); then
                    log_error "- Server rejected supplied client certificate"
                    return $E_CURL_FAILURE;
                fi
            fi
            if ! option_enabled RETRY_ENABLED; then
                return $E_CURL_FAILURE;
            fi
        else
            if ! option_enabled HAS_ERROR; then
                return 0
            fi
        fi
    done

    # Theoretically, we should never make it here, however, if we do for whatever reason, it is because of a failure. This is here to serve as a catch-all
    return $E_CURL_FAILURE
}
