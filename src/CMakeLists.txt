# -----------------------------------------------------------------
# 0. SUPPORT HOMEBREW SUR MACOS (Apple Silicon & Intel)
# -----------------------------------------------------------------
if(APPLE)
    if(EXISTS "/opt/homebrew/opt/libpq")
        set(PostgreSQL_ROOT "/opt/homebrew/opt/libpq")
        list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew" "/opt/homebrew/opt/libpq")
    elseif(EXISTS "/usr/local/opt/libpq")
        set(PostgreSQL_ROOT "/usr/local/opt/libpq")
        list(APPEND CMAKE_PREFIX_PATH "/usr/local" "/usr/local/opt/libpq")
    endif()
endif()


# -----------------------------------------------------------------
# 1. GESTION DE LA DÉPENDANCE JSON (DÉPÔT GIT CORRECT & LÉGER)
# -----------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(json)

set(json_SOURCE_DIR "${json_SOURCE_DIR}" PARENT_SCOPE)

# -----------------------------------------------------------------
# 2. CIBLE DU PROJET (cppsqlx)
# -----------------------------------------------------------------
add_library(cppsqlx SHARED
    connection.cpp
    statement.cpp
    databasemetadata.cpp
    datasetfactory.cpp
    resultset.cpp
    join.cpp
    selectquery.cpp
    insertquery.cpp
    ctasquery.cpp
    updatequery.cpp
    deletequery.cpp
    droptablequery.cpp
    sqlexpression.cpp
    sqljoinexpression.cpp
    sqljoinexpressionbuilder.cpp
    sqlselectbuilder.cpp
    sqlselectionset.cpp
    dataset.cpp
    logger.cpp
    table.cpp
    etltask.cpp
    condition.cpp
    connectioncontext.cpp
    physicalconnection.cpp
    logicalconnection.cpp
    job.cpp
    workflow.cpp
    task.cpp
    unit_select.cpp
    cte.cpp
    dataintegrationproject.cpp
)

find_package(PkgConfig REQUIRED)
find_package(log4cxx REQUIRED)
find_package(PostgreSQL REQUIRED)

target_include_directories(cppsqlx 
    PUBLIC 
        ${PROJECT_SOURCE_DIR}/include
        ${PostgreSQL_INCLUDE_DIRS}
)

target_link_directories(cppsqlx 
    PRIVATE 
        ${LOG4CXX_LIBRARY_DIRS}
)

# -----------------------------------------------------------------
# 3. LIAISON DES LIBS
# -----------------------------------------------------------------
target_link_libraries(cppsqlx 
    PUBLIC
        PostgreSQL::PostgreSQL
    PRIVATE 
        log4cxx 
        nlohmann_json::nlohmann_json
)


set_target_properties(cppsqlx PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)
