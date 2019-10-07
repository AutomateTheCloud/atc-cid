###------------------------------------------------------------------------------------------------
# script:  git.inc
# purpose: Collection of functions related to Git
# version: 1.0.0
#
# function list:
#          - generate_repo_list_organization
#          - is_repo_up_to_date
###------------------------------------------------------------------------------------------------
REQUIRED_EXECUTABLES+=('git')

###--------------------------------------------------------------------------------
## FUNCTION: generate_list_organization_repo()
## - Loads SSL certificate and chain file
## - Arguments
##   - $1: Organization Name
##   - $2: Git API Key
##   - $3: File to ouput list to
function generate_repo_list_organization() {
    local FUNCTION_DESCRIPTION="Generate Repo List (Organization)"
    local FILE_TMP_GIT_ERROR=""
    local FILE_TMP_GIT_REPO_LIST=""
    local FILE_TMP_GIT_API_RESPONSE="";
    local GIT_ORGANIZATION_NAME="${1}"
    local GIT_API_KEY="${2}"
    local FILE_REPO_LIST_OUTPUT="${3}"
    
    local TMP_LINE_COUNT=0
    local TMP_REPO_COUNT=0
    local TMP_PAGE=1
    local RETRIEVE_GIT_LOOP=yes

    if(is_empty "${GIT_ORGANIZATION_NAME}"); then
        log_error "${FUNCTION_DESCRIPTION}: Organization name not specified"
        return $E_BAD_ARGS
    fi
    if(is_empty "${GIT_API_KEY}"); then
        log_error "${FUNCTION_DESCRIPTION}: Git API Key name not specified"
        return $E_BAD_ARGS
    fi
    if(is_empty "${FILE_REPO_LIST_OUTPUT}"); then
        log_error "${FUNCTION_DESCRIPTION}: Repo List Output destination file not specified"
        return $E_BAD_ARGS
    fi
    if [ ! -f "${FILE_REPO_LIST_OUTPUT}" ]; then
        log_error "${FUNCTION_DESCRIPTION}: Repo List Output destination file [${FILE_REPO_LIST_OUTPUT}] does not exist"
        return $E_OBJECT_NOT_FOUND
    fi

    generate_temp_file DISCARDABLE_VARIABLE "Git Error Response"
    FILE_TMP_GIT_ERROR="${DISCARDABLE_VARIABLE}"
    generate_temp_file DISCARDABLE_VARIABLE "Repo List"
    FILE_TMP_GIT_REPO_LIST="${DISCARDABLE_VARIABLE}"
    > ${FILE_TMP_GIT_REPO_LIST}
    generate_temp_file DISCARDABLE_VARIABLE "Git API Response"
    FILE_TMP_GIT_API_RESPONSE="${DISCARDABLE_VARIABLE}"

    log "${FUNCTION_DESCRIPTION}: Retrieving Repo List for [${GIT_ORGANIZATION_NAME}]"

    while option_enabled RETRIEVE_GIT_LOOP; do
        > ${FILE_TMP_GIT_API_RESPONSE}
        $(which curl) --retry 3 --silent --fail https://api.github.com/orgs/${GIT_ORGANIZATION_NAME}/repos?type=all\&per_page=100\&page=${TMP_PAGE}\&access_token=${GIT_API_KEY} | grep "full_name" | awk -F': "' '{print $2}' | sed -e 's/",//g' > ${FILE_TMP_GIT_API_RESPONSE} 2>${FILE_TMP_GIT_ERROR}
        RETURNVAL="$?"
        if [ ${RETURNVAL} -ne 0 ]; then
            TMP_ERROR_MSG="Failed retrieve repo list"
            log_error "${FUNCTION_DESCRIPTION}: ${TMP_ERROR_MSG}"
            log_add_from_file_content "${FILE_TMP_GIT_ERROR}" "Git Error Response"
            return $E_GIT_FAILURE
        fi
        if [[ $(wc -l <${FILE_TMP_GIT_API_RESPONSE}) -gt 0 ]]; then
            cat ${FILE_TMP_GIT_API_RESPONSE} >> ${FILE_TMP_GIT_REPO_LIST}
            TMP_PAGE=$((${TMP_PAGE}+1))
        else
            RETRIEVE_GIT_LOOP=no
        fi
    done
    cat ${FILE_TMP_GIT_REPO_LIST} | sort | uniq | sed '/^\s*$/d' > ${FILE_REPO_LIST_OUTPUT}
    TMP_REPO_COUNT="$(wc -l <${FILE_REPO_LIST_OUTPUT})"
    log_success "- Repo List generated [count::${TMP_REPO_COUNT}]"
    log_add_from_file_content "${FILE_REPO_LIST_OUTPUT}" "Repo List"
    log_success "- Repo List generated [count::${TMP_REPO_COUNT}]"
    return 0
}

###--------------------------------------------------------------------------------
## FUNCTION: is_repo_up_to_date()
## - Checks is repo is up to date (returns 0 if up to date)
## - Arguments
##   - $1: Directory containing repo
function is_repo_up_to_date() {
    local REPO_DIRECTORY="${1}"
    $(which git) -C ${REPO_DIRECTORY} fetch origin >/dev/null 2>&1
    if [[ "$ZZ_$($(which git) -C ${REPO_DIRECTORY} rev-parse HEAD 2>/dev/null)" == "$ZZ_$($(which git) -C ${REPO_DIRECTORY} rev-parse @{u} 2>/dev/null)" ]]; then
        return 0
    else
        return 1
    fi
}
