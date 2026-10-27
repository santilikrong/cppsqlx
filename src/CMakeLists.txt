add_library(cppsqlx SHARED
    connection.cpp
    statement.cpp
    datasetfactory.cpp
    resultset.cpp
    join.cpp
    selectquery.cpp
    sqlexpression.cpp
    sqljoinexpression.cpp
    sqljoinexpressionbuilder.cpp
    sqlselectbuilder.cpp
    dataset.cpp
    logger.cpp
    table.cpp
    cte.cpp
)

find_package(PkgConfig REQUIRED)
find_package(log4cxx REQUIRED)

include_directories(cppsqlx PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    /opt/homebrew/Cellar/log4cxx/1.5.0/include
    /opt/homebrew/Cellar/boost/1.89.0_1/include
)

pkg_check_modules(LIBPQ REQUIRED libpq)

LINK_DIRECTORIES(
    {LOG4CXX_LIBRARY_DIRS} 
)

target_include_directories(cppsqlx PRIVATE ${LIBPQ_INCLUDE_DIRS})
target_link_libraries(cppsqlx PRIVATE log4cxx ${LIBPQ_LINK_LIBRARIES})

# Optional: set version info for shared library
set_target_properties(cppsqlx PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)