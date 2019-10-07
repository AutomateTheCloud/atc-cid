###------------------------------------------------------------------------------------------------
# script:  npm.inc
# purpose: Collection of functions related to NPM
# version: 1.0.0
#
# function list:
#          - npm_does_version_exist
#          - npm_generate_npmrc
###------------------------------------------------------------------------------------------------
REQUIRED_EXECUTABLES+=('npm')

###--------------------------------------------------------------------------------
## FUNCTION: npm_does_version_exist()
## - Checks if NPM Package exists at the specified version against an NPM Registry
## - Returns 0 if exists
## - Returns E_NPM_PACKAGE_DOES_NOT_EXIST if does not exist
## - Arguments
##   - $1: Registry URL
##   - $2: Directory to Output Npmrc file
##   - $3: Filename override (optional, if not specified, defaults to '.npmrc')
function npm_does_version_exist() {
    local FUNCTION_DESCRIPTION="NPM: Does NPM Package Version Exist"
    local TMP_PACKAGE_NAME="${1}"
    local TMP_PACKAGE_VERSION="${2}"
    local TMP_DIRECTORY_WORKING="${3}"
    local TMP_STRING=""

    if(is_empty "${TMP_PACKAGE_NAME}"); then
        log_error "${FUNCTION_DESCRIPTION}: Package Name not specified"
        return $E_BAD_ARGS
    fi
    if(is_empty "${TMP_PACKAGE_VERSION}"); then
        log_error "${FUNCTION_DESCRIPTION}: Package Version not specified"
        return $E_BAD_ARGS
    fi
    TMP_DIRECTORY_WORKING="$(echo "${TMP_DIRECTORY_WORKING}" | sed 's:/*$::')"
    if(! is_empty "${TMP_DIRECTORY_WORKING}"); then
        if [ ! -d "${TMP_DIRECTORY_WORKING}" ]; then
            log_error "${FUNCTION_DESCRIPTION}: Working Directory [${TMP_DIRECTORY_WORKING}] does not exist"
            return $E_OBJECT_NOT_FOUND
        fi
    fi

    log "${FUNCTION_DESCRIPTION}: started"
    log "- Name:              [${TMP_PACKAGE_NAME}]"
    log "- Version:           [${TMP_PACKAGE_VERSION}]"
    line_break

    if(! is_empty "${TMP_DIRECTORY_WORKING}"); then
        log "- Working Directory: [${TMP_DIRECTORY_WORKING}]"
        cd ${TMP_DIRECTORY_WORKING}
    fi

    TMP_STRING="$($(which npm) view ${TMP_PACKAGE_NAME}@${TMP_PACKAGE_VERSION} name)"
    if(! is_empty "${TMP_STRING}"); then
        log "- [Package Version exists in Registry]"
        return 0
    else
        log_warning "- [Package Version does not exists in Registry]"
        return $E_NPM_PACKAGE_DOES_NOT_EXIST
    fi
}

###--------------------------------------------------------------------------------
## FUNCTION: npm_generate_npmrc()
## - Generates .npmrc file at specified location
## - Arguments
##   - $1: Registry URL
##   - $2: Directory to Output Npmrc file
##   - $3: Filename override (optional, if not specified, defaults to '.npmrc')
function npm_generate_npmrc() {
    local FUNCTION_DESCRIPTION="NPM: Generate Npmrc"
    local TMP_REGISTRY_URL="${1}"
    local TMP_DIRECTORY_NPMRC="${2}"
    local TMP_FILENAME_NPMRC="${3}"

    if(is_empty "${TMP_REGISTRY_URL}"); then
        log_error "${FUNCTION_DESCRIPTION}: Registry URL not specified"
        return $E_BAD_ARGS
    fi
    TMP_DIRECTORY_NPMRC="$(echo "${TMP_DIRECTORY_NPMRC}" | sed 's:/*$::')"
    if(is_empty "${TMP_DIRECTORY_NPMRC}"); then
        log_error "${FUNCTION_DESCRIPTION}: Directory not specified"
        return $E_BAD_ARGS
    fi
    if(is_empty "${TMP_FILENAME_NPMRC}"); then
        TMP_FILENAME_NPMRC=".npmrc"
    fi

    if [ ! -d "${TMP_DIRECTORY_NPMRC}" ]; then
        log_error "${FUNCTION_DESCRIPTION}: Directory [${TMP_DIRECTORY_NPMRC}] does not exist"
        return $E_OBJECT_NOT_FOUND
    fi

    log "${FUNCTION_DESCRIPTION}: Generating Npmrc file"
    log "- File:         [${TMP_DIRECTORY_NPMRC}/${TMP_FILENAME_NPMRC}]"
    log "- Registry URL: [${TMP_FILENAME_NPMRC}]"

cat > ${TMP_DIRECTORY_NPMRC}/${TMP_FILENAME_NPMRC} << ZZEOF
registry=${TMP_REGISTRY_URL}
always-auth=true
ZZEOF

    return 0
}
