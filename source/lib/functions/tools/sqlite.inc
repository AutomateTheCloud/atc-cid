###------------------------------------------------------------------------------------------------
# script:  sqlite.inc
# purpose: Collection of functions related to sqlite
# version: 1.0.0
#
# function list:
#          - sqlite_create_database_from_sql
#          - sqlite_export_table
#          - sqlite_import_sql
#          - sqlite_load_parameter
#          - sqlite_query
#          - sqlite_query_to_file
#          - sqlite_set_parameter
#          - sqlite_vacuum_db
#          - sqlite_verify_db
###------------------------------------------------------------------------------------------------
REQUIRED_EXECUTABLES+=('sqlite3')

###------------------------------------------------------------------------------------------------
# Variables
SQLITE_DEFAULT_RETRY_COUNT=3
SQLITE_DEFAULT_RETRY_TIMER_MAX_SEC=5

###------------------------------------------------------------------------------------------------
## FUNCTION: sqlite_create_database_from_sql()
## - creates sqlite database from specified file
## - Arguments
##   - $1: SQLite Database File
##   - $2: SQL File
function sqlite_create_database_from_sql() {
    local FILE_SQLITE_DB="${1}"
    local FILE_SQL_SOURCE="${2}"
    local RETURNVAL=""

    log "SQLite Create DB: Attempting to create DB [Database::${FILE_SQLITE_DB} / SQL::${FILE_SQL_SOURCE}]"

    # Verify that SQL file exists
    if [ ! -f "${FILE_SQL_SOURCE}" ]; then
        log_error "SQLite Create DB: SQL Source file not found [SQL::${FILE_SQL_SOURCE}]"
        return $E_OBJECT_NOT_FOUND
    fi

    # Attempt to create DB skeleton
    $(which touch) ${FILE_SQLITE_DB} 2>/dev/null
    RETURNVAL="$?"
    if [ ${RETURNVAL} -ne 0 ]; then
        log_error "SQLite Create DB: Failed to create Database skeleton [Database::${FILE_SQLITE_DB} / touch_Returned::${RETURNVAL}]"
        return $E_OBJECT_FAILED_TO_CREATE
    fi
    # Empty Database
    > ${FILE_SQLITE_DB}

    # Execute SQL
    log "SQLite Create DB: Executing SQL"
    generate_temp_file DISCARDABLE_VARIABLE "sqlite stderr log"
    SQLITE_STDERR_FILE="${DISCARDABLE_VARIABLE}"
    $(which sqlite3) ${FILE_SQLITE_DB} < ${FILE_SQL_SOURCE} 2>${SQLITE_STDERR_FILE}
    RETURNVAL="$?"
    if [ ${RETURNVAL} -ne 0 ]; then
        log_error "SQLite Create DB: Database creation failed [sqlite_Returned::${RETURNVAL}]"
        log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Create DB: STDERR from previous operation";
        return $E_OBJECT_FAILED_TO_CREATE
    fi
    sqlite_vacuum_db "${FILE_SQLITE_DB}"
    sqlite_verify_db "${FILE_SQLITE_DB}"
    RETURNVAL="$?"
    if [ ${RETURNVAL} -ne 0 ]; then
        log_error "SQLite Create DB: SQL execution was successful, however the database appears to be corrupted, removing database and aborting operation"
        > ${FILE_SQLITE_DB}
        rm -f ${FILE_SQLITE_DB}
        return $E_OBJECT_FAILED_TO_CREATE
    fi

    log "SQLite Create DB: Successfully created database"
    return 0
}

###------------------------------------------------------------------------------------------------
## FUNCTION: sqlite_export_table()
## - exports sqlite database table to file
## - Arguments
##   - $1: SQLite Database File
##   - $2: Table
##   - $3: File to append dump to
##   - $4: Enable Logging (defaults to yes)
function sqlite_export_table() {
    local FILE_SQLITE_DB="${1}"
    local TABLE_TO_EXPORT="${2}"
    local TMP_FILE_DUMP="${3}"
    local SQLITE_LOGGING="${4}"
    if(is_empty "${SQLITE_LOGGING}"); then
        SQLITE_LOGGING="yes"
    fi
    local RETURNVAL=""

    if(is_empty "${FILE_SQLITE_DB}"); then
        log_error "SQLite Export Table: SQLite Database not specified"
        return $E_OBJECT_NOT_FOUND
    fi
    if(is_empty "${TMP_FILE_DUMP}"); then
        log_error "SQLite Export Table: Dump file not specified"
        return $E_OBJECT_NOT_FOUND
    fi
    if [ ! -f "${TMP_FILE_DUMP}" ]; then
        log_error "SQLite Export Table: Dump file not found [${TMP_FILE_DUMP}]"
        return $E_OBJECT_NOT_FOUND
    fi
    if(is_empty "${TABLE_TO_EXPORT}"); then
        log_warning "SQLite Export Table: Table not specified, assuming all tables"
        log "SQLite Export Table: Exporting table [Table::ALL_TABLES]"
    else
        log "SQLite Export Table: Exporting table [Table::${TABLE_TO_EXPORT}]"
    fi

    generate_temp_file DISCARDABLE_VARIABLE "sqlite table dump"
    local TMP_SQLITE_TABLE_DUMP="${DISCARDABLE_VARIABLE}"

    sqlite_query_to_file "${FILE_SQLITE_DB}" ".dump ${TABLE_TO_EXPORT}" "${TMP_SQLITE_TABLE_DUMP}" "${SQLITE_LOGGING}"
    RETURNVAL="$?"
    if [ ${RETURNVAL} -ne 0 ]; then
        log_error "SQLite Export Table: Failed to export table [sqlite_Returned::${RETURNVAL}]"
        return $E_SQLITE_FAILURE
    fi
    log "SQLite Export Table: Outputting results to file [${TMP_FILE_DUMP}]"
    cat ${TMP_SQLITE_TABLE_DUMP} | grep '^INSERT INTO '>> ${TMP_FILE_DUMP}
    return 0
}

###------------------------------------------------------------------------------------------------
## FUNCTION: sqlite_import_sql()
## - imports SQL statements to database from specified file
## - Arguments
##   - $1: SQLite Database File
##   - $2: SQL File for import
function sqlite_import_sql() {
    local FILE_SQLITE_DB="${1}"
    local FILE_SQL_SOURCE="${2}"
    local TMP_DATABASE_BACKUP=""
    local RETURNVAL=""

    log "SQLite Import SQL: Attempting to Import SQL [Database::${FILE_SQLITE_DB} / SQL::${FILE_SQL_SOURCE}]"

    # Verify that SQL file exists
    if [ ! -f "${FILE_SQL_SOURCE}" ]; then
        log_error "SQLite Create DB: SQL Source file not found [SQL::${FILE_SQL_SOURCE}]"
        return $E_OBJECT_NOT_FOUND
    fi

    log "SQLite Import SQL: Temporarily backing up database"
    generate_temp_file DISCARDABLE_VARIABLE "sqlite stderr log"
    TMP_DATABASE_BACKUP="${DISCARDABLE_VARIABLE}"
    log "SQLite Import SQL: Copying to backup [${FILE_SQLITE_DB} --> ${TMP_DATABASE_BACKUP}]"
    cp -f ${FILE_SQLITE_DB} ${TMP_DATABASE_BACKUP}

    # Execute SQL
    log "SQLite Import SQL: Executing SQL"
    generate_temp_file DISCARDABLE_VARIABLE "sqlite stderr log"
    SQLITE_STDERR_FILE="${DISCARDABLE_VARIABLE}"
    $(which sqlite3) ${FILE_SQLITE_DB} < ${FILE_SQL_SOURCE} 2>${SQLITE_STDERR_FILE}
    RETURNVAL="$?"
    if [ ${RETURNVAL} -ne 0 ]; then
        log_error "SQLite Import SQL: SQL execution failed [sqlite_Returned::${RETURNVAL}]"
        log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Import SQL: STDERR from previous operation";
        log "SQLite Import SQL: Rolling back database [${TMP_DATABASE_BACKUP} --> ${FILE_SQLITE_DB}]"
        cp -f ${TMP_DATABASE_BACKUP} ${FILE_SQLITE_DB}
        sync_disks
        return $E_SQLITE_FAILURE
    fi
    sqlite_vacuum_db "${FILE_SQLITE_DB}"
    sqlite_verify_db "${FILE_SQLITE_DB}"
    RETURNVAL="$?"
    if [ ${RETURNVAL} -ne 0 ]; then
        log_error "SQLite Import SQL: SQL execution was successful, however the database appears to be corrupted, removing database and aborting operation"
        log "SQLite Import SQL: Rolling back database [${TMP_DATABASE_BACKUP} --> ${FILE_SQLITE_DB}]"
        cp -f ${TMP_DATABASE_BACKUP} ${FILE_SQLITE_DB}
        sync_disks
        return $E_SQLITE_FAILURE
    fi
    log "SQLite Import SQL: Successfully executed SQL"
    return 0
}

###------------------------------------------------------------------------------------------------
## FUNCTION: sqlite_load_parameter()
## - loads specified parameter into specified variable
## - this is for cases where we are using a SQLite database as a Key-Value store, where we have a column named 'parameter'
## - Arguments
##   - $1: Variable name to pass output to
##   - $2: SQLite Database File
##   - $3: Table
##   - $4: Parameter
function sqlite_load_parameter() {
    local __RESULTVAR="${1}"
    local RESULT_VALUE=""
    local RETURNVAL=""
    sqlite_query "${2}" "SELECT value FROM ${3} WHERE parameter=\"${4}\" LIMIT 1" RESULT_VALUE no
    RETURNVAL="$?"
    if [ ${RETURNVAL} -ne 0 ]; then
        log_error "SQLite Load Parameter: Failed to load parameter [sqlite_Returned::${RETURNVAL}]"
        return $E_SQLITE_FAILURE
    fi
    eval $__RESULTVAR="'${RESULT_VALUE}'"
    return 0
}

###------------------------------------------------------------------------------------------------
## FUNCTION: sqlite_query()
## - executes specified SQL query against SQLite database
## - Arguments
##   - $1: SQLite Database File
##   - $2: Query / Operation
##   - $3: Variable name to pass output to (optional, if not passed, it will only execute the Query, and do nothing with the results
##   - $4: Enable Logging (defaults to yes)
##   - $5: SQLite Retry count (optional, defaults to SQLITE_DEFAULT_RETRY_COUNT)
##   - $6: SQLite Retry Timer value in seconds (optional, defaults to SQLITE_DEFAULT_RETRY_TIMER_MAX_SEC)
function sqlite_query() {
    local SQLITE_FILE="${1}"
    local QUERY="${2}"
    local __RESULTVAR="${3}"
    local SQLITE_LOGGING="${4}"
    if(is_empty "${SQLITE_LOGGING}"); then
        SQLITE_LOGGING="yes"
    fi
    local SQLITE_RETRY_COUNT="${5}"
    if(is_empty "${SQLITE_RETRY_COUNT}"); then
        SQLITE_RETRY_COUNT="${SQLITE_DEFAULT_RETRY_COUNT}"
    fi
    local SQLITE_RETRY_TIMER_MAX_SEC="${6}"
    if(is_empty "${SQLITE_RETRY_TIMER_MAX_SEC}"); then
        SQLITE_RETRY_TIMER_MAX_SEC="${SQLITE_DEFAULT_RETRY_TIMER_MAX_SEC}"
    fi
    local OUTPUT=""
    local RUN=1
    local COUNTER=1
    local RETURNVAL=""
    if option_enabled SQLITE_LOGGING; then
        log "SQLite Query: [${SQLITE_FILE}] - [${QUERY}]"
        generate_temp_file DISCARDABLE_VARIABLE "sqlite stderr log"
        SQLITE_STDERR_FILE="${DISCARDABLE_VARIABLE}"
        while [ "${RUN}" = "1" ]; do
            if [ ${COUNTER} -gt ${SQLITE_RETRY_COUNT} ]; then
                log_error "SQLite Query: Retry count exceeded, aborting operation"
                return $E_SQLITE_FAILURE
            fi
            OUTPUT=$($(which sqlite3) "${SQLITE_FILE}" "${QUERY}" 2>${SQLITE_STDERR_FILE})
            RETURNVAL="$?"
            case $RETURNVAL in
                0)   RUN=0;;
                1)   log_error "SQLite Query: Returned 1 [SQLITE_ERROR - SQL error or missing database]. Aborting operation"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                2)   log_error "SQLite Query: Returned 2 [SQLITE_INTERNAL - Internal logic error in SQLite]. Aborting operation"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                3)   log_error "SQLite Query: Returned 3 [SQLITE_PERM - Access permission denied]. Aborting operation"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                4)   log_warning "SQLite Query: Returned 4 [SQLITE_ABORT - Callback routine requested an abort]. Retrying"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; call_sleep_random ${SQLITE_RETRY_TIMER_MAX_SEC};;
                5)   log_warning "SQLite Query: Returned 5 [SQLITE_BUSY - The database file is locked]. Retrying"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; call_sleep_random ${SQLITE_RETRY_TIMER_MAX_SEC};;
                6)   log_warning "SQLite Query: Returned 6 [SQLITE_LOCKED - A table in the database is locked]. Retrying"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; call_sleep_random ${SQLITE_RETRY_TIMER_MAX_SEC};;
                7)   log_warning "SQLite Query: Returned 7 [SQLITE_NOMEM - A malloc() failed]. Retrying"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; call_sleep_random ${SQLITE_RETRY_TIMER_MAX_SEC};;
                8)   log_error "SQLite Query: Returned 8 [SQLITE_READONLY - Attempt to write a readonly database]. Aborting operation"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                9)   log_warning "SQLite Query: Returned 9 [SQLITE_INTERRUPT - Operation terminated by sqlite3_interrupt()]. Retrying"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; call_sleep_random ${SQLITE_RETRY_TIMER_MAX_SEC};;
                10)  log_warning "SQLite Query: Returned 10 [SQLITE_IOERR - Some kind of disk I/O error occurred]. Retrying"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; call_sleep_random ${SQLITE_RETRY_TIMER_MAX_SEC};;
                11)  log_error "SQLite Query: Returned 11 [SQLITE_CORRUPT - The database disk image is malformed]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                12)  log_error "SQLite Query: Returned 12 [SQLITE_NOTFOUND - Unknown opcode in sqlite3_file_control()]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                13)  log_error "SQLite Query: Returned 13 [SQLITE_FULL - Insertion failed because database is full]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                14)  log_error "SQLite Query: Returned 14 [SQLITE_CANTOPEN - Unable to open the database file]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                15)  log_error "SQLite Query: Returned 15 [SQLITE_PROTOCOL - Database lock protocol error]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                16)  log_error "SQLite Query: Returned 16 [SQLITE_EMPTY - Database is empty]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                17)  log_error "SQLite Query: Returned 17 [SQLITE_SCHEMA - The database schema changed]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                18)  log_error "SQLite Query: Returned 18 [SQLITE_TOOBIG - String or BLOB exceeds size limit]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                19)  log_error "SQLite Query: Returned 19 [SQLITE_CONSTRAINT - Abort due to constraint violation]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                20)  log_error "SQLite Query: Returned 20 [SQLITE_MISMATCH - Data type mismatch]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                21)  log_error "SQLite Query: Returned 21 [SQLITE_MISUSE - Library used incorrectly]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                22)  log_error "SQLite Query: Returned 22 [SQLITE_NOLFS - Uses OS features not supported on host]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                23)  log_error "SQLite Query: Returned 23 [SQLITE_AUTH - Authorization denied]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                24)  log_error "SQLite Query: Returned 24 [SQLITE_FORMAT - Auxiliary database format error]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                25)  log_error "SQLite Query: Returned 25 [SQLITE_RANGE - 2nd parameter to sqlite3_bind out of range]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                26)  log_error "SQLite Query: Returned 26 [SQLITE_NOTADB - File opened that is not a database file]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                27)  log_error "SQLite Query: Returned 27 [SQLITE_NOTICE - Notifications from sqlite3_log()]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                28)  log_error "SQLite Query: Returned 28 [SQLITE_WARNING - Warnings from sqlite3_log()]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                100) log_error "SQLite Query: Returned 100 [SQLITE_ROW - sqlite3_step() has another row ready]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                101) log_error "SQLite Query: Returned 101 [SQLITE_DONE - sqlite3_step() has finished executing]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                *)   log_error "SQLite Query: Returned $RETURNVAL [Unknown - This result code is not listed by SQLite, it is possible the result codes have changed]. Aborting operation"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                ?)   log_error "SQLite Query: Returned $RETURNVAL [Unknown - This result code is not listed by SQLite, it is possible the result codes have changed]. Aborting operation"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
            esac
            COUNTER=$((${COUNTER}+1))
        done
        log "SQLite Query: Returned success"
        if (! is_empty "$__RESULTVAR"); then
            log "SQLite Query: loading results into variable: ${__RESULTVAR}"
            local SINGLE_LINE_OUTPUT="$(echo "${OUTPUT[@]}" | sed 's/^[ \t]*//;s/[ \t]*$//')"
            eval $__RESULTVAR="'${SINGLE_LINE_OUTPUT}'"
        fi
        return 0
    else
        # BE QUIET SQLITE!
        while [ "${RUN}" = "1" ]; do
            if [ ${COUNTER} -gt ${SQLITE_RETRY_COUNT} ]; then
                return $E_SQLITE_FAILURE
            fi
            OUTPUT=$($(which sqlite3) "${SQLITE_FILE}" "${QUERY}" 2>/dev/null)
            RETURNVAL="$?"
            case $RETURNVAL in
                0)   RUN=0;;
                1)   return $E_SQLITE_FAILURE;;
                2)   return $E_SQLITE_FAILURE;;
                3)   return $E_SQLITE_FAILURE;;
                4)   call_sleep_random ${SQLITE_RETRY_TIMER_MAX_SEC};;
                5)   call_sleep_random ${SQLITE_RETRY_TIMER_MAX_SEC};;
                6)   call_sleep_random ${SQLITE_RETRY_TIMER_MAX_SEC};;
                7)   call_sleep_random ${SQLITE_RETRY_TIMER_MAX_SEC};;
                8)   return $E_SQLITE_FAILURE;;
                9)   call_sleep_random ${SQLITE_RETRY_TIMER_MAX_SEC};;
                10)  call_sleep_random ${SQLITE_RETRY_TIMER_MAX_SEC};;
                11)  return $E_SQLITE_FAILURE;;
                12)  return $E_SQLITE_FAILURE;;
                13)  return $E_SQLITE_FAILURE;;
                14)  return $E_SQLITE_FAILURE;;
                15)  return $E_SQLITE_FAILURE;;
                16)  return $E_SQLITE_FAILURE;;
                17)  return $E_SQLITE_FAILURE;;
                18)  return $E_SQLITE_FAILURE;;
                19)  return $E_SQLITE_FAILURE;;
                20)  return $E_SQLITE_FAILURE;;
                21)  return $E_SQLITE_FAILURE;;
                22)  return $E_SQLITE_FAILURE;;
                23)  return $E_SQLITE_FAILURE;;
                24)  return $E_SQLITE_FAILURE;;
                25)  return $E_SQLITE_FAILURE;;
                26)  return $E_SQLITE_FAILURE;;
                27)  return $E_SQLITE_FAILURE;;
                28)  return $E_SQLITE_FAILURE;;
                100) return $E_SQLITE_FAILURE;;
                101) return $E_SQLITE_FAILURE;;
                *)   return $E_SQLITE_FAILURE;;
                ?)   return $E_SQLITE_FAILURE;;
            esac
            COUNTER=$((${COUNTER}+1))
        done
        if (! is_empty "$__RESULTVAR"); then
            local SINGLE_LINE_OUTPUT="$(echo "${OUTPUT[@]}" | sed 's/^[ \t]*//;s/[ \t]*$//')"
            eval $__RESULTVAR="'${SINGLE_LINE_OUTPUT}'"
        fi
        return 0
    fi
}

###------------------------------------------------------------------------------------------------
## FUNCTION: sqlite_query_to_file()
## - executes specified SQL query against SQLite database and dumps the results to a file
## - Arguments
##   - $1: SQLite Database File
##   - $2: Query / Operation
##   - $3: Filename to pass query results to
##   - $4: Enable Logging (defaults to yes)
##   - $5: SQLite Retry count (optional, defaults to SQLITE_DEFAULT_RETRY_COUNT)
##   - $6: SQLite Retry Timer value in seconds (optional, defaults to SQLITE_DEFAULT_RETRY_TIMER_MAX_SEC)
function sqlite_query_to_file() {
    local SQLITE_FILE="${1}"
    local QUERY="${2}"
    local TMP_TARGET_FILE="${3}"
    local SQLITE_LOGGING="${4}"
    if(is_empty "${SQLITE_LOGGING}"); then
        SQLITE_LOGGING="yes"
    fi
    local SQLITE_RETRY_COUNT="${5}"
    if(is_empty "${SQLITE_RETRY_COUNT}"); then
        SQLITE_RETRY_COUNT="${SQLITE_DEFAULT_RETRY_COUNT}"
    fi
    local SQLITE_RETRY_TIMER_MAX_SEC="${6}"
    if(is_empty "${SQLITE_RETRY_TIMER_MAX_SEC}"); then
        SQLITE_RETRY_TIMER_MAX_SEC="${SQLITE_DEFAULT_RETRY_TIMER_MAX_SEC}"
    fi
    if(is_empty "${TMP_TARGET_FILE}"); then
        log_error "SQLite Export Table: Dump file not specified"
        return $E_OBJECT_NOT_FOUND
    fi
    if [ ! -f "${TMP_TARGET_FILE}" ]; then
        log_error "SQLite Export Table: Dump file not found [${TMP_FILE_DUMP}]"
        return $E_OBJECT_NOT_FOUND
    fi
    local OUTPUT=""
    local RUN=1
    local COUNTER=1
    local RETURNVAL=""
    if option_enabled SQLITE_LOGGING; then
        log "SQLite Query to File: [${SQLITE_FILE}] - [${QUERY}]"
        generate_temp_file DISCARDABLE_VARIABLE "sqlite stderr log"
        SQLITE_STDERR_FILE="${DISCARDABLE_VARIABLE}"
        while [ "${RUN}" = "1" ]; do
            if [ ${COUNTER} -gt ${SQLITE_RETRY_COUNT} ]; then
                log_error "SQLite Query to File: Retry count exceeded, aborting operation"
                return $E_SQLITE_FAILURE
            fi
            OUTPUT=$($(which sqlite3) "${SQLITE_FILE}" "${QUERY}" 2>${SQLITE_STDERR_FILE})
            RETURNVAL="$?"
            case $RETURNVAL in
                0)   RUN=0;;
                1)   log_error "SQLite Query to File: Returned 1 [SQLITE_ERROR - SQL error or missing database]. Aborting operation"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                2)   log_error "SQLite Query to File: Returned 2 [SQLITE_INTERNAL - Internal logic error in SQLite]. Aborting operation"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                3)   log_error "SQLite Query to File: Returned 3 [SQLITE_PERM - Access permission denied]. Aborting operation"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                4)   log_warning "SQLite Query to File: Returned 4 [SQLITE_ABORT - Callback routine requested an abort]. Retrying"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; call_sleep_random ${SQLITE_RETRY_TIMER_MAX_SEC};;
                5)   log_warning "SQLite Query to File: Returned 5 [SQLITE_BUSY - The database file is locked]. Retrying"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; call_sleep_random ${SQLITE_RETRY_TIMER_MAX_SEC};;
                6)   log_warning "SQLite Query to File: Returned 6 [SQLITE_LOCKED - A table in the database is locked]. Retrying"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; call_sleep_random ${SQLITE_RETRY_TIMER_MAX_SEC};;
                7)   log_warning "SQLite Query to File: Returned 7 [SQLITE_NOMEM - A malloc() failed]. Retrying"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; call_sleep_random ${SQLITE_RETRY_TIMER_MAX_SEC};;
                8)   log_error "SQLite Query to File: Returned 8 [SQLITE_READONLY - Attempt to write a readonly database]. Aborting operation"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                9)   log_warning "SQLite Query to File: Returned 9 [SQLITE_INTERRUPT - Operation terminated by sqlite3_interrupt()]. Retrying"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; call_sleep_random ${SQLITE_RETRY_TIMER_MAX_SEC};;
                10)  log_warning "SQLite Query to File: Returned 10 [SQLITE_IOERR - Some kind of disk I/O error occurred]. Retrying"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; call_sleep_random ${SQLITE_RETRY_TIMER_MAX_SEC};;
                11)  log_error "SQLite Query to File: Returned 11 [SQLITE_CORRUPT - The database disk image is malformed]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                12)  log_error "SQLite Query to File: Returned 12 [SQLITE_NOTFOUND - Unknown opcode in sqlite3_file_control()]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                13)  log_error "SQLite Query to File: Returned 13 [SQLITE_FULL - Insertion failed because database is full]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                14)  log_error "SQLite Query to File: Returned 14 [SQLITE_CANTOPEN - Unable to open the database file]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                15)  log_error "SQLite Query to File: Returned 15 [SQLITE_PROTOCOL - Database lock protocol error]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                16)  log_error "SQLite Query to File: Returned 16 [SQLITE_EMPTY - Database is empty]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                17)  log_error "SQLite Query to File: Returned 17 [SQLITE_SCHEMA - The database schema changed]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                18)  log_error "SQLite Query to File: Returned 18 [SQLITE_TOOBIG - String or BLOB exceeds size limit]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                19)  log_error "SQLite Query to File: Returned 19 [SQLITE_CONSTRAINT - Abort due to constraint violation]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                20)  log_error "SQLite Query to File: Returned 20 [SQLITE_MISMATCH - Data type mismatch]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                21)  log_error "SQLite Query to File: Returned 21 [SQLITE_MISUSE - Library used incorrectly]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                22)  log_error "SQLite Query to File: Returned 22 [SQLITE_NOLFS - Uses OS features not supported on host]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                23)  log_error "SQLite Query to File: Returned 23 [SQLITE_AUTH - Authorization denied]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                24)  log_error "SQLite Query to File: Returned 24 [SQLITE_FORMAT - Auxiliary database format error]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                25)  log_error "SQLite Query to File: Returned 25 [SQLITE_RANGE - 2nd parameter to sqlite3_bind out of range]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                26)  log_error "SQLite Query to File: Returned 26 [SQLITE_NOTADB - File opened that is not a database file]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                27)  log_error "SQLite Query to File: Returned 27 [SQLITE_NOTICE - Notifications from sqlite3_log()]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                28)  log_error "SQLite Query to File: Returned 28 [SQLITE_WARNING - Warnings from sqlite3_log()]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                100) log_error "SQLite Query to File: Returned 100 [SQLITE_ROW - sqlite3_step() has another row ready]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                101) log_error "SQLite Query to File: Returned 101 [SQLITE_DONE - sqlite3_step() has finished executing]"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                *)   log_error "SQLite Query to File: Returned $RETURNVAL [Unknown - This result code is not listed by SQLite, it is possible the result codes have changed]. Aborting operation"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
                ?)   log_error "SQLite Query to File: Returned $RETURNVAL [Unknown - This result code is not listed by SQLite, it is possible the result codes have changed]. Aborting operation"; log_add_from_file "${SQLITE_STDERR_FILE}" "SQLite Query to File: STDERR from previous operation [Attempt::${COUNTER} of ${SQLITE_RETRY_COUNT}]"; return $E_SQLITE_FAILURE;;
            esac
            COUNTER=$((${COUNTER}+1))
        done
        log "SQLite Query to File: Returned success"
        log "SQLite Query to File: Outputting results to file [${TMP_TARGET_FILE}]"
        echo "${OUTPUT}" > ${TMP_TARGET_FILE}
        return 0
    else
        # BE QUIET SQLITE!
        while [ "${RUN}" = "1" ]; do
            if [ ${COUNTER} -gt ${SQLITE_RETRY_COUNT} ]; then
                return $E_SQLITE_FAILURE
            fi
            OUTPUT=$($(which sqlite3) "${SQLITE_FILE}" "${QUERY}" 2>/dev/null)
            RETURNVAL="$?"
            case $RETURNVAL in
                0)   RUN=0;;
                1)   return $E_SQLITE_FAILURE;;
                2)   return $E_SQLITE_FAILURE;;
                3)   return $E_SQLITE_FAILURE;;
                4)   call_sleep_random ${SQLITE_RETRY_TIMER_MAX_SEC};;
                5)   call_sleep_random ${SQLITE_RETRY_TIMER_MAX_SEC};;
                6)   call_sleep_random ${SQLITE_RETRY_TIMER_MAX_SEC};;
                7)   call_sleep_random ${SQLITE_RETRY_TIMER_MAX_SEC};;
                8)   return $E_SQLITE_FAILURE;;
                9)   call_sleep_random ${SQLITE_RETRY_TIMER_MAX_SEC};;
                10)  call_sleep_random ${SQLITE_RETRY_TIMER_MAX_SEC};;
                11)  return $E_SQLITE_FAILURE;;
                12)  return $E_SQLITE_FAILURE;;
                13)  return $E_SQLITE_FAILURE;;
                14)  return $E_SQLITE_FAILURE;;
                15)  return $E_SQLITE_FAILURE;;
                16)  return $E_SQLITE_FAILURE;;
                17)  return $E_SQLITE_FAILURE;;
                18)  return $E_SQLITE_FAILURE;;
                19)  return $E_SQLITE_FAILURE;;
                20)  return $E_SQLITE_FAILURE;;
                21)  return $E_SQLITE_FAILURE;;
                22)  return $E_SQLITE_FAILURE;;
                23)  return $E_SQLITE_FAILURE;;
                24)  return $E_SQLITE_FAILURE;;
                25)  return $E_SQLITE_FAILURE;;
                26)  return $E_SQLITE_FAILURE;;
                27)  return $E_SQLITE_FAILURE;;
                28)  return $E_SQLITE_FAILURE;;
                100) return $E_SQLITE_FAILURE;;
                101) return $E_SQLITE_FAILURE;;
                *)   return $E_SQLITE_FAILURE;;
                ?)   return $E_SQLITE_FAILURE;;
            esac
            COUNTER=$((${COUNTER}+1))
        done
        echo "${OUTPUT}" > ${TMP_TARGET_FILE}
        return 0
    fi
}

###------------------------------------------------------------------------------------------------
## FUNCTION: sqlite_set_parameter()
## - sets parameter to specified value
## - this is for cases where we are using a SQLite database as a Key-Value store, where we have a column named 'parameter'
## - Arguments
##   - $1: SQLite Database File
##   - $2: Table
##   - $3: Parameter
##   - $4: Value
function sqlite_set_parameter() {
    local RETURNVAL=""
    if option_enabled ELEVATION_DEVOPS; then
        log "SQLite Set Parameter: Attempting to set parameter (db: [${1}] / table: [${2}] / parameter: [${3}] / value: [${4}])"
        sqlite_query "${1}" "INSERT OR REPLACE INTO \"${2}\" VALUES (\"${3}\",\"${4}\")"
        RETURNVAL="$?"
        if [ ${RETURNVAL} -ne 0 ]; then
            log_error "SQLite Set Parameter: Failed to set parameter [sqlite_Returned::${RETURNVAL}]"
            return $E_SQLITE_FAILURE
        fi
        return 0
    else
        log_error "SQLite Set Parameter: Use of this function requires devops permission"
        exit_logic $E_NOT_AUTHORIZED
    fi
}

###------------------------------------------------------------------------------------------------
## FUNCTION: sqlite_vacuum_db()
## - performs VACUUM procedure against specified database
## - this function really sucks (golf clap)
## - Arguments
##   - $1: SQLite Database File
function sqlite_vacuum_db() {
    local SQLITE_FILE=$1
    # log "SQLite Vacuum DB: attempting to vacuum database [${SQLITE_FILE}]"
    local RETURNVAL=""
    sqlite_query "${SQLITE_FILE}" "vacuum"
    RETURNVAL="$?"
    if [ ${RETURNVAL} -ne 0 ]; then
        log_error "SQLite Vacuum DB: Failed to vacuum"
        return ${RETURNVAL}
    else
        :
        #log "SQLite Vacuum DB: Vacuum successful [${SQLITE_FILE}]"
    fi
    return 0
}

###------------------------------------------------------------------------------------------------
## FUNCTION: sqlite_verify_db()
## - verifies that specified SQLite database is actually a real, functioning database and not empty or corrupt
## - Arguments
##   - $1: SQLite Database File
##   - $2: Enable Logging (defaults to yes)
function sqlite_verify_db() {
    local RETURNVAL=""
    local INTEGRITY_OUTPUT=""
    local SQLITE_FILE=$1
    local SQLITE_LOGGING="${2}"
    if(is_empty "${SQLITE_LOGGING}"); then
        SQLITE_LOGGING="yes"
    fi

    # log "SQLite Verify DB: verifying database [${SQLITE_FILE}]"
    # Check if file exists
    if [ ! -f "${SQLITE_FILE}" ]; then
        if option_enabled SQLITE_LOGGING; then
            log_warning "SQLite Verify DB: ${SQLITE_FILE} file not found"
        fi
        return $E_OBJECT_NOT_FOUND
    fi

    # Check to make sure file is not 0 bytes in size (an empty file passes the integrity check)
    local SQLITE_FILE_SIZE="$(return_filesize_of_file_in_bytes "${SQLITE_FILE}")"
    if [ ${SQLITE_FILE_SIZE} -eq 0 ]; then
        if option_enabled SQLITE_LOGGING; then
            log_warning "SQLite Verify DB: Database is empty: [Size::${SQLITE_FILE_SIZE}]"
        fi
        return $E_SQLITE_FAILURE
    fi

    # Integrity check
    sqlite_query "${SQLITE_FILE}" "pragma integrity_check" INTEGRITY_OUTPUT no
    RETURNVAL="$?"
    if [ ${RETURNVAL} -ne 0 ]; then
        if option_enabled SQLITE_LOGGING; then
            log_error "SQLite Verify DB: Failed integrity check"
        fi
        return ${RETURNVAL}
    else
        if [ "${INTEGRITY_OUTPUT}" == "ok" ]; then
            :
            #log "SQLite Verify DB: Database verified (ok) [${SQLITE_FILE}]"
        else
            if option_enabled SQLITE_LOGGING; then
                log_error "SQLite Verify DB: Database integrity check returned status: [${INTEGRITY_OUTPUT}]"
            fi
            return $E_SQLITE_FAILURE
        fi
    fi
}
