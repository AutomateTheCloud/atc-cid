###------------------------------------------------------------------------------------------------
# script:  tar.inc
# purpose: Collection of functions related to tar
# version: 1.0.0
#
# function list:
#          - extract_tar_to_directory
#          - extract_tar_to_directory_compress_bzip2
#          - extract_tar_to_directory_compress_gzip
#          - tar_directory
#          - tar_directory_compress_bzip2
#          - tar_directory_compress_gzip
#          - tar_file_compress_bzip2
###------------------------------------------------------------------------------------------------
REQUIRED_EXECUTABLES+=('tar')

###------------------------------------------------------------------------------------------------
# Variables
TAR_BZIP2_FILE_EXTENSION="tbz2"
TAR_GZIP_FILE_EXTENSION="tgz"
TAR_FILE_EXTENSION="tar"

###------------------------------------------------------------------------------------------------
## FUNCTION: extract_tar_to_directory()
## - wrapper function for making tar calls
## - this function does not use compression when using tar
## - Arguments
##   - $1: file name (tar archive to extract)
##   - $2: destination directory (optional, if not set, absolute path must be enabled)
##   - $3: use absolute path (optional, yes/no, default: no; If yes, files will be extracted with absolute path from "/", if no, files will be extracted with relative path)
function extract_tar_to_directory() {
    local FILE_ARCHIVE="${1}"
    local DESTINATION_DIRECTORY="${2}"
    local USE_ABSOLUTE_PATH="${3}"
    if(is_empty "${USE_ABSOLUTE_PATH}"); then
        USE_ABSOLUTE_PATH=no
    fi
    if option_enabled USE_ABSOLUTE_PATH; then
        DESTINATION_DIRECTORY="/"
    else
        if(is_empty "${DESTINATION_DIRECTORY}"); then
            log_error "Tar: Cannot extract archive, Destination Directory must be specified if not using absolute path"
            return $BAD_ARGS
        fi
    fi

    log "Tar: Attempting to extract archive (Archive::${FILE_ARCHIVE} / Destination::[${DESTINATION_DIRECTORY}] / Use_Absolute_Path::${USE_ABSOLUTE_PATH} / Compression_Format::none)"
    if [ ! -f "${FILE_ARCHIVE}" ]; then
        log_error "Tar: File archive does not exist [${FILE_ARCHIVE}]"
        return $E_OBJECT_NOT_FOUND
    fi
    if [ ! -d "${DESTINATION_DIRECTORY}" ]; then
        log_error "Tar: Destination directory does not exist [${DESTINATION_DIRECTORY}]"
        return $E_OBJECT_NOT_FOUND
    fi

    generate_temp_file DISCARDABLE_VARIABLE "tar output log"
    local TAR_OUTPUT_LOG="${DISCARDABLE_VARIABLE}"
    local RETURNVAL=""

    $(which tar) -xf "${FILE_ARCHIVE}" -C "${DESTINATION_DIRECTORY}" 2> ${TAR_OUTPUT_LOG}
    RETURNVAL="$?"
    if [ ${RETURNVAL} -ne 0 ]; then
        log_error "Tar: Failed to extract archive [${FILE_ARCHIVE}] (Tar_Returned::${RETURNVAL})"
        log_add_from_file "${TAR_OUTPUT_LOG}" "output of tar operation"
        return $E_OBJECT_FAILED_TO_CREATE
    else
        log "Tar: Successfully extracted archive (Archive::${FILE_ARCHIVE} / Destination::[${DESTINATION_DIRECTORY}])"
        return 0
    fi
}

###------------------------------------------------------------------------------------------------
## FUNCTION: extract_tar_to_directory_compress_bzip2()
## - wrapper function for making tar calls
## - this function use bzip2 compression when using tar
## - Arguments
##   - $1: file name (tar archive to extract)
##   - $2: destination directory (optional, if not set, absolute path must be enabled)
##   - $3: use absolute path (optional, yes/no, default: no; If yes, files will be extracted with absolute path from "/", if no, files will be extracted with relative path)
function extract_tar_to_directory_compress_bzip2() {
    local FILE_ARCHIVE="${1}"
    local DESTINATION_DIRECTORY="${2}"
    local USE_ABSOLUTE_PATH="${3}"
    if(is_empty "${USE_ABSOLUTE_PATH}"); then
        USE_ABSOLUTE_PATH=no
    fi
    if option_enabled USE_ABSOLUTE_PATH; then
        DESTINATION_DIRECTORY="/"
    else
        if(is_empty "${DESTINATION_DIRECTORY}"); then
            log_error "Tar: Cannot extract archive, Destination Directory must be specified if not using absolute path"
            return $BAD_ARGS
        fi
    fi

    log "Tar: Attempting to extract archive (Archive::${FILE_ARCHIVE} / Destination::[${DESTINATION_DIRECTORY}] / Use_Absolute_Path::${USE_ABSOLUTE_PATH} / Compression_Format: bzip2)"
    if [ ! -f "${FILE_ARCHIVE}" ]; then
        log_error "Tar: File archive does not exist [${FILE_ARCHIVE}]"
        return $E_OBJECT_NOT_FOUND
    fi
    if [ ! -d "${DESTINATION_DIRECTORY}" ]; then
        log_error "Tar: Destination directory does not exist [${DESTINATION_DIRECTORY}]"
        return $E_OBJECT_NOT_FOUND
    fi

    generate_temp_file DISCARDABLE_VARIABLE "tar output log"
    local TAR_OUTPUT_LOG="${DISCARDABLE_VARIABLE}"
    local RETURNVAL=""

    $(which tar) -jxf "${FILE_ARCHIVE}" -C "${DESTINATION_DIRECTORY}" 2> ${TAR_OUTPUT_LOG}
    RETURNVAL="$?"
    if [ ${RETURNVAL} -ne 0 ]; then
        log_error "Tar: Failed to extract archive [${FILE_ARCHIVE}] (Tar_Returned: ${RETURNVAL})"
        log_add_from_file "${TAR_OUTPUT_LOG}" "output of tar operation"
        return $E_OBJECT_FAILED_TO_CREATE
    else
        log "Tar: Successfully extracted archive (Archive::${FILE_ARCHIVE} / Destination::[${DESTINATION_DIRECTORY}])"
        return 0
    fi
}

###------------------------------------------------------------------------------------------------
## FUNCTION: extract_tar_to_directory_compress_gzip()
## - wrapper function for making tar calls
## - this function use bzip2 compression when using tar
## - Arguments
##   - $1: file name (tar archive to extract)
##   - $2: destination directory (optional, if not set, absolute path must be enabled)
##   - $3: use absolute path (optional, yes/no, default: no; If yes, files will be extracted with absolute path from "/", if no, files will be extracted with relative path)
function extract_tar_to_directory_compress_gzip() {
    local FILE_ARCHIVE="${1}"
    local DESTINATION_DIRECTORY="${2}"
    local USE_ABSOLUTE_PATH="${3}"
    if(is_empty "${USE_ABSOLUTE_PATH}"); then
        USE_ABSOLUTE_PATH=no
    fi
    if option_enabled USE_ABSOLUTE_PATH; then
        DESTINATION_DIRECTORY="/"
    else
        if(is_empty "${DESTINATION_DIRECTORY}"); then
            log_error "Tar: Cannot extract archive, Destination Directory must be specified if not using absolute path"
            return $BAD_ARGS
        fi
    fi

    log "Tar: Attempting to extract archive (Archive::${FILE_ARCHIVE} / Destination::[${DESTINATION_DIRECTORY}] / Use_Absolute_Path::${USE_ABSOLUTE_PATH} / Compression_Format: gzip)"
    if [ ! -f "${FILE_ARCHIVE}" ]; then
        log_error "Tar: File archive does not exist [${FILE_ARCHIVE}]"
        return $E_OBJECT_NOT_FOUND
    fi
    if [ ! -d "${DESTINATION_DIRECTORY}" ]; then
        log_error "Tar: Destination directory does not exist [${DESTINATION_DIRECTORY}]"
        return $E_OBJECT_NOT_FOUND
    fi

    generate_temp_file DISCARDABLE_VARIABLE "tar output log"
    local TAR_OUTPUT_LOG="${DISCARDABLE_VARIABLE}"
    local RETURNVAL=""

    $(which tar) -zxf "${FILE_ARCHIVE}" -C "${DESTINATION_DIRECTORY}" 2> ${TAR_OUTPUT_LOG}
    RETURNVAL="$?"
    if [ ${RETURNVAL} -ne 0 ]; then
        log_error "Tar: Failed to extract archive [${FILE_ARCHIVE}] (Tar_Returned: ${RETURNVAL})"
        log_add_from_file "${TAR_OUTPUT_LOG}" "output of tar operation"
        return $E_OBJECT_FAILED_TO_CREATE
    else
        log "Tar: Successfully extracted archive (Archive::${FILE_ARCHIVE} / Destination::[${DESTINATION_DIRECTORY}])"
        return 0
    fi
}

###------------------------------------------------------------------------------------------------
## FUNCTION: tar_directory()
## - wrapper function for making tar calls
## - this function does not use compression when using tar
## - Arguments
##   - $1: output file name (tar archive to create)
##   - $2: target to archive
##   - $3: use absolute path (optional, yes/no, default: yes; If yes, files will be archived with absolute path, if no, files will be archived with relative path)
function tar_directory() {
    local FILE_ARCHIVE="${1}"
    local TARGET_TO_ARCHIVE="${2}"
    local USE_ABSOLUTE_PATH="${3}"
    if(is_empty "${USE_ABSOLUTE_PATH}"); then
        USE_ABSOLUTE_PATH=yes
    fi
    log "Tar: Attempting to create archive (Archive::${FILE_ARCHIVE} / Directory::[${TARGET_TO_ARCHIVE}] / Use_Absolute_Path::${USE_ABSOLUTE_PATH} / Compression_Format::none)"
    generate_temp_file DISCARDABLE_VARIABLE "tar output log"
    local TAR_OUTPUT_LOG="${DISCARDABLE_VARIABLE}"
    local RETURNVAL=""
    PROCESSING_WORKING_FILE="${FILE_ARCHIVE}"
    if option_enabled USE_ABSOLUTE_PATH; then
        $(which tar) -cf "${FILE_ARCHIVE}" "${TARGET_TO_ARCHIVE}" 2> ${TAR_OUTPUT_LOG}
        RETURNVAL="$?"
    else
        $(which tar) -cf "${FILE_ARCHIVE}" -C "${TARGET_TO_ARCHIVE}" . 2> ${TAR_OUTPUT_LOG}
        RETURNVAL="$?"
    fi
    PROCESSING_WORKING_FILE=""
    if [ ${RETURNVAL} -ne 0 ]; then
        log_error "Tar: Failed to create archive [${FILE_ARCHIVE}] (Tar_Returned::${RETURNVAL})"
        log_add_from_file "${TAR_OUTPUT_LOG}" "output of tar operation"
        rm -f "${FILE_ARCHIVE}"
        return $E_OBJECT_FAILED_TO_CREATE
    else
        if [ ! -f "${FILE_ARCHIVE}" ]; then
            log_error "Tar: Tar operation returned success, however the generated archive [${FILE_ARCHIVE}] could not be found"
            return $E_OBJECT_FAILED_TO_CREATE
        else
            log "Tar: Successfully created archive (Archive::${FILE_ARCHIVE} / Size::$(return_filesize_of_file "${FILE_ARCHIVE}") / SHA256::$(return_file_sha256sum "${FILE_ARCHIVE}"))"
            return 0
        fi
    fi
}

###------------------------------------------------------------------------------------------------
## FUNCTION: tar_directory_compress_bzip2()
## - wrapper function for making tar calls utilizing bzip2 compression
## - if lbzip2 is installed, function will use that instead of bzip2
## - Arguments
##   - $1: output file name (tar archive to create)
##   - $2: target to archive
##   - $3: use absolute path (optional, yes/no, default: yes; If yes, files will be archived with absolute path, if no, files will be archived with relative path)
function tar_directory_compress_bzip2() {
    local FILE_ARCHIVE="${1}"
    local TARGET_TO_ARCHIVE="${2}"
    local USE_ABSOLUTE_PATH="${3}"
    if(is_empty "${USE_ABSOLUTE_PATH}"); then
        USE_ABSOLUTE_PATH=yes
    fi
    if which lbzip2 > /dev/null 2>&1; then
        log "Tar: Attempting to create archive (Archive::${FILE_ARCHIVE} / Directory::[${TARGET_TO_ARCHIVE}] / Use_Absolute_Path::${USE_ABSOLUTE_PATH} / Compression_Format::bzip2 [using lbzip2])"
    else
        log "Tar: Attempting to create archive (Archive::${FILE_ARCHIVE} / Directory::[${TARGET_TO_ARCHIVE}] / Use_Absolute_Path::${USE_ABSOLUTE_PATH} / Compression_Format::bzip2)"
    fi
    generate_temp_file DISCARDABLE_VARIABLE "tar output log"
    local TAR_OUTPUT_LOG="${DISCARDABLE_VARIABLE}"
    local RETURNVAL=""
    PROCESSING_WORKING_FILE="${FILE_ARCHIVE}"
    if option_enabled USE_ABSOLUTE_PATH; then
        if which lbzip2 > /dev/null 2>&1; then
            $(which tar) -I lbzip2 -cf "${FILE_ARCHIVE}" "${TARGET_TO_ARCHIVE}" 2> ${TAR_OUTPUT_LOG}
            RETURNVAL="$?"
        else
            $(which tar) -jcf "${FILE_ARCHIVE}" "${TARGET_TO_ARCHIVE}" 2> ${TAR_OUTPUT_LOG}
            RETURNVAL="$?"
        fi
    else
        if which lbzip2 > /dev/null 2>&1; then
            $(which tar) -I lbzip2 -cf "${FILE_ARCHIVE}" -C "${TARGET_TO_ARCHIVE}" . 2> ${TAR_OUTPUT_LOG}
            RETURNVAL="$?"
        else
            $(which tar) -jcf "${FILE_ARCHIVE}" -C "${TARGET_TO_ARCHIVE}" . 2> ${TAR_OUTPUT_LOG}
            RETURNVAL="$?"
        fi
    fi
    PROCESSING_WORKING_FILE=""
    if [ ${RETURNVAL} -ne 0 ]; then
        log_error "Tar: Failed to create archive [${FILE_ARCHIVE}] (Tar_Returned::${RETURNVAL})"
        log_add_from_file "${TAR_OUTPUT_LOG}" "output of tar operation"
        rm -f "${FILE_ARCHIVE}"
        return $E_OBJECT_FAILED_TO_CREATE
    else
        if [ ! -f "${FILE_ARCHIVE}" ]; then
            log_error "Tar: Tar operation returned success, however the generated archive [${FILE_ARCHIVE}] could not be found"
            return $E_OBJECT_FAILED_TO_CREATE
        else
            log "Tar: Successfully created archive (Archive::${FILE_ARCHIVE} / Size::$(return_filesize_of_file "${FILE_ARCHIVE}") / SHA256::$(return_file_sha256sum "${FILE_ARCHIVE}"))"
            return 0
        fi
    fi
}

###------------------------------------------------------------------------------------------------
## FUNCTION: tar_directory_compress_gzip()
## - wrapper function for making tar calls utilizing gzip compression
## - Arguments
##   - $1: output file name (tar archive to create)
##   - $2: target to archive
##   - $3: use absolute path (optional, yes/no, default: yes; If yes, files will be archived with absolute path, if no, files will be archived with relative path)
function tar_directory_compress_gzip() {
    local FILE_ARCHIVE="${1}"
    local TARGET_TO_ARCHIVE="${2}"
    local USE_ABSOLUTE_PATH="${3}"
    if(is_empty "${USE_ABSOLUTE_PATH}"); then
        USE_ABSOLUTE_PATH=yes
    fi
    log "Tar: Attempting to create archive (Archive::${FILE_ARCHIVE} / Directory::[${TARGET_TO_ARCHIVE}] / Use_Absolute_Path::${USE_ABSOLUTE_PATH} / Compression_Format::gzip)"
    generate_temp_file DISCARDABLE_VARIABLE "tar output log"
    local TAR_OUTPUT_LOG="${DISCARDABLE_VARIABLE}"
    local RETURNVAL=""
    PROCESSING_WORKING_FILE="${FILE_ARCHIVE}"
    if option_enabled USE_ABSOLUTE_PATH; then
        $(which tar) -zcf "${FILE_ARCHIVE}" "${TARGET_TO_ARCHIVE}" 2> ${TAR_OUTPUT_LOG}
        RETURNVAL="$?"
    else
        $(which tar) -zcf "${FILE_ARCHIVE}" -C "${TARGET_TO_ARCHIVE}" . 2> ${TAR_OUTPUT_LOG}
        RETURNVAL="$?"
    fi
    PROCESSING_WORKING_FILE=""
    if [ ${RETURNVAL} -ne 0 ]; then
        log_error "Tar: Failed to create archive [${FILE_ARCHIVE}] (Tar_Returned::${RETURNVAL})"
        log_add_from_file "${TAR_OUTPUT_LOG}" "output of tar operation"
        rm -f "${FILE_ARCHIVE}"
        return $E_OBJECT_FAILED_TO_CREATE
    else
        if [ ! -f "${FILE_ARCHIVE}" ]; then
            log_error "Tar: Tar operation returned success, however the generated archive [${FILE_ARCHIVE}] could not be found"
            return $E_OBJECT_FAILED_TO_CREATE
        else
            log "Tar: Successfully created archive (Archive::${FILE_ARCHIVE} / Size::$(return_filesize_of_file "${FILE_ARCHIVE}") / SHA256::$(return_file_sha256sum "${FILE_ARCHIVE}"))"
            return 0
        fi
    fi
}

###------------------------------------------------------------------------------------------------
## FUNCTION: tar_file_compress_bzip2()
## - wrapper function for making tar calls utilizing bzip2 compression
## - if lbzip2 is installed, function will use that instead of bzip2
## - Arguments
##   - $1: output file name (tar archive to create)
##   - $2: target to archive
##   - $3: use absolute path (optional, yes/no, default: yes; If yes, files will be archived with absolute path, if no, files will be archived with relative path)
function tar_file_compress_bzip2() {
    local FILE_ARCHIVE="${1}"
    local TARGET_TO_ARCHIVE="${2}"
    local USE_ABSOLUTE_PATH="${3}"
    if(is_empty "${USE_ABSOLUTE_PATH}"); then
        USE_ABSOLUTE_PATH=no
    fi
    if which lbzip2 > /dev/null 2>&1; then
        log "Tar: Attempting to create archive (Archive::${FILE_ARCHIVE} / Directory::[${TARGET_TO_ARCHIVE}] / Use_Absolute_Path::${USE_ABSOLUTE_PATH} / Compression_Format::bzip2 [using lbzip2])"
    else
        log "Tar: Attempting to create archive (Archive::${FILE_ARCHIVE} / Directory::[${TARGET_TO_ARCHIVE}] / Use_Absolute_Path::${USE_ABSOLUTE_PATH} / Compression_Format::bzip2)"
    fi
    generate_temp_file DISCARDABLE_VARIABLE "tar output log"
    local TAR_OUTPUT_LOG="${DISCARDABLE_VARIABLE}"
    local RETURNVAL=""
    PROCESSING_WORKING_FILE="${FILE_ARCHIVE}"
    if option_enabled USE_ABSOLUTE_PATH; then
        if which lbzip2 > /dev/null 2>&1; then
            $(which tar) -I lbzip2 -cf "${FILE_ARCHIVE}" "${TARGET_TO_ARCHIVE}" 2> ${TAR_OUTPUT_LOG}
            RETURNVAL="$?"
        else
            $(which tar) -jcf "${FILE_ARCHIVE}" "${TARGET_TO_ARCHIVE}" 2> ${TAR_OUTPUT_LOG}
            RETURNVAL="$?"
        fi
    else
        if which lbzip2 > /dev/null 2>&1; then
            $(which tar) -I lbzip2 -cf "${FILE_ARCHIVE}" -C "$($(which dirname) "${TARGET_TO_ARCHIVE}")" "${TARGET_TO_ARCHIVE}" 2> ${TAR_OUTPUT_LOG}
            RETURNVAL="$?"
        else
            $(which tar) -jcf "${FILE_ARCHIVE}" -C "$($(which dirname) "${TARGET_TO_ARCHIVE}")" "${TARGET_TO_ARCHIVE}" 2> ${TAR_OUTPUT_LOG}
            RETURNVAL="$?"
        fi
    fi
    PROCESSING_WORKING_FILE=""
    if [ ${RETURNVAL} -ne 0 ]; then
        log_error "Tar: Failed to create archive [${FILE_ARCHIVE}] (Tar_Returned::${RETURNVAL})"
        log_add_from_file "${TAR_OUTPUT_LOG}" "output of tar operation"
        rm -f "${FILE_ARCHIVE}"
        return $E_OBJECT_FAILED_TO_CREATE
    else
        if [ ! -f "${FILE_ARCHIVE}" ]; then
            log_error "Tar: Tar operation returned success, however the generated archive [${FILE_ARCHIVE}] could not be found"
            return $E_OBJECT_FAILED_TO_CREATE
        else
            log "Tar: Successfully created archive (Archive::${FILE_ARCHIVE} / Size::$(return_filesize_of_file "${FILE_ARCHIVE}") / SHA256::$(return_file_sha256sum "${FILE_ARCHIVE}"))"
            return 0
        fi
    fi
}
