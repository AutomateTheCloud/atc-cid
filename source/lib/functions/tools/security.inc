###------------------------------------------------------------------------------------------------
# script:  security.inc
# purpose: Collection of functions related to Security
# version: 1.0.1
#
# function list:
#          - get_users
#          - get_synced_users
#          - import_users
#          - initialize_local_admin_users_group
#          - initialize_local_users_group
#          - install_google_authenticator
#          - install_ossec
#          - install_ssh_authorization_script
#          - sanitize_username
#          - setup_cron_security
#          - ssh_configuration
#          - ssh_get_group_users
#          - ssh_get_user_public_key
#          - ssh_put_group_users
#          - ssh_put_user_public_key
#          - sudoers_create
#          - user_cleanup
#          - user_create
#          - user_remove
###------------------------------------------------------------------------------------------------
REQUIRED_EXECUTABLES+=('aws')

###------------------------------------------------------------------------------------------------
# Variables
AWS_SECURITY_DEFAULT_RETRY_COUNT=3
AWS_SECURITY_DEFAULT_RETRY_TIMER_MAX_SEC=30

ARRAY_RPM_OSSEC=(
    'ossec-hids-3.2.0-6132.amzn1.art.x86_64.rpm'
    'ossec-hids-server-3.2.0-6132.amzn1.art.x86_64.rpm'
)

LOCAL_USERS_GROUP="local-users"
LOCAL_ADMIN_USERS_GROUP="local-admin-users"

SSM_PARAMETER_SECURITY_BASE="/security"
SSM_PARAMETER_SECURITY_GROUPS_BASE="${SSM_PARAMETER_SECURITY_BASE}/groups"
SSM_PARAMETER_SECURITY_USERS_BASE="${SSM_PARAMETER_SECURITY_BASE}/users"

SSM_KEYNAME_SSH_PUBLIC_KEY="ssh_public_key"

FILE_OSSEC_SERVER_CONFIG_DESTINATION="/var/ossec/etc/ossec-server.conf"
FILE_LOCAL_SSH_AUTHORIZATION="/opt/ssh_authorization.sh"
FILE_CLOUDWATCH_OSSEC_CONFIG="/etc/awslogs/config/ossec.config"
FILE_CRON_SECURITY_LOCAL_USERS="/etc/cron.d/security-local-users"

ARRAY_GROUPS=()
ARRAY_GROUPS_ADMIN=()
ARRAY_GROUPS_VIEW=()

ARRAY_SYNCED_USERS=()
ARRAY_LOCAL_USERS=()
ARRAY_LOCAL_ADMIN_USERS=()

###------------------------------------------------------------------------------------------------
## FUNCTION: get_users()
## - Gets list of users from Security Secrets
## - Arguments
##   - $1: Groups (Admin)
##   - $2: Groups (View)
##   - $3: AWS Region (optional)
function get_users() {
    local FUNCTION_DESCRIPTION="Get Users from Security Secrets"
    local TMP_GROUPS_ADMIN="${1}"
    local TMP_GROUPS_VIEW="${2}"
    local TMP_AWS_REGION="${3}"
    local TMP_USER=""
    local TMP_LOCAL_GROUP=""
    local TMP_USERS_LIST=""

    log_notice "${FUNCTION_DESCRIPTION}: Users (Admin)"
    for TMP_LOCAL_GROUP in $(echo "${TMP_GROUPS_ADMIN}" | sed "s/;/ /g"); do
        ARRAY_GROUPS+=("$(trim "${TMP_LOCAL_GROUP}")")
        ARRAY_GROUPS_ADMIN+=("$(trim "${TMP_LOCAL_GROUP}")")
        log "- ${TMP_LOCAL_GROUP}"
        ssh_get_group_users TMP_USERS_LIST "${TMP_LOCAL_GROUP}" "${TMP_AWS_REGION}"
        for TMP_USER in $(echo "${TMP_USERS_LIST}" | sed "s/;/ /g"); do
            log "-  - [$(sanitize_username "${TMP_USER}")]"
            ARRAY_LOCAL_USERS+=("$(sanitize_username "${TMP_USER}")")
            ARRAY_LOCAL_ADMIN_USERS+=("$(sanitize_username "${TMP_USER}")")
            sleep 0.5
        done
    done

    log_notice "${FUNCTION_DESCRIPTION}: Users (View)"
    for TMP_LOCAL_GROUP in $(echo "${TMP_GROUPS_VIEW}" | sed "s/;/ /g"); do
        ARRAY_GROUPS+=("$(trim "${TMP_LOCAL_GROUP}")")
        ARRAY_GROUPS_VIEW+=("$(trim "${TMP_LOCAL_GROUP}")")
        log "- ${TMP_LOCAL_GROUP}"
        ssh_get_group_users TMP_USERS_LIST "${TMP_LOCAL_GROUP}" "${TMP_AWS_REGION}"
        for TMP_USER in $(echo "${TMP_USERS_LIST}" | sed "s/;/ /g"); do
            log "-  - [$(sanitize_username "${TMP_USER}")]"
            ARRAY_LOCAL_USERS+=("$(sanitize_username "${TMP_USER}")")
            sleep 0.5
        done
    done

    readarray -t ARRAY_GROUPS < <(for a in "${ARRAY_GROUPS[@]}"; do echo "${a}"; done | sort | uniq)
    readarray -t ARRAY_GROUPS_ADMIN < <(for a in "${ARRAY_GROUPS_ADMIN[@]}"; do echo "${a}"; done | sort | uniq)
    readarray -t ARRAY_GROUPS_VIEW < <(for a in "${ARRAY_GROUPS_VIEW[@]}"; do echo "${a}"; done | sort | uniq)
    readarray -t ARRAY_LOCAL_USERS < <(for a in "${ARRAY_LOCAL_USERS[@]}"; do echo "${a}"; done | sort | uniq)
    readarray -t ARRAY_LOCAL_ADMIN_USERS < <(for a in "${ARRAY_LOCAL_ADMIN_USERS[@]}"; do echo "${a}"; done | sort | uniq)
    return 0
}

###------------------------------------------------------------------------------------------------
## FUNCTION: get_synced_users()
## - Gets list of already Synced Users and sets them in ARRAY_SYNCED_USERS
function get_synced_users() {
    local TMP_USER=""
    for TMP_USER in $($(which getent) group "${LOCAL_USERS_GROUP}" 2>/dev/null | cut -d : -f4- | sed "s/,/ /g"); do
        ARRAY_SYNCED_USERS+=("${TMP_USER}")
    done
    readarray -t ARRAY_SYNCED_USERS < <(for a in "${ARRAY_SYNCED_USERS[@]}"; do echo "${a}"; done | sort | uniq)
    return 0
}

###------------------------------------------------------------------------------------------------
## FUNCTION: import_users()
## - Creates Local users from imported list
## - Arguments
##   - $1: Use Google Authenticator (yes/no)
##   - $2: AWS Region
function import_users() {
    local FUNCTION_DESCRIPTION="Import Local Users"
    local TMP_USE_GOOGLE_AUTHENTICATOR="${1}"
    local TMP_AWS_REGION="${2}"
    local COUNTER=0
    log_notice "${FUNCTION_DESCRIPTION}: started"
    for TMP_USER in "${ARRAY_LOCAL_USERS[@]}"; do
        user_create "${TMP_USER}" "${TMP_USE_GOOGLE_AUTHENTICATOR}" "${TMP_AWS_REGION}"
        COUNTER=$((${COUNTER}+1))
    done
    log "- user_count: [${COUNTER}]"
    return 0
}

###------------------------------------------------------------------------------------------------
## FUNCTION: initialize_local_admin_users_group()
## - Creates Group for Admin Local Users
function initialize_local_admin_users_group() {
    local FUNCTION_DESCRIPTION="Initialize (Local Admin Users Group)"
    local RETURNVAL=""
    if ! $(which getent) group "${LOCAL_ADMIN_USERS_GROUP}" >/dev/null 2>&1 > /dev/null 2>&1; then
        log_notice "${FUNCTION_DESCRIPTION}: Creating Local Admin Users Group [${LOCAL_ADMIN_USERS_GROUP}]"
        $(which groupadd) "${LOCAL_ADMIN_USERS_GROUP}" >/dev/null 2>&1
        RETURNVAL="$?"
        if [ ${RETURNVAL} -ne 0 ]; then
            log_error "${FUNCTION_DESCRIPTION}: Failed to create Local Admin Users Group [${LOCAL_ADMIN_USERS_GROUP}]"
            return $E_OBJECT_FAILED_TO_CREATE
        fi
    else
        log_notice "${FUNCTION_DESCRIPTION}: Local Admin Users Group [${LOCAL_ADMIN_USERS_GROUP}] already exists, skipping"
    fi
    return 0
}

###------------------------------------------------------------------------------------------------
## FUNCTION: initialize_local_users_group()
## - Creates Group for Local Users
function initialize_local_users_group() {
    local FUNCTION_DESCRIPTION="Initialize (Local Users Group)"
    local RETURNVAL=""
    if ! $(which getent) group "${LOCAL_USERS_GROUP}" >/dev/null 2>&1 > /dev/null 2>&1; then
        log_notice "${FUNCTION_DESCRIPTION}: Creating Local Users Group [${LOCAL_USERS_GROUP}]"
        $(which groupadd) "${LOCAL_USERS_GROUP}" >/dev/null 2>&1
        RETURNVAL="$?"
        if [ ${RETURNVAL} -ne 0 ]; then
            log_error "${FUNCTION_DESCRIPTION}: Failed to create Local Users Group [${LOCAL_USERS_GROUP}]"
            return $E_OBJECT_FAILED_TO_CREATE
        fi
    else
        log_notice "${FUNCTION_DESCRIPTION}: Local Users Group [${LOCAL_USERS_GROUP}] already exists, skipping"
    fi
    return 0
}

###------------------------------------------------------------------------------------------------
## FUNCTION: install_google_authenticator()
## - Installs Google Authenticator
function install_google_authenticator() {
    local FUNCTION_DESCRIPTION="Install Google Authenticator"
    local LOG_FILE_YUM_OUTPUT=""

    if (yum_is_package_installed "google-authenticator"); then
        log_notice "${FUNCTION_DESCRIPTION}: Google Authenticator already installed"
        return 0
    fi
    log "${FUNCTION_DESCRIPTION}: Installing"
    generate_temp_file LOG_FILE_YUM_OUTPUT "yum output file"
    $(which yum) install -y google-authenticator >${LOG_FILE_YUM_OUTPUT} 2>&1
    RETURNVAL="$?"
    if [ ${RETURNVAL} -ne 0 ]; then
        TMP_ERROR_MSG="Failed to install package (Yum_Returned::${RETURNVAL})"
        log_error "- ${TMP_ERROR_MSG}"
        log_add_from_file "${LOG_FILE_YUM_OUTPUT}" "output of yum operation"
        return $E_YUM_FAILURE
    fi
    return 0
}

###------------------------------------------------------------------------------------------------
## FUNCTION: install_ossec()
## - Installs OSSEC
## - Arguments
##   - $1: Organization Abbreviation
##   - $2: AWS Region
function install_ossec() {
    local FUNCTION_DESCRIPTION="Install OSSEC"
    local TMP_ORG_ABBR="${1}"
    local TMP_AWS_REGION="${2}"
    local LOG_FILE_YUM_OUTPUT=""

    if (yum_is_package_installed "ossec-hids-server"); then
        log_notice "${FUNCTION_DESCRIPTION}: OSSEC already installed"
        return 0
    fi
    log_notice "${FUNCTION_DESCRIPTION}: Installing"
    yum_install_rpms "ARRAY_RPM_OSSEC[@]" "${DIRECTORY_LIB_DEPENDENCIES_RPM}"
    RETURNVAL="$?"
    if [ ${RETURNVAL} -ne 0 ]; then
        TMP_ERROR_MSG="Failed to install OSSEC"
        log_error "- ${TMP_ERROR_MSG}"
        return $RETURNVAL
    fi
    cat ${LIB_DEPENDENCIES_OSSEC_SERVER_CONF} > ${FILE_OSSEC_SERVER_CONFIG_DESTINATION}
    /var/ossec/bin/ossec-control restart >/dev/null 2>&1

    if(! is_empty "${TMP_ORG_ABBR}"); then
        cat << EOF > ${FILE_CLOUDWATCH_OSSEC_CONFIG}
[/var/ossec/logs/alerts/alerts.json]
file = /var/ossec/logs/alerts/alerts.json
log_group_name = ${TMP_ORG_ABBR}-${TMP_AWS_REGION}-linux-ossec
log_stream_name = {instance_id}
buffer_duration = 5000
initial_position = start_of_file
EOF
        sleep 1; sync; wait; sleep 1
        /bin/systemctl restart awslogsd.service >/dev/null 2>&1
    fi

    return 0
}

###--------------------------------------------------------------------------------
## FUNCTION: install_ssh_authorization_script()
## - Copies SCRIPTS_COMMANDS_SSH_AUTHORIZATION to the correct location [FILE_LOCAL_SSH_AUTHORIZATION]
function install_ssh_authorization_script() {
    local FUNCTION_DESCRIPTION="Install Authorized Keys Command (SSH)"
    /bin/cp -f ${SCRIPTS_COMMANDS_SSH_AUTHORIZATION} ${FILE_LOCAL_SSH_AUTHORIZATION} >/dev/null 2>&1
    $(which chmod) 0555 ${FILE_LOCAL_SSH_AUTHORIZATION} >/dev/null 2>&1
    return 0
}

###------------------------------------------------------------------------------------------------
## FUNCTION: sanitize_username()
## - Sanitizes Username UNIX use
## - Arguments
##   - $1: Username
function sanitize_username() {
    local TMP_USERNAME="${1}"
    TMP_USERNAME=${TMP_USERNAME//"+"/".plus."}
    TMP_USERNAME=${TMP_USERNAME//"="/".equal."}
    TMP_USERNAME=${TMP_USERNAME//","/".comma."}
    TMP_USERNAME=${TMP_USERNAME//"@"/".at."}
    echo -n "${TMP_USERNAME}" | sed -e 's/^ *//g' -e 's/ *$//g' | tr "[:upper:]" "[:lower:]"
}

###------------------------------------------------------------------------------------------------
## FUNCTION: setup_cron_security()
## - Creates cron job to Import Local Users
function setup_cron_security() {
    local FUNCTION_DESCRIPTION="Setup Cron Job (Security)"
    local TIME_OFFSET=$((1 + RANDOM % 58))
    log_notice "${FUNCTION_DESCRIPTION}: started"
    log "- file:     [${FILE_CRON_SECURITY_LOCAL_USERS}]"
    log "- schedule: [${TIME_OFFSET}]"

cat << EOF > ${FILE_CRON_SECURITY_LOCAL_USERS}
${TIME_OFFSET} * * * * root ${SCRIPTS_SECURITY_IMPORT_LOCAL_USERS} >/dev/null 2>&1
EOF
    sleep 1; sync; wait; sleep 1
    /bin/systemctl restart crond.service >/dev/null 2>&1
    return 0
}

###------------------------------------------------------------------------------------------------
## FUNCTION: ssh_configuration()
## - Configures /etc/pam.d/sshd and /etc/ssh/sshd_config
## - Arguments
##   - $1: Use Google Authenticator (yes/no, defaults to no)
##   - $2: Ignore MFA CIDRs list (semicolon separated)
function ssh_configuration() {
    local FUNCTION_DESCRIPTION="Config (SSH)"
    local USE_GOOGLE_AUTHENTICATOR="${1}"
    local IGNORE_MFA_CIDRS="${2}"
    if (is_empty "${USE_GOOGLE_AUTHENTICATOR}"); then
        USE_GOOGLE_AUTHENTICATOR=no
    fi
    local TMP_STRING=""
    log_notice "${FUNCTION_DESCRIPTION}: started"
    log "- Google Authenticator: [${USE_GOOGLE_AUTHENTICATOR}]"
    log "- Ignore MFA CIDRs:     [${IGNORE_MFA_CIDRS}]"

    log "${FUNCTION_DESCRIPTION}: configuring file [/etc/pam.d/sshd]"
    if option_enabled USE_GOOGLE_AUTHENTICATOR; then
cat << ZZ_EOF > /etc/pam.d/sshd
auth       [success=done default=ignore] pam_access.so accessfile=/etc/security/access-local.conf
auth       required     pam_google_authenticator.so
auth       required     pam_sepermit.so
auth       include      postlogin
-auth      optional     pam_reauthorize.so prepare
account    required     pam_nologin.so
account    include      password-auth
password   include      password-auth
session    required     pam_selinux.so close
session    required     pam_loginuid.so
session    required     pam_selinux.so open env_params
session    required     pam_namespace.so
session    optional     pam_keyinit.so force revoke
session    include      password-auth
session    include      postlogin
-session   optional     pam_reauthorize.so prepare

ZZ_EOF
    else
cat << ZZ_EOF > /etc/pam.d/sshd
auth       required     pam_sepermit.so
auth       substack     password-auth
auth       include      postlogin
-auth      optional     pam_reauthorize.so prepare
account    required     pam_nologin.so
account    include      password-auth
password   include      password-auth
session    required     pam_selinux.so close
session    required     pam_loginuid.so
session    required     pam_selinux.so open env_params
session    required     pam_namespace.so
session    optional     pam_keyinit.so force revoke
session    include      password-auth
session    include      postlogin
-session   optional     pam_reauthorize.so prepare
ZZ_EOF
    fi

    log "${FUNCTION_DESCRIPTION}: configuring file [/etc/ssh/sshd_config]"
    if option_enabled USE_GOOGLE_AUTHENTICATOR; then
cat << ZZ_EOF > /etc/ssh/sshd_config
Port 22
HostKey /etc/ssh/ssh_host_rsa_key
HostKey /etc/ssh/ssh_host_ecdsa_key
HostKey /etc/ssh/ssh_host_ed25519_key
SyslogFacility AUTHPRIV
PermitRootLogin forced-commands-only
AuthorizedKeysFile .ssh/authorized_keys
AuthorizedKeysCommand ${FILE_LOCAL_SSH_AUTHORIZATION}
AuthorizedKeysCommandUser nobody
PasswordAuthentication no
ChallengeResponseAuthentication yes
UsePAM yes
X11Forwarding yes
PrintLastLog yes
UsePrivilegeSeparation sandbox
AcceptEnv LANG LC_CTYPE LC_NUMERIC LC_TIME LC_COLLATE LC_MONETARY LC_MESSAGES
AcceptEnv LC_PAPER LC_NAME LC_ADDRESS LC_TELEPHONE LC_MEASUREMENT
AcceptEnv LC_IDENTIFICATION LC_ALL LANGUAGE
AcceptEnv XMODIFIERS
Subsystem sftp  /usr/libexec/openssh/sftp-server
AuthenticationMethods publickey,keyboard-interactive
ZZ_EOF
    else
cat << ZZ_EOF > /etc/ssh/sshd_config
Port 22
HostKey /etc/ssh/ssh_host_rsa_key
HostKey /etc/ssh/ssh_host_ecdsa_key
HostKey /etc/ssh/ssh_host_ed25519_key
SyslogFacility AUTHPRIV
PermitRootLogin forced-commands-only
AuthorizedKeysFile .ssh/authorized_keys
AuthorizedKeysCommand ${FILE_LOCAL_SSH_AUTHORIZATION}
AuthorizedKeysCommandUser nobody
PasswordAuthentication no
ChallengeResponseAuthentication no
UsePAM yes
X11Forwarding yes
PrintLastLog yes
UsePrivilegeSeparation sandbox
AcceptEnv LANG LC_CTYPE LC_NUMERIC LC_TIME LC_COLLATE LC_MONETARY LC_MESSAGES
AcceptEnv LC_PAPER LC_NAME LC_ADDRESS LC_TELEPHONE LC_MEASUREMENT
AcceptEnv LC_IDENTIFICATION LC_ALL LANGUAGE
AcceptEnv XMODIFIERS
Subsystem sftp  /usr/libexec/openssh/sftp-server
ZZ_EOF
    fi

    if option_enabled USE_GOOGLE_AUTHENTICATOR; then
        log "${FUNCTION_DESCRIPTION}: configuring file [/etc/security/access-local.conf]"
        touch /etc/security/access-local.conf
        > /etc/security/access-local.conf
        local TMP_ARRAY=(${IGNORE_MFA_CIDRS//;/ })
        for TMP_STRING in "${TMP_ARRAY[@]}"; do
            echo "+ : ALL : ${TMP_STRING}" >> /etc/security/access-local.conf
        done
        echo "+ : ALL : LOCAL" >> /etc/security/access-local.conf
        echo "+ : ec2-user : ALL" >> /etc/security/access-local.conf
        echo "- : ALL : ALL" >> /etc/security/access-local.conf
    fi
    sleep 1; sync; wait; sleep 1
    /bin/systemctl restart sshd.service >/dev/null 2>&1
    return 0
}

###------------------------------------------------------------------------------------------------
## FUNCTION: ssh_get_group_users()
## - Retrieves User List for specified Group from SSM Stored Parameters
## - Arguments
##   - $1: Variable name to pass info to (optional)
##   - $2: Group Name
##   - $3: AWS Region
function ssh_get_group_users() {
    local __RESULTVAR="${1}"
    local TMP_GROUP_NAME="${2}"
    local TMP_AWS_REGION="${3}"
    local AWS_REGION_STRING=""
    if(! is_empty "${TMP_AWS_REGION}"); then
        AWS_REGION_STRING="--region ${TMP_AWS_REGION}"
    fi
    local TMP_STRING_USERS=""

    TMP_STRING_USERS="$($(which aws) ${AWS_REGION_STRING} ssm get-parameter --name "${SSM_PARAMETER_SECURITY_GROUPS_BASE}/${TMP_GROUP_NAME}" --with-decryption --query "Parameter.[Value]" --output text 2>/dev/null)"
    sleep 0.25
    if (is_empty "${__RESULTVAR}"); then
        printf "${TMP_STRING_USERS}" | sed -e 's/^ *//g' -e 's/ *$//g'
    else
        eval $__RESULTVAR="'$(echo "$(trim "${TMP_STRING_USERS}")" | sed -e 's/^ *//g' -e 's/ *$//g')'"
    fi
}

###------------------------------------------------------------------------------------------------
## FUNCTION: ssh_get_user_public_key()
## - Retrieves SSH Public Key for specified user from SSM Stored Parameters
## - Arguments
##   - $1: Variable name to pass info to (optional)
##   - $2: Username
##   - $3: AWS Region
function ssh_get_user_public_key() {
    local __RESULTVAR="${1}"
    local TMP_USERNAME="$(sanitize_username "${2}")"
    local TMP_AWS_REGION="${3}"
    local AWS_REGION_STRING=""
    if(! is_empty "${TMP_AWS_REGION}"); then
        AWS_REGION_STRING="--region ${TMP_AWS_REGION}"
    fi
    local TMP_STRING_SSH_PUBLIC_KEY=""

    TMP_STRING_SSH_PUBLIC_KEY="$($(which aws) ${AWS_REGION_STRING} ssm get-parameter --name "${SSM_PARAMETER_SECURITY_USERS_BASE}/${TMP_USERNAME}/${SSM_KEYNAME_SSH_PUBLIC_KEY}" --with-decryption --query "Parameter.[Value]" --output text 2>/dev/null)"
    sleep 0.25
    if (is_empty "${__RESULTVAR}"); then
        printf "${TMP_STRING_SSH_PUBLIC_KEY}" | sed -e 's/^ *//g' -e 's/ *$//g'
    else
        eval $__RESULTVAR="'$(echo "$(trim "${TMP_STRING_SSH_PUBLIC_KEY}")" | sed -e 's/^ *//g' -e 's/ *$//g')'"
    fi
}

###------------------------------------------------------------------------------------------------
## FUNCTION: ssh_put_group_users()
## - Puts Users in specified group in SSM Stored Parameters (Secrets)
## - Arguments
##   - $1: Group Name
##   - $2: Users (semicolon separated)
##   - $3: AWS Region
function ssh_put_group_users() {
    local FUNCTION_DESCRIPTION="SSH (Secrets - Put Group Users)"
    local TMP_GROUP_NAME="${1}"
    local TMP_USERS_LIST="${2}"
    local TMP_AWS_REGION="${3}"

    local TMP_LOCAL_USER=""
    local TMP_AWS_ACCOUNT_NUMBER=""
    local TMP_ARN_KMS_KEY=""

    local RETURNVAL=""

    aws_metadata_account_id_from_cli TMP_AWS_ACCOUNT_NUMBER

    if(is_empty "${TMP_GROUP_NAME}"); then
        log_error "${FUNCTION_DESCRIPTION}: Group Name not specified"
        return $E_BAD_ARGS
    fi
    if(is_empty "${TMP_AWS_REGION}"); then
        log_error "${FUNCTION_DESCRIPTION}: AWS Region not specified"
        return $E_BAD_ARGS
    fi
    if(is_empty "${TMP_AWS_ACCOUNT_NUMBER}"); then
        log_error "${FUNCTION_DESCRIPTION}: AWS Account Number lookup failed"
        return $E_BAD_ARGS
    fi

    TMP_ARN_KMS_KEY="arn:aws:kms:${TMP_AWS_REGION}:${TMP_AWS_ACCOUNT_NUMBER}:alias/security-${TMP_AWS_REGION}"

    log_notice "${FUNCTION_DESCRIPTION}: started"
    log "- Group Name: [${TMP_GROUP_NAME}]"
    log "- Users:"
    for TMP_LOCAL_USER in $(echo "${TMP_USERS_LIST}" | sed "s/;/ /g"); do
        log "-           - [${TMP_LOCAL_USER}]"
    done
    log "- Region:     [${TMP_AWS_REGION}]"

    if(! is_empty "${TMP_USERS_LIST}"); then
        parameter_put "${SSM_PARAMETER_SECURITY_GROUPS_BASE}/${TMP_GROUP_NAME}" "${TMP_USERS_LIST}" "${TMP_ARN_KMS_KEY}" "Group: ${TMP_GROUP_NAME}" yes "${TMP_AWS_REGION}"
        RETURNVAL="$?"
        if [ ${RETURNVAL} -ne 0 ]; then
            log_error "- Failed to put Group Users"
            return $RETURNVAL
        fi
    else
        log_warning "- No users detected, deleting SSM Parameter"
        parameter_delete "${SSM_PARAMETER_SECURITY_GROUPS_BASE}/${TMP_GROUP_NAME}" no "${TMP_AWS_REGION}"
    fi

    log "${FUNCTION_DESCRIPTION}: successfully put Group Users"
    return 0
}

###------------------------------------------------------------------------------------------------
## FUNCTION: ssh_put_user_public_key()
## - Puts SSH Public Key for specified user in SSM Stored Parameters (Secrets)
## - Arguments
##   - $1: Username
##   - $2: SSH Public Key (string)
##   - $3: AWS Region
function ssh_put_user_public_key() {
    local FUNCTION_DESCRIPTION="SSH (Secrets - Put User Public Key)"
    local TMP_USERNAME="$(sanitize_username "${1}")"
    local TMP_SSH_PUBLIC_KEY="${2}"
    local TMP_AWS_REGION="${3}"

    local TMP_AWS_ACCOUNT_NUMBER=""
    local TMP_ARN_KMS_KEY=""

    local RETURNVAL=""

    aws_metadata_account_id_from_cli TMP_AWS_ACCOUNT_NUMBER

    if(is_empty "${TMP_USERNAME}"); then
        log_error "${FUNCTION_DESCRIPTION}: Username not specified"
        return $E_BAD_ARGS
    fi
    if(is_empty "${TMP_SSH_PUBLIC_KEY}"); then
        log_error "${FUNCTION_DESCRIPTION}: SSH Public Key not specified"
        return $E_BAD_ARGS
    fi
    if(is_empty "${TMP_AWS_REGION}"); then
        log_error "${FUNCTION_DESCRIPTION}: AWS Region not specified"
        return $E_BAD_ARGS
    fi
    if(is_empty "${TMP_AWS_ACCOUNT_NUMBER}"); then
        log_error "${FUNCTION_DESCRIPTION}: AWS Account Number lookup failed"
        return $E_BAD_ARGS
    fi

    TMP_ARN_KMS_KEY="arn:aws:kms:${TMP_AWS_REGION}:${TMP_AWS_ACCOUNT_NUMBER}:alias/security-${TMP_AWS_REGION}"

    log_notice "${FUNCTION_DESCRIPTION}: started"
    log "- Username:         [${TMP_USERNAME}]"
    log "- SSH Public Key:   [${TMP_SSH_PUBLIC_KEY}]"
    log "- Region:           [${TMP_AWS_REGION}]"

    parameter_put "${SSM_PARAMETER_SECURITY_USERS_BASE}/${TMP_USERNAME}/${SSM_KEYNAME_SSH_PUBLIC_KEY}" "${TMP_SSH_PUBLIC_KEY}" "${TMP_ARN_KMS_KEY}" "SSH Public Key for User: ${TMP_USERNAME}" yes "${TMP_AWS_REGION}"
    RETURNVAL="$?"
    if [ ${RETURNVAL} -ne 0 ]; then
        log_error "- Failed to put SSH Public Key for user"
        return $RETURNVAL
    fi

    log "${FUNCTION_DESCRIPTION}: successfully put SSH Public Key for user"
    return 0
}

###------------------------------------------------------------------------------------------------
## FUNCTION: sudoers_create()
## - Creates Sudoers file for Admin Users
function sudoers_create() {
    local FUNCTION_DESCRIPTION="Sudoers (create)"
    local TMP_SUDOERS_FILE="/etc/sudoers.d/${LOCAL_ADMIN_USERS_GROUP}"
    log_notice "${FUNCTION_DESCRIPTION}: Creating sudoers file [${TMP_SUDOERS_FILE}]"
    echo "%${LOCAL_ADMIN_USERS_GROUP} ALL=(ALL) NOPASSWD:ALL" > "${TMP_SUDOERS_FILE}"
    return 0
}

###------------------------------------------------------------------------------------------------
## FUNCTION: user_cleanup()
## - Removes Users that may have been previously synced, but are no longer valid
function user_cleanup() {
    local FUNCTION_DESCRIPTION="User (cleanup)"
    log_notice "${FUNCTION_DESCRIPTION}: Checking for invalid users"
    for TMP_USER in "${ARRAY_SYNCED_USERS[@]}"; do
        if ! does_array_contain_element "${TMP_USER}" "${ARRAY_LOCAL_USERS[@]}"; then
            log_notice "${FUNCTION_DESCRIPTION}: User no longer valid [${TMP_USER}]"
            user_remove "${TMP_USER}"
        fi
    done
    return 0
}

###------------------------------------------------------------------------------------------------
## FUNCTION: user_create()
## - Creates or Updates User
## - Arguments
##   - $1: Username
##   - $2: Use Google Authenticator (yes/no)
##   - $3: AWS Region
function user_create() {
    local FUNCTION_DESCRIPTION="User (create)"
    local TMP_USERNAME="${1}"
    local TMP_USE_GOOGLE_AUTHENTICATOR="${2}"
    local TMP_AWS_REGION="${3}"
    local RETURNVAL=""
    if(is_empty "${TMP_USERNAME}"); then
        log_error "${FUNCTION_DESCRIPTION}: Username not specified"
        return $E_BAD_ARGS
    fi
    if (is_empty "${TMP_USE_GOOGLE_AUTHENTICATOR}"); then
        TMP_USE_GOOGLE_AUTHENTICATOR=no
    fi
    local TMP_SUDOERS_FILE="/etc/sudoers.d/${TMP_USERNAME}"

    log "${FUNCTION_DESCRIPTION}: Creating User [${TMP_USERNAME}]"

    if ! id "${TMP_USERNAME}" >/dev/null 2>&1; then
        $(which useradd) --user-group --create-home --shell /bin/bash "${TMP_USERNAME}" >/dev/null 2>&1
        RETURNVAL="$?"
        if [ ${RETURNVAL} -ne 0 ]; then
            log_error "${FUNCTION_DESCRIPTION}: Failed to create user [${TMP_USERNAME}]"
            return $E_OBJECT_FAILED_TO_CREATE
        fi
    fi
    $(which chown) -R "${TMP_USERNAME}:${TMP_USERNAME}" "/home/${TMP_USERNAME}" >/dev/null 2>&1
    $(which usermod) -a -G "${LOCAL_USERS_GROUP}" "${TMP_USERNAME}" >/dev/null 2>&1

    if does_array_contain_element "${TMP_USERNAME}" "${ARRAY_LOCAL_ADMIN_USERS[@]}"; then
        log "- User is listed as an admin, adding to Admin Group"
        $(which usermod) -G ${LOCAL_USERS_GROUP},${LOCAL_ADMIN_USERS_GROUP} ${TMP_USERNAME} >/dev/null 2>&1
    else
        if [ -f "${TMP_SUDOERS_FILE}" ]; then
            log "- User is no longer an admin, removing from Admin Group"
            $(which usermod) -G ${LOCAL_USERS_GROUP} ${TMP_USERNAME} >/dev/null 2>&1
        fi
    fi
    if option_enabled TMP_USE_GOOGLE_AUTHENTICATOR; then
        ga_generate_user_config "${TMP_USERNAME}" "${TMP_AWS_REGION}"
    fi
    return 0
}

###------------------------------------------------------------------------------------------------
## FUNCTION: user_remove()
## - Removes User
## - Arguments
##   - $1: Username
function user_remove() {
    local FUNCTION_DESCRIPTION="User (remove)"
    local TMP_USERNAME="${1}"
    if(is_empty "${TMP_USERNAME}"); then
        log_error "${FUNCTION_DESCRIPTION}: Username not specified"
        return $E_BAD_ARGS
    fi
    local TMP_SUDOERS_FILE="/etc/sudoers.d/${TMP_USERNAME}"
    log "${FUNCTION_DESCRIPTION}: Removing User [${TMP_USERNAME}]"
    $(which usermod) -L -s /sbin/nologin "${TMP_USERNAME}" >/dev/null 2>&1
    $(which pkill) -15 -u "${TMP_USERNAME}" >/dev/null 2>&1
    call_sleep 5
    $(which pkill) -9 -u "${TMP_USERNAME}" >/dev/null 2>&1
    call_sleep 1
    sync_disks
    $(which userdel) -f -r "${TMP_USERNAME}" >/dev/null 2>&1
    sync_disks
    rm -rf /home/${TMP_USERNAME} >/dev/null 2>&1
    if [ -f "${TMP_SUDOERS_FILE}" ]; then
        sudoers_remove "${TMP_USERNAME}"
    fi
    return 0
}
