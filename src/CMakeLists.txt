# -----------------------------------------------------------------
# 0. SUPPORT HOMEBREW SUR MACOS (Apple Silicon & Intel)
# -----------------------------------------------------------------
if(APPLE)
    if(EXISTS "/opt/homebrew")
        list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew" "/opt/homebrew/opt/libpq")
        set(ENV{PKG_CONFIG_PATH} "/opt/homebrew/opt/libpq/lib/pkgconfig:/opt/homebrew/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
    elseif(EXISTS "/usr/local")
        list(APPEND CMAKE_PREFIX_PATH "/usr/local" "/usr/local/opt/libpq")
        set(ENV{PKG_CONFIG_PATH} "/usr/local/opt/libpq/lib/pkgconfig:/usr/local/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
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
pkg_check_modules(LIBPQ REQUIRED libpq)

target_include_directories(cppsqlx 
    PUBLIC 
        ${PROJECT_SOURCE_DIR}/include
    PRIVATE 
        ${LIBPQ_INCLUDE_DIRS}
)

target_link_directories(cppsqlx 
    PRIVATE 
        ${LOG4CXX_LIBRARY_DIRS}
        ${LIBPQ_LIBRARY_DIRS}
)

# -----------------------------------------------------------------
# 3. LIAISON DES LIBS (Passage en PRIVATE pour éviter l'erreur d'export)
# -----------------------------------------------------------------
target_link_libraries(cppsqlx 
    PRIVATE 
        log4cxx 
        ${LIBPQ_LINK_LIBRARIES}
        nlohmann_json::nlohmann_json
)

set_target_properties(cppsqlx PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)
