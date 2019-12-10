###------------------------------------------------------------------------------------------------
# script:  definitions.inc
# purpose: Declaration of library source files to be used to ensure consistency.
# version: 1.0.0
###------------------------------------------------------------------------------------------------

###------------------------------------------------------------------------------------------------
## Configuration files
declare LIB_CONFIG_GLOBAL="${DIRECTORY_BASE}/config/global.config"

###------------------------------------------------------------------------------------------------
## Directories
declare DIRECTORY_CONFIG="${DIRECTORY_BASE}/config"
declare DIRECTORY_LIB="${DIRECTORY_BASE}/lib"
declare DIRECTORY_LIB_DEPENDENCIES="${DIRECTORY_LIB}/dependencies"
declare DIRECTORY_LIB_FUNCTIONS="${DIRECTORY_LIB}/functions"
declare DIRECTORY_LIB_REFERENCE="${DIRECTORY_LIB}/reference"
declare DIRECTORY_SCRIPTS="${DIRECTORY_BASE}/scripts"

###------------------------------------------------------------------------------------------------
## Distribution Files
declare DISTRIBUTION_README="${DIRECTORY_BASE}/README.md"
declare DISTRIBUTION_LICENSE="${DIRECTORY_BASE}/LICENSE.md"

###------------------------------------------------------------------------------------------------
## Reference Files
declare LIB_REFERENCE_DEFINITIONS="${DIRECTORY_BASE}/lib/reference/definitions.inc"
declare LIB_REFERENCE_DETAILS="${DIRECTORY_BASE}/lib/reference/details.inc"
declare LIB_REFERENCE_EXIT_CODES="${DIRECTORY_BASE}/lib/reference/exit_codes.inc"

###------------------------------------------------------------------------------------------------
## Dependencies
declare LIB_DEPENDENCIES_CURL_OUTPUT_FORMAT="${DIRECTORY_BASE}/lib/dependencies/curl_output.format"
declare LIB_DEPENDENCIES_CURL_OUTPUT_FORMAT_INFO="${DIRECTORY_BASE}/lib/dependencies/curl_output.format.info"

###------------------------------------------------------------------------------------------------
## Function: Core
declare LIB_FUNCTIONS_CORE_CID="${DIRECTORY_BASE}/lib/functions/core/cid.inc"
declare LIB_FUNCTIONS_CORE_COLOR="${DIRECTORY_BASE}/lib/functions/core/color.inc"
declare LIB_FUNCTIONS_CORE_COMMON="${DIRECTORY_BASE}/lib/functions/core/common.inc"
declare LIB_FUNCTIONS_CORE_LOGGING="${DIRECTORY_BASE}/lib/functions/core/logging.inc"
declare LIB_FUNCTIONS_CORE_SCRIPT_LOGIC="${DIRECTORY_BASE}/lib/functions/core/script_logic.inc"

###------------------------------------------------------------------------------------------------
## Function: Tools
declare LIB_FUNCTIONS_TOOLS_ANCHORE="${DIRECTORY_BASE}/lib/functions/tools/anchore.inc"
declare LIB_FUNCTIONS_TOOLS_CURL="${DIRECTORY_BASE}/lib/functions/tools/curl.inc"
declare LIB_FUNCTIONS_TOOLS_GIT="${DIRECTORY_BASE}/lib/functions/tools/git.inc"
declare LIB_FUNCTIONS_TOOLS_GOOGLE_AUTHENTICATOR="${DIRECTORY_BASE}/lib/functions/tools/google_authenticator.inc"
declare LIB_FUNCTIONS_TOOLS_MAIL="${DIRECTORY_BASE}/lib/functions/tools/mail.inc"
declare LIB_FUNCTIONS_TOOLS_NPM="${DIRECTORY_BASE}/lib/functions/tools/npm.inc"
declare LIB_FUNCTIONS_TOOLS_SECURITY="${DIRECTORY_BASE}/lib/functions/tools/security.inc"
declare LIB_FUNCTIONS_TOOLS_SQLITE="${DIRECTORY_BASE}/lib/functions/tools/sqlite.inc"
declare LIB_FUNCTIONS_TOOLS_SSH="${DIRECTORY_BASE}/lib/functions/tools/ssh.inc"
declare LIB_FUNCTIONS_TOOLS_TAR="${DIRECTORY_BASE}/lib/functions/tools/tar.inc"

###------------------------------------------------------------------------------------------------
## Scripts: Artifact
declare SCRIPTS_ARTIFACT_NEXUS3_DOWNLOAD_RAW="${DIRECTORY_BASE}/scripts/artifact/nexus3_download_raw.sh"
declare SCRIPTS_ARTIFACT_NEXUS3_UPLOAD_RAW="${DIRECTORY_BASE}/scripts/artifact/nexus3_upload_raw.sh"
declare SCRIPTS_ARTIFACT_NEXUS3_VALIDATE_VERSION="${DIRECTORY_BASE}/scripts/artifact/nexus3_validate_version.sh"

###------------------------------------------------------------------------------------------------
## Scripts: Commands
declare SCRIPTS_COMMANDS_ARCHIVE_CI_PROPERTIES="${DIRECTORY_BASE}/scripts/commands/archive_ci_properties.sh"
declare SCRIPTS_COMMANDS_INITIALIZE_WORKSPACE="${DIRECTORY_BASE}/scripts/commands/initialize_workspace.sh"
declare SCRIPTS_COMMANDS_LOAD_VAR_FROM_HEADER_STRING="${DIRECTORY_BASE}/scripts/commands/load_var_from_header_string.sh"
declare SCRIPTS_COMMANDS_LOAD_VARS_FROM_FILE="${DIRECTORY_BASE}/scripts/commands/load_vars_from_file.sh"

###------------------------------------------------------------------------------------------------
## Scripts: Docker
declare SCRIPTS_DOCKER_ANCHORE_EVALUATE_IMAGE="${DIRECTORY_BASE}/scripts/docker/anchore_evaluate_image.sh"
declare SCRIPTS_DOCKER_GENERATE_INFO_FILE="${DIRECTORY_BASE}/scripts/docker/generate_info_file.sh"

###------------------------------------------------------------------------------------------------
## Scripts: Git
declare SCRIPTS_GIT_TAG_BUILD="${DIRECTORY_BASE}/scripts/git/tag_build.sh"

###------------------------------------------------------------------------------------------------
## Scripts: NPM
declare SCRIPTS_NPM_GENERATE_NPMRC="${DIRECTORY_BASE}/scripts/npm/generate_npmrc.sh"
declare SCRIPTS_NPM_GENERATE_PRERELEASE_VERSION="${DIRECTORY_BASE}/scripts/npm/generate_prerelease_version.sh"
declare SCRIPTS_NPM_LOAD_NAME="${DIRECTORY_BASE}/scripts/npm/load_name.sh"
declare SCRIPTS_NPM_LOAD_VERSION="${DIRECTORY_BASE}/scripts/npm/load_version.sh"
declare SCRIPTS_NPM_LOAD_VERSION="${DIRECTORY_BASE}/scripts/npm/validate_version.sh"

###------------------------------------------------------------------------------------------------
## Scripts: SSL
declare SCRIPTS_SSL_GENERATE_ROOT_CA="${DIRECTORY_BASE}/scripts/ssl/generate_root_ca.sh"
declare SCRIPTS_SSL_GENERATE_SERVER_CERTIFICATE="${DIRECTORY_BASE}/scripts/ssl/generate_server_certificate.sh"

###------------------------------------------------------------------------------------------------
## Scripts: Utility
declare SCRIPTS_UTILITY_INSTALL="${DIRECTORY_BASE}/scripts/utility/install.sh"
declare SCRIPTS_UTILITY_README_GENERATOR="${DIRECTORY_BASE}/scripts/utility/readme_generator.sh"
declare SCRIPTS_UTILITY_REPLACE_PLACEHOLDERS="${DIRECTORY_BASE}/scripts/utility/replace_placeholders.sh"
