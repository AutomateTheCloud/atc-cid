#!/bin/bash
###------------------------------------------------------------------------------------------------
# alias:   cid.initialize_workspace
# script:  initialize_workspace.sh
# purpose: Initializes Azure DevOps Pipeline workspace
# version: 1.0.0
###------------------------------------------------------------------------------------------------

###------------------------------------------------------------------------------------------------
## Config
declare -r SELF_IDENTITY="initialize_workspace"
declare -r SELF_IDENTITY_H="Initialize Workspace"
declare -a ARGUMENTS_DESCRIPTION=(
    '-d <directory> : CI Properties Override Directory'
)

###------------------------------------------------------------------------------------------------
## Load Defaults
declare -r GLOBAL_CONFIG_FILE="${DIRECTORY_CID:-/opt/cid}/config/global.config"
source "${GLOBAL_CONFIG_FILE}" || exit 3

###------------------------------------------------------------------------------------------------
## Declare dependencies
REQUIRED_SOURCE_FILES+=(
    "${LIB_FUNCTIONS_CORE_CID}"
)

###------------------------------------------------------------------------------------------------
## Variables
RETURNVAL=""
TMP_ERROR_MSG=""
DIRECTORY_CI_PROPERTIES_OVERRIDE=""

###------------------------------------------------------------------------------------------------
## Main
# Process Arguments
while getopts "hVd:" OPTION; do
    case $OPTION in
        h) usage; exit 0;;
        d) DIRECTORY_CI_PROPERTIES_OVERRIDE=$OPTARG;;
        V) echo "$(return_script_version "${0}")"; exit 0;;
        *) echo "ERROR: There is an error with one or more of the arguments"; usage; exit $E_BAD_ARGS;;
        ?) echo "ERROR: There is an error with one or more of the arguments"; usage; exit $E_BAD_ARGS;;
    esac
done
start_logic
log "${SELF_IDENTITY_H}: Started"

initialize_workspace
RETURNVAL="$?"; if [ ${RETURNVAL} -ne 0 ]; then exit_logic ${RETURNVAL}; fi

if ((${#GITCRYPT_KEY[@]})); then
    log_notice "- Git-Crypt Key detected, decrypting repo"
    echo -n "${GITCRYPT_KEY}" | base64 --decode > ${DIRECTORY_WORKSPACE_INCLUDE}/gitcrypt.key
    cd ${DIRECTORY_REPOSITORY}
    $(which git-crypt) unlock ${DIRECTORY_WORKSPACE_INCLUDE}/gitcrypt.key
    > ${DIRECTORY_WORKSPACE_INCLUDE}/gitcrypt.key
    rm -f ${DIRECTORY_WORKSPACE_INCLUDE}/gitcrypt.key
fi

if(! is_empty "${DIRECTORY_CI_PROPERTIES_OVERRIDE}"); then
    FILE_CI_PROPERTIES="$($(which find) ${DIRECTORY_CI_PROPERTIES_OVERRIDE} -not -path '*/\.*' -name ci.properties | head -1 | sed -e 's/^ *//g' -e 's/ *$//g')"
else
    FILE_CI_PROPERTIES="$($(which find) ${DIRECTORY_BASE_AGENT} -not -path '*/\.*' -name ci.properties | head -1 | sed -e 's/^ *//g' -e 's/ *$//g')"
fi
if(! is_empty "${FILE_CI_PROPERTIES}"); then
    log_notice "- CI Properties file found [${FILE_CI_PROPERTIES}], loading variables"
    exec_script "${SCRIPTS_COMMANDS_LOAD_VARS_FROM_FILE}" -f "${FILE_CI_PROPERTIES}"
    RETURNVAL="$?"; if [ ${RETURNVAL} -ne 0 ]; then exit_logic ${RETURNVAL}; fi
fi

log_success "${SELF_IDENTITY_H}: Finished"
exit_logic 0
