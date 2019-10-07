# CID
- Extends the capabilities of the CI Toolsets such as Jenkins and Azure DevOps
- Documentation Updated: 2019-09-29 15:58:43 EDT

## Table of Contents
   * [Getting Started](#getting-started)
      * [Prerequisites](#prerequisites)
      * [Installing](#installing)
   * [Scripts](#scripts)
      * [artifact](#artifact)
         * [nexus3_download_raw.sh](#nexus3_download_rawsh)
         * [nexus3_upload_raw.sh](#nexus3_upload_rawsh)
         * [nexus3_validate_version.sh](#nexus3_validate_versionsh)
      * [commands](#commands)
         * [archive_ci_properties.sh](#archive_ci_propertiessh)
         * [initialize_workspace.sh](#initialize_workspacesh)
         * [load_var_from_header_string.sh](#load_var_from_header_stringsh)
         * [load_vars_from_file.sh](#load_vars_from_filesh)
      * [docker](#docker)
         * [anchore_evaluate_image.sh](#anchore_evaluate_imagesh)
         * [generate_info_file.sh](#generate_info_filesh)
      * [git](#git)
         * [tag_build.sh](#tag_buildsh)
      * [npm](#npm)
         * [generate_npmrc.sh](#generate_npmrcsh)
         * [generate_prerelease_version.sh](#generate_prerelease_versionsh)
         * [load_name.sh](#load_namesh)
         * [load_version.sh](#load_versionsh)
         * [validate_version.sh](#validate_versionsh)
      * [utility](#utility)
         * [install.sh](#installsh)
         * [readme_generator.sh](#readme_generatorsh)
         * [replace_placeholders.sh](#replace_placeholderssh)
   * [Functions](#functions)
      * [core/cid.inc](#corecidinc)
         * [initialize_workspace()](#initialize_workspace)
         * [verify_workspace()](#verify_workspace)
      * [core/color.inc](#corecolorinc)
         * [color_lookup()](#color_lookup)
         * [color_text()](#color_text)
         * [debug_color_text()](#debug_color_text)
      * [core/common.inc](#corecommoninc)
         * [add_element_to_array()](#add_element_to_array)
         * [base64_decode()](#base64_decode)
         * [base64_encode()](#base64_encode)
         * [base64_encode_and_compress()](#base64_encode_and_compress)
         * [call_sleep()](#call_sleep)
         * [call_sleep_random()](#call_sleep_random)
         * [does_array_contain_element()](#does_array_contain_element)
         * [echo_key_value()](#echo_key_value)
         * [explode_variables_to_file()](#explode_variables_to_file)
         * [file_update_owner()](#file_update_owner)
         * [file_update_permissions()](#file_update_permissions)
         * [filesize_bytes_to_human_readable()](#filesize_bytes_to_human_readable)
         * [function_exists()](#function_exists)
         * [generate_temp_directory()](#generate_temp_directory)
         * [generate_temp_file()](#generate_temp_file)
         * [generate_uuid()](#generate_uuid)
         * [is_empty()](#is_empty)
         * [is_int()](#is_int)
         * [is_server_up()](#is_server_up)
         * [json_safe()](#json_safe)
         * [load_array_key_values_from_file()](#load_array_key_values_from_file)
         * [load_array_key_values_from_yaml_file()](#load_array_key_values_from_yaml_file)
         * [load_array_properties_from_file()](#load_array_properties_from_file)
         * [load_key_value_from_file()](#load_key_value_from_file)
         * [load_property_from_file()](#load_property_from_file)
         * [option_enabled()](#option_enabled)
         * [return_file_last_modify_timestamp()](#return_file_last_modify_timestamp)
         * [return_file_md5sum()](#return_file_md5sum)
         * [return_file_mime()](#return_file_mime)
         * [return_file_sha1sum()](#return_file_sha1sum)
         * [return_file_sha256sum()](#return_file_sha256sum)
         * [return_filesize_of_file()](#return_filesize_of_file)
         * [return_filesize_of_file_in_bytes()](#return_filesize_of_file_in_bytes)
         * [return_header_string()](#return_header_string)
         * [return_parameter_string()](#return_parameter_string)
         * [return_yaml_string()](#return_yaml_string)
         * [source_include()](#source_include)
         * [sync_disks()](#sync_disks)
         * [time_elapsed()](#time_elapsed)
         * [timestamp_to_human_readable()](#timestamp_to_human_readable)
         * [to_lower()](#to_lower)
         * [to_upper()](#to_upper)
         * [trim()](#trim)
         * [uptime_greater_than()](#uptime_greater_than)
         * [verify_array_key_values()](#verify_array_key_values)
         * [version_to_integer()](#version_to_integer)
         * [write_key_value_to_file()](#write_key_value_to_file)
      * [core/logging.inc](#corelogginginc)
         * [line_break()](#line_break)
         * [log()](#log)
         * [log_add_from_file()](#log_add_from_file)
         * [log_error()](#log_error)
         * [log_file_content()](#log_file_content)
         * [log_highlight()](#log_highlight)
         * [log_load_color_child()](#log_load_color_child)
         * [log_load_color_parent()](#log_load_color_parent)
         * [log_notice()](#log_notice)
         * [log_quiet()](#log_quiet)
         * [log_success()](#log_success)
         * [log_warning()](#log_warning)
         * [verbose_display_date()](#verbose_display_date)
      * [core/script_logic.inc](#corescript_logicinc)
         * [cleanup()](#cleanup)
         * [exec_script()](#exec_script)
         * [exit_logic()](#exit_logic)
         * [exit_logic_skip_cleanup()](#exit_logic_skip_cleanup)
         * [exit_trap()](#exit_trap)
         * [exit_trap_ensure_graceful()](#exit_trap_ensure_graceful)
         * [lookup_exit_code()](#lookup_exit_code)
         * [return_script_purpose()](#return_script_purpose)
         * [return_script_version()](#return_script_version)
         * [sort_array_required_executeables()](#sort_array_required_executeables)
         * [sort_array_required_source_files()](#sort_array_required_source_files)
         * [start_logic()](#start_logic)
         * [usage()](#usage)
         * [usage_banner()](#usage_banner)
         * [usage_normalized_arguments()](#usage_normalized_arguments)
         * [verify_dependencies()](#verify_dependencies)
      * [tools/anchore.inc](#toolsanchoreinc)
         * [anchore_image_analysis_check()](#anchore_image_analysis_check)
         * [anchore_image_analysis_queue()](#anchore_image_analysis_queue)
         * [anchore_image_analysis_wait()](#anchore_image_analysis_wait)
         * [anchore_image_evaluate()](#anchore_image_evaluate)
         * [anchore_image_list()](#anchore_image_list)
         * [anchore_report()](#anchore_report)
         * [anchore_report_image_evaluation()](#anchore_report_image_evaluation)
         * [anchore_report_image_vulnerability()](#anchore_report_image_vulnerability)
      * [tools/curl.inc](#toolscurlinc)
         * [use_curl()](#use_curl)
      * [tools/git.inc](#toolsgitinc)
         * [generate_list_organization_repo()](#generate_list_organization_repo)
         * [is_repo_up_to_date()](#is_repo_up_to_date)
      * [tools/google_authenticator.inc](#toolsgoogle_authenticatorinc)
         * [ga_generate_token()](#ga_generate_token)
         * [ga_generate_user_config()](#ga_generate_user_config)
         * [ga_get_user_token()](#ga_get_user_token)
         * [ga_load_from_file()](#ga_load_from_file)
         * [ga_put_user_token()](#ga_put_user_token)
      * [tools/mail.inc](#toolsmailinc)
         * [send_mail()](#send_mail)
      * [tools/npm.inc](#toolsnpminc)
         * [npm_does_version_exist()](#npm_does_version_exist)
         * [npm_generate_npmrc()](#npm_generate_npmrc)
      * [tools/security.inc](#toolssecurityinc)
         * [get_users()](#get_users)
         * [get_synced_users()](#get_synced_users)
         * [import_users()](#import_users)
         * [initialize_local_admin_users_group()](#initialize_local_admin_users_group)
         * [initialize_local_users_group()](#initialize_local_users_group)
         * [install_google_authenticator()](#install_google_authenticator)
         * [install_ossec()](#install_ossec)
         * [install_ssh_authorization_script()](#install_ssh_authorization_script)
         * [sanitize_username()](#sanitize_username)
         * [setup_cron_security()](#setup_cron_security)
         * [ssh_configuration()](#ssh_configuration)
         * [ssh_get_group_users()](#ssh_get_group_users)
         * [ssh_get_user_public_key()](#ssh_get_user_public_key)
         * [ssh_put_group_users()](#ssh_put_group_users)
         * [ssh_put_user_public_key()](#ssh_put_user_public_key)
         * [sudoers_create()](#sudoers_create)
         * [user_cleanup()](#user_cleanup)
         * [user_create()](#user_create)
         * [user_remove()](#user_remove)
      * [tools/sqlite.inc](#toolssqliteinc)
         * [sqlite_create_database_from_sql()](#sqlite_create_database_from_sql)
         * [sqlite_export_table()](#sqlite_export_table)
         * [sqlite_import_sql()](#sqlite_import_sql)
         * [sqlite_load_parameter()](#sqlite_load_parameter)
         * [sqlite_query()](#sqlite_query)
         * [sqlite_query_to_file()](#sqlite_query_to_file)
         * [sqlite_set_parameter()](#sqlite_set_parameter)
         * [sqlite_vacuum_db()](#sqlite_vacuum_db)
         * [sqlite_verify_db()](#sqlite_verify_db)
      * [tools/ssh.inc](#toolssshinc)
         * [prepare_ssh_response_file_for_log()](#prepare_ssh_response_file_for_log)
         * [use_ssh()](#use_ssh)
      * [tools/tar.inc](#toolstarinc)
         * [extract_tar_to_directory()](#extract_tar_to_directory)
         * [extract_tar_to_directory_compress_bzip2()](#extract_tar_to_directory_compress_bzip2)
         * [extract_tar_to_directory_compress_gzip()](#extract_tar_to_directory_compress_gzip)
         * [tar_directory()](#tar_directory)
         * [tar_directory_compress_bzip2()](#tar_directory_compress_bzip2)
         * [tar_directory_compress_gzip()](#tar_directory_compress_gzip)
         * [tar_file_compress_bzip2()](#tar_file_compress_bzip2)
***

## Getting Started
These instructions describe how to get this project running on your machines for development and/or production use.

### Prerequisites
- Linux
- BASH shell

### Dependencies
- none, apart from the standard binaries installed as part of a modern Linux Distro (tested on RHEL and Debian variants, other distros mileage may vary)
- additional dependencies are required for specific Toolsets, please see the Required Executeables detail as indicated in the Library Function files

### Installing
Extract files
```
mkdir -p /opt/cid
tar zxf cid-latest.tgz -C /opt/cid
```

Initialize Application
```
bash /opt/cid/scripts/utility/install.sh -d /usr/local/sbin
```

## Scripts
Available Scripts
> The following arguments are available to all scripts

| Argument | Description |
| --- | --- |
| -h | Show help message
| -V | Show version number and exit
***
### artifact
#### `nexus3_download_raw.sh`
Download Artifact from Nexus 3 (Raw Format)

| Argument | Description |
| --- | --- |
| -I &lt;artifact_info_file&gt; | Artifact Info file (optional)
| -r &lt;artifact_repository&gt; | Repo
| -g &lt;artifact_group_id&gt; | Group ID
| -i &lt;artifact_id&gt; | ID (Name)
| -v &lt;artifact_version&gt; | Version
| -f &lt;artifact_filename&gt; | Filename
| -c &lt;artifact_md5_checksum&gt; | MD5 Checksum (optional, if not specified, skips validation)
| -o &lt;file_output&gt; | Output File
```
script: /opt/cid/scripts/artifact/nexus3_download_raw.sh <arguments>
alias:  cid.artifact.nexus3_download_raw <arguments>
```
#### `nexus3_upload_raw.sh`
Upload Artifact to Nexus 3 (Raw Format) and generates optional artifact.info file

| Argument | Description |
| --- | --- |
| -I &lt;artifact_info_file&gt; | Artifact Info file (optional)
| -F &lt;artifact_file_to_upload&gt; | File to upload
| -r &lt;artifact_repository&gt; | Repo
| -g &lt;artifact_group_id&gt; | Group ID
| -i &lt;artifact_id&gt; | ID (Name)
| -v &lt;artifact_version&gt; | Version
| -f &lt;artifact_filename&gt; | Filename
```
script: /opt/cid/scripts/artifact/nexus3_upload_raw.sh <arguments>
alias:  cid.artifact.nexus3_upload_raw <arguments>
```
#### `nexus3_validate_version.sh`
Validates Artifact Version from Nexus 3 (Raw Format)

| Argument | Description |
| --- | --- |
| -I &lt;artifact_info_file&gt; | Artifact Info file (optional)
| -r &lt;artifact_repository&gt; | Repo
| -g &lt;artifact_group_id&gt; | Group ID
| -i &lt;artifact_id&gt; | ID (Name)
| -v &lt;artifact_version&gt; | Version
```
script: /opt/cid/scripts/artifact/nexus3_validate_version.sh <arguments>
alias:  cid.artifact.nexus3_validate_version <arguments>
```
***
### commands
#### `archive_ci_properties.sh`
Archives CI Properties file to Workspace Artifact Directory (if file exists)
```
script: /opt/cid/scripts/commands/archive_ci_properties.sh <arguments>
alias:  cid.archive_ci_properties <arguments>
```
#### `initialize_workspace.sh`
Initializes Azure DevOps Pipeline workspace

| Argument | Description |
| --- | --- |
| -d &lt;directory&gt; | CI Properties Override Directory
```
script: /opt/cid/scripts/commands/initialize_workspace.sh <arguments>
alias:  cid.initialize_workspace <arguments>
```
#### `load_var_from_header_string.sh`
Loads Variable from Comment Header in specified file

| Argument | Description |
| --- | --- |
| -f | &lt;file&gt;
| -s | &lt;string_identifier&gt;
| -v | &lt;variable_name&gt;
```
script: /opt/cid/scripts/commands/load_var_from_header_string.sh <arguments>
alias:  cid.load_var_from_header_string <arguments>
```
#### `load_vars_from_file.sh`
Loads Variables from specified Key Value file into Azure DevOps Pipeline

| Argument | Description |
| --- | --- |
| -f | &lt;key_value_file&gt;
```
script: /opt/cid/scripts/commands/load_vars_from_file.sh <arguments>
alias:  cid.load_vars_from_file <arguments>
```
***
### docker
#### `anchore_evaluate_image.sh`
Performs Security Evaluation using Anchore for specified Docker Image

| Argument | Description |
| --- | --- |
| -d &lt;docker_image&gt; | Docker Image
| -r &lt;file_report&gt; | Report File (optional)
| -f | Force Evaluation Pass (sets exit code to 0 even if there is a failure)
| -U &lt;anchore_api_url&gt; | Anchore API URL
| -u &lt;anchore_user&gt; | Anchore User
| -p &lt;anchore_password&gt; | Anchore Password
```
script: /opt/cid/scripts/docker/anchore_evaluate_image.sh <arguments>
alias:  cid.docker.anchore_evaluate_image <arguments>
```
#### `generate_info_file.sh`
Generates Docker Info file for use in CI Pipelines

| Argument | Description |
| --- | --- |
| -I &lt;docker_info_file&gt; | Docker Info file
```
script: /opt/cid/scripts/docker/generate_info_file.sh <arguments>
alias:  cid.docker.generate_info_file <arguments>
```
***
### git
#### `tag_build.sh`
Tags Git Repository with specified Tag

| Argument | Description |
| --- | --- |
| -t &lt;tag&gt; | Tag String
```
script: /opt/cid/scripts/git/tag_build.sh <arguments>
alias:  cid.git.tag_build <arguments>
```
***
### npm
#### `generate_npmrc.sh`
Generates Temporary Npmrc file

| Argument | Description |
| --- | --- |
| -r | &lt;registry_url&gt;
| -d | &lt;directory&gt;
| -f | &lt;filename_override&gt; (optional, if not specified, defaults to ".npmrc")
```
script: /opt/cid/scripts/npm/generate_npmrc.sh <arguments>
alias:  cid.npm.generate_npmrc <arguments>
```
#### `generate_prerelease_version.sh`
Generates Prerelease Version for NPM Package

| Argument | Description |
| --- | --- |
| -t | &lt;tag&gt;
| -p | &lt;prerelease_id&gt; (optional, defaults to "SNAPSHOT")
| -d | &lt;package_directory&gt;
```
script: /opt/cid/scripts/npm/generate_prerelease_version.sh <arguments>
alias:  cid.npm.generate_prerelease_version <arguments>
```
#### `load_name.sh`
Loads Package Name from specified package.json file

| Argument | Description |
| --- | --- |
| -f | &lt;file&gt;
| -v | &lt;variable_name&gt;
```
script: /opt/cid/scripts/npm/load_name.sh <arguments>
alias:  cid.npm.load_name <arguments>
```
#### `load_version.sh`
Loads Package Version from specified package.json file

| Argument | Description |
| --- | --- |
| -f | &lt;file&gt;
| -v | &lt;variable_name&gt;
```
script: /opt/cid/scripts/npm/load_version.sh <arguments>
alias:  cid.npm.load_version <arguments>
```
#### `validate_version.sh`
Validates Package Version to see if the current version has been published to the Repository

| Argument | Description |
| --- | --- |
| -p | &lt;package_name&gt;
| -v | &lt;package_version&gt;
| -d | &lt;working_directory&gt;
```
script: /opt/cid/scripts/npm/validate_version.sh <arguments>
alias:  cid.npm.validate_version <arguments>
```
***
### utility
#### `install.sh`
Installs CID Functionality

| Argument | Description |
| --- | --- |
| -d &lt;alias_links_directory&gt; | Alias Links Directory (optional, if not specified, no aliases will be generated
```
script: /opt/cid/scripts/utility/install.sh <arguments>
alias:  cid.utility.install <arguments>
```
#### `readme_generator.sh`
Scans CID application directory and generates an appropriate README.md

| Argument | Description |
| --- | --- |
| -f &lt;filename&gt; | Output file (example
```
script: /opt/cid/scripts/utility/readme_generator.sh <arguments>
alias:  cid.utility.readme_generator <arguments>
```
#### `replace_placeholders.sh`
Replace Placeholder Variables in all files in specified directory

| Argument | Description |
| --- | --- |
| -d &lt;directory&gt; | Directory to Scan
| -p &lt;file&gt; | Placeholder Var file
| -i &lt;file&gt; | Placeholder Info file
```
script: /opt/cid/scripts/utility/replace_placeholders.sh <arguments>
alias:  cid.utility.replace_placeholders <arguments>
```

## Functions
Available Functions
***
### core/cid.inc
Collection of functions related to CID

#### `initialize_workspace()`
- Prepares Workspace, removes and recreates critical workspace directories

| Arg | Description |
| --- | --- |
| none

#### `verify_workspace()`
- Verifies that the Job is running in the context of the Workspace

| Arg | Description |
| --- | --- |
| none
***
### core/color.inc
Collection of functions related to bringing some color to STDOUT

#### `color_lookup()`
- wrapper function for making curl calls
- performs retries if enabled and appropriate
- attempts to catch all curl specific return codes and attempts to handle the return code appropriately

| Arg | Description |
| --- | --- |
| $1 | color

#### `color_text()`
- returns text gently swaddled in radiant color

| Arg | Description |
| --- | --- |
| $1 | color
| $2 | text
| $3 | color to resume

#### `debug_color_text()`
- echos to screen a test line with each of the defined colors
- for debug purposes only
***
### core/common.inc
Collection of common functions

#### `add_element_to_array()`

| Arg | Description |
| --- | --- |
| $1 | Array Name
| $2 | Element

#### `base64_decode()`
- Decodes a string to base64

| Arg | Description |
| --- | --- |
| $1 | Variable name to pass info to
| $2 | encoded string

#### `base64_encode()`
- Encodes a string to base64

| Arg | Description |
| --- | --- |
| $1 | Variable name to pass info to
| $2 | string

#### `base64_encode_and_compress()`
- Encodes a string to base64 and compresses it (using gzip -9)

| Arg | Description |
| --- | --- |
| $1 | Variable name to pass info to
| $2 | string

#### `call_sleep()`
- used to call sleep

| Arg | Description |
| --- | --- |
| $1 | Number of seconds to sleep
| $2 | Reason (optional)

#### `call_sleep_random()`
- used to call sleep, will randomize the amount of seconds based on Max sleep time

| Arg | Description |
| --- | --- |
| $1 | Max sleep time (optional, defaults to 60 seconds if not specified)

#### `does_array_contain_element()`

| Arg | Description |
| --- | --- |
| $1 | Element string to search for
| $2 | Array (pass this way | "${array[@]}")

#### `echo_key_value()`
- Returns Key Value line used in variable output files

| Arg | Description |
| --- | --- |
| $1 | key
| $2 | value

#### `explode_variables_to_file()`

| Arg | Description |
| --- | --- |
| $1 | File to write to
| $2 | Delimited Variable

#### `file_update_owner()`

| Arg | Description |
| --- | --- |
| $1 | file
| $2 | owner (example | apache:apache)

#### `file_update_permissions()`

| Arg | Description |
| --- | --- |
| $1 | file
| $2 | owner (example | 0400)

#### `filesize_bytes_to_human_readable()`
- converts bytes into a human readable format (GB / MB / KB)

| Arg | Description |
| --- | --- |
| $1 | filesize in bytes (as integer)

#### `function_exists()`
- Returns true if specified function name exists, otherwise false

| Arg | Description |
| --- | --- |
| $1 | Function name

#### `generate_temp_directory()`
- creates a temporary directory and returns the name of said directory to the passed variable name

| Arg | Description |
| --- | --- |
| $1 | Variable name to pass info to
| $2 | Purpose string (optional)
| $3 | Base Directory override (optional)

#### `generate_temp_file()`
- creates a temporary file and returns the name of said file to the passed variable name

| Arg | Description |
| --- | --- |
| $1 | Variable name to pass info to
| $2 | Purpose string (optional)
| $3 | Directory override (optional)

#### `generate_uuid()`
- Creates a UUID (Removes hyphens from normal UUID format)

| Arg | Description |
| --- | --- |
| $1 | Variable name to pass output to (optional)
| $2 | String Length (optional)

#### `is_empty()`
- Checks if a variable is set (not null) and has a value.

| Arg | Description |
| --- | --- |
| $1 | Variable passed to check
| $2 | Strict Checking - will return true if the value exists but is zero (optional, defaults to yes)

#### `is_int()`
- Checks if a variable an integer

| Arg | Description |
| --- | --- |
| $1 | Variable passed to check

#### `is_server_up()`
- performs ping test against specified server to determine if server is up

| Arg | Description |
| --- | --- |
| $1 | Target server address

#### `json_safe()`
- strips out or replaces characters that would mess up the JSON structure

| Arg | Description |
| --- | --- |
| $1 | string to json_safe

#### `load_array_key_values_from_file()`
- loads Key values passed via an array from specified file

| Arg | Description |
| --- | --- |
| $1 | array
| $2 | file to load from
| $3 | Prepend String (optional, if supplied automatically adds an Underscore)
| $4 | Parse String (optional)

#### `load_array_key_values_from_yaml_file()`
- loads Key values passed via an array from specified YAML file

| Arg | Description |
| --- | --- |
| $1 | array
| $2 | file to load from
| $3 | Prepend String (optional, if supplied automatically adds an Underscore)

#### `load_array_properties_from_file()`
- loads properties variables passed via an array from specified file

| Arg | Description |
| --- | --- |
| $1 | array
| $2 | file to load from
| $3 | Prepend String (optional, if supplied automatically adds an Underscore)

#### `load_key_value_from_file()`
- Loads key value from file

| Arg | Description |
| --- | --- |
| $1 | Variable name to pass info to (optional, if not specified outputs String to STDOUT)
| $2 | Key name
| $3 | Filename
| $4 | Parse String (optional)

#### `load_property_from_file()`
- Loads property from file

| Arg | Description |
| --- | --- |
| $1 | Variable name to pass info to (optional, if not specified outputs String to STDOUT)
| $2 | Key name
| $3 | Filename

#### `option_enabled()`
- Utilizes yes/no flag for variable. Returns true if 'yes'
- Usage | "if option_enabled VERBOSE; then"  - NOTE | the argument is the variable name, not the variable (no $)

| Arg | Description |
| --- | --- |
| $1 | Variable passed to check

#### `return_file_last_modify_timestamp()`
- returns last modify timestamp of specified file

| Arg | Description |
| --- | --- |
| $1 | filename
| $2 | Variable name to pass info to (optional)

#### `return_file_md5sum()`
- returns md5 checksum of file

| Arg | Description |
| --- | --- |
| $1 | filename
| $2 | Variable name to pass info to (optional)

#### `return_file_mime()`
- returns MIME type of file

| Arg | Description |
| --- | --- |
| $1 | filename
| $2 | Variable name to pass info to (optional)

#### `return_file_sha1sum()`
- returns sha1 checksum of file

| Arg | Description |
| --- | --- |
| $1 | filename
| $2 | Variable name to pass info to (optional)

#### `return_file_sha256sum()`
- returns sha256 checksum of file

| Arg | Description |
| --- | --- |
| $1 | filename
| $2 | Variable name to pass info to (optional)

#### `return_filesize_of_file()`
- returns filesize (in human-readable format) of specified file

| Arg | Description |
| --- | --- |
| $1 | filename
| $2 | Variable name to pass info to (optional)

#### `return_filesize_of_file_in_bytes()`
- returns filesize (in bytes) of specified file

| Arg | Description |
| --- | --- |
| $1 | filename
| $2 | Variable name to pass info to (optional)

#### `return_header_string()`
- Returns Header string information from comments of file
- This keys off of the '# &lt;header_identifier&gt; | ' comment line found at the top of the scripts

| Arg | Description |
| --- | --- |
| $1 | Header Identifier
| $2 | File to inspect
| $3 | Variable name to pass info to (optional)

#### `return_parameter_string()`
- Returns Parameter string information from comments of file
- This keys off of the '## &lt;parameter_identifier&gt; | ' comment line found inside the file

| Arg | Description |
| --- | --- |
| $1 | Parameter Identifier
| $2 | File to inspect
| $3 | Variable name to pass info to (optional)

#### `return_yaml_string()`
- Returns YAML string information from file

| Arg | Description |
| --- | --- |
| $1 | Variable name to pass info to (optional, if not specified outputs String to STDOUT)
| $2 | String Identifier (Header)
| $3 | File to inspect

#### `source_include()`
- Loads specified file as source and logs it

| Arg | Description |
| --- | --- |
| $1 | Source file

#### `sync_disks()`
- Performs sync and logs it as an entry

#### `time_elapsed()`
- calculates the amount of time elapsed between two timestamps
- this function uses timestamps with microsecond precision

| Arg | Description |
| --- | --- |
| $1 | Time Start (required)
| $2 | Time End (optional, if not supplied it will calculate based on the current time)

#### `timestamp_to_human_readable()`
- converts UNIX timestamp to human readable format
- trims UNIX timestamp down to 10 characters (this allows us to pass extended timestamps (UNIX+microseconds) and not have an error

| Arg | Description |
| --- | --- |
| $1 | UNIX timestamp

#### `to_lower()`
- Converts string to lowercase

| Arg | Description |
| --- | --- |
| $1 | string to convert

#### `to_upper()`
- Converts string to uppercase

| Arg | Description |
| --- | --- |
| $1 | string to convert

#### `trim()`
- trims whitespace from left and right side of supplied string

| Arg | Description |
| --- | --- |
| $1 | string to trim

#### `uptime_greater_than()`
- Utilizes yes/no flag for variable. Returns true if 'yes'

| Arg | Description |
| --- | --- |
| $1 | Target uptime in seconds

#### `verify_array_key_values()`
- verifies Key values passed via an array are loaded into memory and are not empty

| Arg | Description |
| --- | --- |
| $1 | array
| $2 | Prepend String (optional, if supplied automatically adds an Underscore)

#### `version_to_integer()`
- verifies Key values passed via an array are loaded into memory and are not empty

| Arg | Description |
| --- | --- |
| $1 | Version String
| $2 | Variable name to pass output to (optional)

#### `write_key_value_to_file()`
- Writes [key]=value line to file

| Arg | Description |
| --- | --- |
| $1  | File to write to
| $2  | Variable Name
| $3  | Variable Value
***
### core/logging.inc
Collection of logging related functions

#### `line_break()`
- prints to screen a line that spans across the full width (columns) of the open terminal
- serves no purpose other than to be a pretty, pretty princess

#### `log()`
- echo log entry to screen

| Arg | Description |
| --- | --- |
| $1 | entry string
| $2 | Message Type
| $3 | Message Content Color Override

#### `log_add_from_file()`
- echo content of a file  to screen

| Arg | Description |
| --- | --- |
| $1 | file containing data to add to log
| $2 | string description of contents
| $3 | number of lines to display (optional, defaults to 99999)

#### `log_error()`
- echo log error entry to screen
- this function prepends "ERROR:" to the beginning of the log entry to ensure consistency in the logs

| Arg | Description |
| --- | --- |
| $1 | entry string
| $2 | Message Content Color Override

#### `log_file_content()`
- logs file content

| Arg | Description |
| --- | --- |
| $1 | Line Number
| $2 | Entry string

#### `log_highlight()`
- echo log highlight entry to screen

| Arg | Description |
| --- | --- |
| $1 | entry string

#### `log_load_color_child()`
- loads color palette for CHILD type scripts

#### `log_load_color_parent()`
- loads color palette for PARENT type scripts

#### `log_notice()`
- echo log notice entry to screen
- this function prepends "NOTICE:" to the beginning of the log entry to ensure consistency in the logs

| Arg | Description |
| --- | --- |
| $1 | entry string
| $2 | Message Content Color Override

#### `log_quiet()`
- suppresses echoing log entry to screen

| Arg | Description |
| --- | --- |
| $1 | entry string

#### `log_success()`
- echo log success entry to screen
- this function prepends "SUCCESS:" to the beginning of the log entry to ensure consistency in the logs

| Arg | Description |
| --- | --- |
| $1 | entry string
| $2 | Message Content Color Override

#### `log_warning()`
- echo log warning entry to screen
- this function prepends "WARNING:" to the beginning of the log entry to ensure consistency in the logs

| Arg | Description |
| --- | --- |
| $1 | entry string
| $2 | Message Content Color Override

#### `verbose_display_date()`
- truncates log date timestamp to HH:MM:SS only

| Arg | Description |
| --- | --- |
| $1 | timestamp
***
### core/script_logic.inc
Collection of Script Logic related functions.

#### `cleanup()`
- attempts to perform cleanup
- removes temporary files as specified in array TEMPORARY_FILES
- this will only remove files in a recognized temp directory, otherwise it issues a warning and skips the file
- performs disk sync once finished

#### `exec_script()`
- handles logic for calling a script from within a script (inception?)
- returns the exit status of the script which was ran

| Arg | Description |
| --- | --- |
| $1 | script to execute (use variables as supplied in reference_script_definitions.inc)
| $2 | arguments to pass to script

#### `exit_logic()`
- handles exit logic
- attempts to run all required cleanup

| Arg | Description |
| --- | --- |
| $1 | Exit code to return
| $2 | Exit message (optional)

#### `exit_logic_skip_cleanup()`
- handles exit logic
- skips cleanup

| Arg | Description |
| --- | --- |
| $1 | Exit code to return
| $2 | Exit message (optional)

#### `exit_trap()`
- handles exit logic when a trap is caught
- attempts to run all required cleanup

| Arg | Description |
| --- | --- |
| $1 | Exit code to return
| $2 | Exit message (optional)

#### `exit_trap_ensure_graceful()`
- handles exit logic when trap EXIT is caught
- attempts to run all required cleanup
- this is a safety feature in case a graceful exit is not already defined in the script (ie | [exit_logic 0] not included as the last line of a script)

#### `lookup_exit_code()`
- Returns human-friendly string information based on exit code integer (will return 'UNKNOWN' if it is unable to parse exitcode from file)

| Arg | Description |
| --- | --- |
| $1 | Exit Code
| $2 | File containing exit codes, optional (if not supplied, we will use the default REFERENCE_EXIT_CODES

#### `return_script_purpose()`
- Returns version number of the selected script ($1)
- This keys off of the '# purpose | ' comment line found at the top of the scripts
- Normally called in self-reference [via | $(return_script_version "${0}")]

| Arg | Description |
| --- | --- |
| $1 | File to inspect

#### `return_script_version()`
- Returns version number of the selected script ($1)
- This keys off of the '# version | ' comment line found at the top of the scripts
- Normally called in self-reference [via | $(return_script_version "${0}")]

| Arg | Description |
| --- | --- |
| $1 | File to inspect

#### `sort_array_required_executeables()`
- sorts / removes duplicates in the REQUIRED_EXECUTABLES global array

#### `sort_array_required_source_files()`
- sorts / removes duplicates in the REQUIRED_SOURCE_FILES global array

#### `start_logic()`
- handles all script startup logic
- reference code handling
- log initialization
- loads required source files (as defined in array REQUIRED_SOURCE_FILES)
- verifies dependencies (as defined in array REQUIRED_EXECUTABLES)
- should be called right after argument collection in every script

#### `usage()`
- Displays help / usage

#### `usage_banner()`
- Displays Usage Banner (Details, Version info, etc)

| Arg | Description |
| --- | --- |
| $1 | File to inspect

#### `usage_normalized_arguments()`
- displays argument list which is common for all scripts

#### `verify_dependencies()`
- validates that all required executables for the script to run are present (and accessible via PATH)
- based on elements from REQUIRED_EXECUTABLES array
***
### tools/anchore.inc
Collection of functions related to Anchore

#### `anchore_image_analysis_check()`
- Checks Analysis status of Docker Image from Anchore
- This process may take some time. It does not report to STDOUT until wait cycle is complete

| Arg | Description |
| --- | --- |
| $1 | Docker Image

#### `anchore_image_analysis_queue()`
- Queue Docker Image for Anchore Analysis
- This process may take some time. It does not report to STDOUT until wait cycle is complete

| Arg | Description |
| --- | --- |
| $1 | Docker Image

#### `anchore_image_analysis_wait()`
- Wait for Docker Image Anchore Analysis to complete

| Arg | Description |
| --- | --- |
| $1 | Docker Image

#### `anchore_image_evaluate()`
- Performs security evaluation of specified Docker Image in Anchore

| Arg | Description |
| --- | --- |
| $1 | Docker Image

#### `anchore_image_list()`
- Lists Docker images loaded into Anchore

#### `anchore_report()`
- Report Generation - Full Report

#### `anchore_report_image_evaluation()`
- Report Generation - Image Evaluation

| Arg | Description |
| --- | --- |
| $1 | Docker Image
| $2 | Output Report File

#### `anchore_report_image_vulnerability()`
- Report Generation - Image Vulnerability
***
### tools/curl.inc
Collection of functions related to curl

#### `use_curl()`
- wrapper function for making curl calls
- performs retries if enabled and appropriate
- attempts to catch all curl specific return codes and attempts to handle the return code appropriately"&lt;max_timeout&gt;"

| Arg | Description |
| --- | --- |
| $1 | URL
| $2 | Output File (optional, &lt;filename&gt;, if not specified, file/request will save to a temporary file)
| $3 | Display Data File (optional, yes/no, defaults to no if not specified. Useful for when downloading files, as not to spam the log with binary data)
| $4 | Post Data (optional)
| $5 | Authentication - Username (optional, if not specified, no Authentication credentials will be sent)
| $6 | Authentication - Password (optional)
| $7 | Authentication - Client Certificate File (optional, &lt;filename&gt;, if not specified, no certificate will be sent)
| $8 | Resume Download (optional, yes/no, defaults to yes if not specified)
| $9 | Rate Limit (optional, &lt;value, ie | 1k, 1m&gt;, defaults to CURL_DEFAULT_LIMIT_RATE [0] if not specified)
| $10 | Curl Speed Timeout (optional, defaults to CURL_DEFAULT_SPEED_TIMEOUT - this is used to prevent stalled downloads, ie | less than 1 byte/sec for SPEED_TIMEOUT seconds)
| $11 | Curl Max Timeout (optional, defaults to CURL_DEFAULT_MAX_TIMEOUT - this is used to prevent ridiculously long downloads)
***
### tools/git.inc
Collection of functions related to Git

#### `generate_list_organization_repo()`
- Loads SSL certificate and chain file

| Arg | Description |
| --- | --- |
| $1 | Organization Name
| $2 | Git API Key
| $3 | File to ouput list to

#### `is_repo_up_to_date()`
- Checks is repo is up to date (returns 0 if up to date)

| Arg | Description |
| --- | --- |
| $1 | Directory containing repo
***
### tools/google_authenticator.inc
Collection of functions related to Google Authenticator

#### `ga_generate_token()`
- Generates Google Authenticator Token for specified user and outputs details to specified Key-Value file

| Arg | Description |
| --- | --- |
| $1 | File to dump key-value data to
| $2 | Username
| $3 | Identifier (example | [SSH | Amazon])

#### `ga_generate_user_config()`
- Generates Google Authenticator Config for specified user

| Arg | Description |
| --- | --- |
| $1 | Username
| $2 | AWS Region

#### `ga_get_user_token()`
- Retrieves Google Authenticator Token for specified user from SSM Stored Parameters

| Arg | Description |
| --- | --- |
| $1 | Variable name to pass info to (optional)
| $2 | Username
| $3 | AWS Region

#### `ga_load_from_file()`

| Arg | Description |
| --- | --- |
| $1 | File to read values from

#### `ga_put_user_token()`
- Puts Google Authenticator Token for specified user in SSM Stored Parameters (Secrets)

| Arg | Description |
| --- | --- |
| $1 | Username
| $2 | Google Authenticator Token
| $3 | AWS Region
***
### tools/mail.inc
Collection of functions related to mail

#### `send_mail()`
- Sends email

| Arg | Description |
| --- | --- |
| $1 | Recipients
| $2 | Subject
| $3 | Message
| $4 | Message File (optional, if message specified)
| $5 | Attachment File (optional)
| $6 | Sender Address (optional)
***
### tools/npm.inc
Collection of functions related to NPM

#### `npm_does_version_exist()`
- Checks if NPM Package exists at the specified version against an NPM Registry
- Returns 0 if exists
- Returns E_NPM_PACKAGE_DOES_NOT_EXIST if does not exist

| Arg | Description |
| --- | --- |
| $1 | Registry URL
| $2 | Directory to Output Npmrc file
| $3 | Filename override (optional, if not specified, defaults to '.npmrc')

#### `npm_generate_npmrc()`
- Generates .npmrc file at specified location

| Arg | Description |
| --- | --- |
| $1 | Registry URL
| $2 | Directory to Output Npmrc file
| $3 | Filename override (optional, if not specified, defaults to '.npmrc')
***
### tools/security.inc
Collection of functions related to Security

#### `get_users()`
- Gets list of users from Security Secrets

| Arg | Description |
| --- | --- |
| $1 | Groups (Admin)
| $2 | Groups (View)
| $3 | AWS Region (optional)

#### `get_synced_users()`
- Gets list of already Synced Users and sets them in ARRAY_SYNCED_USERS

#### `import_users()`
- Creates Local users from imported list

| Arg | Description |
| --- | --- |
| $1 | Use Google Authenticator (yes/no)
| $2 | AWS Region

#### `initialize_local_admin_users_group()`
- Creates Group for Admin Local Users

#### `initialize_local_users_group()`
- Creates Group for Local Users

#### `install_google_authenticator()`
- Installs Google Authenticator

#### `install_ossec()`
- Installs OSSEC

| Arg | Description |
| --- | --- |
| $1 | Organization Abbreviation
| $2 | AWS Region

#### `install_ssh_authorization_script()`
- Copies SCRIPTS_COMMANDS_SSH_AUTHORIZATION to the correct location [FILE_LOCAL_SSH_AUTHORIZATION]

#### `sanitize_username()`
- Sanitizes Username UNIX use

| Arg | Description |
| --- | --- |
| $1 | Username

#### `setup_cron_security()`
- Creates cron job to Import Local Users

#### `ssh_configuration()`
- Configures /etc/pam.d/sshd and /etc/ssh/sshd_config

| Arg | Description |
| --- | --- |
| $1 | Use Google Authenticator (yes/no, defaults to no)
| $2 | Ignore MFA CIDRs list (semicolon separated)

#### `ssh_get_group_users()`
- Retrieves User List for specified Group from SSM Stored Parameters

| Arg | Description |
| --- | --- |
| $1 | Variable name to pass info to (optional)
| $2 | Group Name
| $3 | AWS Region

#### `ssh_get_user_public_key()`
- Retrieves SSH Public Key for specified user from SSM Stored Parameters

| Arg | Description |
| --- | --- |
| $1 | Variable name to pass info to (optional)
| $2 | Username
| $3 | AWS Region

#### `ssh_put_group_users()`
- Puts Users in specified group in SSM Stored Parameters (Secrets)

| Arg | Description |
| --- | --- |
| $1 | Group Name
| $2 | Users (semicolon separated)
| $3 | AWS Region

#### `ssh_put_user_public_key()`
- Puts SSH Public Key for specified user in SSM Stored Parameters (Secrets)

| Arg | Description |
| --- | --- |
| $1 | Username
| $2 | SSH Public Key (string)
| $3 | AWS Region

#### `sudoers_create()`
- Creates Sudoers file for Admin Users

#### `user_cleanup()`
- Removes Users that may have been previously synced, but are no longer valid

#### `user_create()`
- Creates or Updates User

| Arg | Description |
| --- | --- |
| $1 | Username
| $2 | Use Google Authenticator (yes/no)
| $3 | AWS Region

#### `user_remove()`
- Removes User

| Arg | Description |
| --- | --- |
| $1 | Username
***
### tools/sqlite.inc
Collection of functions related to sqlite

#### `sqlite_create_database_from_sql()`
- creates sqlite database from specified file

| Arg | Description |
| --- | --- |
| $1 | SQLite Database File
| $2 | SQL File

#### `sqlite_export_table()`
- exports sqlite database table to file

| Arg | Description |
| --- | --- |
| $1 | SQLite Database File
| $2 | Table
| $3 | File to append dump to
| $4 | Enable Logging (defaults to yes)

#### `sqlite_import_sql()`
- imports SQL statements to database from specified file

| Arg | Description |
| --- | --- |
| $1 | SQLite Database File
| $2 | SQL File for import

#### `sqlite_load_parameter()`
- loads specified parameter into specified variable
- this is for cases where we are using a SQLite database as a Key-Value store, where we have a column named 'parameter'

| Arg | Description |
| --- | --- |
| $1 | Variable name to pass output to
| $2 | SQLite Database File
| $3 | Table
| $4 | Parameter

#### `sqlite_query()`
- executes specified SQL query against SQLite database

| Arg | Description |
| --- | --- |
| $1 | SQLite Database File
| $2 | Query / Operation
| $3 | Variable name to pass output to (optional, if not passed, it will only execute the Query, and do nothing with the results
| $4 | Enable Logging (defaults to yes)
| $5 | SQLite Retry count (optional, defaults to SQLITE_DEFAULT_RETRY_COUNT)
| $6 | SQLite Retry Timer value in seconds (optional, defaults to SQLITE_DEFAULT_RETRY_TIMER_MAX_SEC)

#### `sqlite_query_to_file()`
- executes specified SQL query against SQLite database and dumps the results to a file

| Arg | Description |
| --- | --- |
| $1 | SQLite Database File
| $2 | Query / Operation
| $3 | Filename to pass query results to
| $4 | Enable Logging (defaults to yes)
| $5 | SQLite Retry count (optional, defaults to SQLITE_DEFAULT_RETRY_COUNT)
| $6 | SQLite Retry Timer value in seconds (optional, defaults to SQLITE_DEFAULT_RETRY_TIMER_MAX_SEC)

#### `sqlite_set_parameter()`
- sets parameter to specified value
- this is for cases where we are using a SQLite database as a Key-Value store, where we have a column named 'parameter'

| Arg | Description |
| --- | --- |
| $1 | SQLite Database File
| $2 | Table
| $3 | Parameter
| $4 | Value

#### `sqlite_vacuum_db()`
- performs VACUUM procedure against specified database
- this function really sucks (golf clap)

| Arg | Description |
| --- | --- |
| $1 | SQLite Database File

#### `sqlite_verify_db()`
- verifies that specified SQLite database is actually a real, functioning database and not empty or corrupt

| Arg | Description |
| --- | --- |
| $1 | SQLite Database File
| $2 | Enable Logging (defaults to yes)
***
### tools/ssh.inc
Collection of functions related to SSH

#### `prepare_ssh_response_file_for_log()`
- Cleans up the response file to remove entries that we dont need or want

| Arg | Description |
| --- | --- |
| $1 | Response file

#### `use_ssh()`

| Arg | Description |
| --- | --- |
| $1 | Remote IP
| $2 | Command to execute
| $3 | Response file (optional, if not supplied, response output will be sent to a temporary file)
| $4 | Log Response (optional, defaults to yes if not supplied. Useful for preventing log spam, yes/no)
| $5 | Identity file (optional)
| $6 | SSH retry count (optional, defaults to SSH_DEFAULT_RETRY_COUNT)
| $7 | SSH Timeout (optional, defaults to SSH_DEFAULT_TIMEOUT)
| $8 | SSH Retry Timer max value (optional, defaults to SSH_DEFAULT_RETRY_TIMER_MAX_SEC)
| $9 | SSH Operation timeout (optional, defaults to SSH_DEFAULT_OPERATION_TIMEOUT), used to kill the script if it runs for too long
| $10 | Suppress command display in the logs (optional, yes/no)
***
### tools/tar.inc
Collection of functions related to tar

#### `extract_tar_to_directory()`
- wrapper function for making tar calls
- this function does not use compression when using tar

| Arg | Description |
| --- | --- |
| $1 | file name (tar archive to extract)
| $2 | destination directory (optional, if not set, absolute path must be enabled)
| $3 | use absolute path (optional, yes/no, default | no; If yes, files will be extracted with absolute path from "/", if no, files will be extracted with relative path)

#### `extract_tar_to_directory_compress_bzip2()`
- wrapper function for making tar calls
- this function use bzip2 compression when using tar

| Arg | Description |
| --- | --- |
| $1 | file name (tar archive to extract)
| $2 | destination directory (optional, if not set, absolute path must be enabled)
| $3 | use absolute path (optional, yes/no, default | no; If yes, files will be extracted with absolute path from "/", if no, files will be extracted with relative path)

#### `extract_tar_to_directory_compress_gzip()`
- wrapper function for making tar calls
- this function use bzip2 compression when using tar

| Arg | Description |
| --- | --- |
| $1 | file name (tar archive to extract)
| $2 | destination directory (optional, if not set, absolute path must be enabled)
| $3 | use absolute path (optional, yes/no, default | no; If yes, files will be extracted with absolute path from "/", if no, files will be extracted with relative path)

#### `tar_directory()`
- wrapper function for making tar calls
- this function does not use compression when using tar

| Arg | Description |
| --- | --- |
| $1 | output file name (tar archive to create)
| $2 | target to archive
| $3 | use absolute path (optional, yes/no, default | yes; If yes, files will be archived with absolute path, if no, files will be archived with relative path)

#### `tar_directory_compress_bzip2()`
- wrapper function for making tar calls utilizing bzip2 compression
- if lbzip2 is installed, function will use that instead of bzip2

| Arg | Description |
| --- | --- |
| $1 | output file name (tar archive to create)
| $2 | target to archive
| $3 | use absolute path (optional, yes/no, default | yes; If yes, files will be archived with absolute path, if no, files will be archived with relative path)

#### `tar_directory_compress_gzip()`
- wrapper function for making tar calls utilizing gzip compression

| Arg | Description |
| --- | --- |
| $1 | output file name (tar archive to create)
| $2 | target to archive
| $3 | use absolute path (optional, yes/no, default | yes; If yes, files will be archived with absolute path, if no, files will be archived with relative path)

#### `tar_file_compress_bzip2()`
- wrapper function for making tar calls utilizing bzip2 compression
- if lbzip2 is installed, function will use that instead of bzip2

| Arg | Description |
| --- | --- |
| $1 | output file name (tar archive to create)
| $2 | target to archive
| $3 | use absolute path (optional, yes/no, default | yes; If yes, files will be archived with absolute path, if no, files will be archived with relative path)
