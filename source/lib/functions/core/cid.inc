###--------------------------------------------------------------------------------
# script:  cid.inc
# purpose: Collection of functions related to CID
# version: 1.0.3
#
# function list:
#          - initialize_workspace
#          - verify_workspace
###------------------------------------------------------------------------------------------------
REQUIRED_EXECUTABLES+=()

###--------------------------------------------------------------------------------
# Variables
DIRECTORY_BASE_AGENT=""

if option_enabled INIT_AZURE_DEVOPS; then
    if(! is_empty "${AGENT_BUILDDIRECTORY}"); then
        DIRECTORY_BASE_AGENT="$(echo "${AGENT_BUILDDIRECTORY}" | sed 's:/*$::')"
    else
        if(! is_empty "${AGENT_RELEASEDIRECTORY}"); then
            DIRECTORY_BASE_AGENT="$(echo "${AGENT_RELEASEDIRECTORY}" | sed 's:/*$::')"
        fi
    fi
    DIRECTORY_BASE_WORKSPACE="${DIRECTORY_BASE_AGENT}/workspace"
    DIRECTORY_WORKSPACE_ARTIFACTS="${DIRECTORY_BASE_WORKSPACE}/artifacts"
    DIRECTORY_WORKSPACE_COMPILED="${DIRECTORY_BASE_WORKSPACE}/compiled"
    DIRECTORY_WORKSPACE_INCLUDE="${DIRECTORY_BASE_WORKSPACE}/include"
    DIRECTORY_WORKSPACE_PACKAGE="${DIRECTORY_BASE_WORKSPACE}/package"
    DIRECTORY_WORKSPACE_TEMP="${DIRECTORY_BASE_WORKSPACE}/temp"
fi
if option_enabled INIT_JENKINS; then
    DIRECTORY_BASE_AGENT="$(echo "${WORKSPACE}" | sed 's:/*$::')"
    FILE_ENV_PROPERTIES="${DIRECTORY_BASE_AGENT}/env.properties"
    DIRECTORY_BASE_WORKSPACE="${DIRECTORY_BASE_AGENT}"
    DIRECTORY_WORKSPACE_ARTIFACTS="${DIRECTORY_BASE_WORKSPACE}/artifacts"
    DIRECTORY_WORKSPACE_COMPILED="${DIRECTORY_BASE_WORKSPACE}/compiled"
    DIRECTORY_WORKSPACE_INCLUDE="${DIRECTORY_BASE_WORKSPACE}/include"
    DIRECTORY_WORKSPACE_PACKAGE="${DIRECTORY_BASE_WORKSPACE}/package"
    DIRECTORY_WORKSPACE_TEMP="${DIRECTORY_BASE_WORKSPACE}/temp"
fi

if(! is_empty "${BUILD_REPOSITORY_LOCALPATH}"); then
    DIRECTORY_REPOSITORY="${BUILD_REPOSITORY_LOCALPATH}"
else
    if [ -d "${DIRECTORY_BASE_AGENT}/repository" ]; then
        DIRECTORY_REPOSITORY="${DIRECTORY_BASE_AGENT}/repository"
    fi
fi

###--------------------------------------------------------------------------------
## FUNCTION: initialize_workspace()
## - Prepares Workspace, removes and recreates critical workspace directories
## - Arguments
##   - none
function initialize_workspace() {
    local FUNCTION_DESCRIPTION="Workspace (Initialize)"
    local RETURNVAL=""

    log_notice "${FUNCTION_DESCRIPTION}: started"
    verify_workspace
    RETURNVAL="$?"; if [ ${RETURNVAL} -ne 0 ]; then return ${RETURNVAL}; fi

    log "${FUNCTION_DESCRIPTION}: performing cleanup"
    if option_enabled INIT_AZURE_DEVOPS; then
      if [ -d "${DIRECTORY_BASE_WORKSPACE}" ]; then
          log "- deleting previous workspace [${DIRECTORY_BASE_WORKSPACE}]"
          rm -rf ${DIRECTORY_BASE_WORKSPACE} >/dev/null 2>&1
      fi
    fi
    if option_enabled INIT_JENKINS; then
      if [ -d "${DIRECTORY_WORKSPACE_ARTIFACTS}" ]; then
          log "- deleting previous workspace [${DIRECTORY_WORKSPACE_ARTIFACTS}]"
          rm -rf ${DIRECTORY_WORKSPACE_ARTIFACTS} >/dev/null 2>&1
      fi
      if [ -d "${DIRECTORY_WORKSPACE_COMPILED}" ]; then
          log "- deleting previous workspace [${DIRECTORY_WORKSPACE_COMPILED}]"
          rm -rf ${DIRECTORY_WORKSPACE_COMPILED} >/dev/null 2>&1
      fi
      if [ -d "${DIRECTORY_WORKSPACE_INCLUDE}" ]; then
          log "- deleting previous workspace [${DIRECTORY_WORKSPACE_INCLUDE}]"
          rm -rf ${DIRECTORY_WORKSPACE_INCLUDE} >/dev/null 2>&1
      fi
      if [ -d "${DIRECTORY_WORKSPACE_PACKAGE}" ]; then
          log "- deleting previous workspace [${DIRECTORY_WORKSPACE_PACKAGE}]"
          rm -rf ${DIRECTORY_WORKSPACE_PACKAGE} >/dev/null 2>&1
      fi
      if [ -d "${DIRECTORY_WORKSPACE_TEMP}" ]; then
          log "- deleting previous workspace [${DIRECTORY_WORKSPACE_TEMP}]"
          rm -rf ${DIRECTORY_WORKSPACE_TEMP} >/dev/null 2>&1
      fi
    fi

    log "${FUNCTION_DESCRIPTION}: generating directory structure"
    $(which mkdir) -p ${DIRECTORY_WORKSPACE_ARTIFACTS} >/dev/null 2>&1
    $(which mkdir) -p ${DIRECTORY_WORKSPACE_COMPILED} >/dev/null 2>&1
    $(which mkdir) -p ${DIRECTORY_WORKSPACE_INCLUDE} >/dev/null 2>&1
    $(which mkdir) -p ${DIRECTORY_WORKSPACE_PACKAGE} >/dev/null 2>&1
    $(which mkdir) -p ${DIRECTORY_WORKSPACE_TEMP} >/dev/null 2>&1

    log "${FUNCTION_DESCRIPTION}: populating global variables"
    if option_enabled INIT_AZURE_DEVOPS; then
        echo "##vso[task.setvariable variable=workspace.directory.base]${DIRECTORY_BASE_AGENT}"
        echo "##vso[task.setvariable variable=workspace.directory.artifacts]${DIRECTORY_WORKSPACE_ARTIFACTS}"
        echo "##vso[task.setvariable variable=workspace.directory.compiled]${DIRECTORY_WORKSPACE_COMPILED}"
        echo "##vso[task.setvariable variable=workspace.directory.include]${DIRECTORY_WORKSPACE_INCLUDE}"
        echo "##vso[task.setvariable variable=workspace.directory.package]${DIRECTORY_WORKSPACE_PACKAGE}"
        echo "##vso[task.setvariable variable=workspace.directory.temp]${DIRECTORY_WORKSPACE_TEMP}"
        if(! is_empty "${DIRECTORY_REPOSITORY}"); then
            echo "##vso[task.setvariable variable=workspace.directory.repository]${DIRECTORY_REPOSITORY}"
        fi
    fi
    if option_enabled INIT_JENKINS; then
        log "- [${FILE_ENV_PROPERTIES}]"
        > ${FILE_ENV_PROPERTIES}
        echo "WORKSPACE_DIRECTORY_BASE=${DIRECTORY_BASE_AGENT}" >> ${FILE_ENV_PROPERTIES}
        echo "WORKSPACE_DIRECTORY_ARTIFACTS=${DIRECTORY_WORKSPACE_ARTIFACTS}" >> ${FILE_ENV_PROPERTIES}
        echo "WORKSPACE_DIRECTORY_COMPILED=${DIRECTORY_WORKSPACE_COMPILED}" >> ${FILE_ENV_PROPERTIES}
        echo "WORKSPACE_DIRECTORY_INCLUDE=${DIRECTORY_WORKSPACE_INCLUDE}" >> ${FILE_ENV_PROPERTIES}
        echo "WORKSPACE_DIRECTORY_PACKAGE=${DIRECTORY_WORKSPACE_PACKAGE}" >> ${FILE_ENV_PROPERTIES}
        echo "WORKSPACE_DIRECTORY_TEMP=${DIRECTORY_WORKSPACE_TEMP}" >> ${FILE_ENV_PROPERTIES}
        if(! is_empty "${DIRECTORY_REPOSITORY}"); then
            echo "WORKSPACE_DIRECTORY_REPOSITORY=${DIRECTORY_REPOSITORY}" >> ${FILE_ENV_PROPERTIES}
        fi
    fi

    line_break
    if option_enabled INIT_AZURE_DEVOPS; then
        log "- workspace.directory.base:       [${DIRECTORY_BASE_AGENT}]"
        log "- workspace.directory.artifacts:  [${DIRECTORY_WORKSPACE_ARTIFACTS}]"
        log "- workspace.directory.compiled:   [${DIRECTORY_WORKSPACE_COMPILED}]"
        log "- workspace.directory.include:    [${DIRECTORY_WORKSPACE_INCLUDE}]"
        log "- workspace.directory.package:    [${DIRECTORY_WORKSPACE_PACKAGE}]"
        log "- workspace.directory.temp:       [${DIRECTORY_WORKSPACE_TEMP}]"
        if(! is_empty "${DIRECTORY_REPOSITORY}"); then
            log "- workspace.directory.repository: [${DIRECTORY_REPOSITORY}]"
        fi
    fi
    if option_enabled INIT_JENKINS; then
        log "- WORKSPACE_DIRECTORY_BASE:       [${DIRECTORY_BASE_AGENT}]"
        log "- WORKSPACE_DIRECTORY_ARTIFACTS:  [${DIRECTORY_WORKSPACE_ARTIFACTS}]"
        log "- WORKSPACE_DIRECTORY_COMPILED:   [${DIRECTORY_WORKSPACE_COMPILED}]"
        log "- WORKSPACE_DIRECTORY_INCLUDE:    [${DIRECTORY_WORKSPACE_INCLUDE}]"
        log "- WORKSPACE_DIRECTORY_PACKAGE:    [${DIRECTORY_WORKSPACE_PACKAGE}]"
        log "- WORKSPACE_DIRECTORY_TEMP:       [${DIRECTORY_WORKSPACE_TEMP}]"
        if(! is_empty "${DIRECTORY_REPOSITORY}"); then
            log "- WORKSPACE_DIRECTORY_REPOSITORY: [${DIRECTORY_REPOSITORY}]"
        fi
    fi
    line_break

    log_success "${FUNCTION_DESCRIPTION}: finished"
    return 0
}

###--------------------------------------------------------------------------------
## FUNCTION: verify_workspace()
## - Verifies that the Job is running in the context of the Workspace
## - Arguments
##   - none
function verify_workspace() {
    local FUNCTION_DESCRIPTION="Workspace (Verify)"
    log_notice "${FUNCTION_DESCRIPTION}: started"
    if(is_empty "${DIRECTORY_BASE_AGENT}"); then
        log_error "${FUNCTION_DESCRIPTION}: Agent Build Directory not defined, cannot verify Workspace"
        return $E_BAD_ARGS
    fi
    if [ ! -d "${DIRECTORY_BASE_AGENT}" ]; then
        log_error "${FUNCTION_DESCRIPTION}: Agent Build Directory not found [${DIRECTORY_BASE_AGENT}], cannot verify Workspace"
        return $E_OBJECT_NOT_FOUND
    fi
    return 0
}