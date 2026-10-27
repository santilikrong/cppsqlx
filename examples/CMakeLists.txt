add_executable(example1 example1.cpp)
add_executable(example2 example2.cpp)
add_executable(example3 example3.cpp)
add_executable(example4 example4.cpp)

set_target_properties(example1 PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
set_target_properties(example2 PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)


if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

find_package(log4cxx REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPQ REQUIRED libpq)

include_directories(example1 PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    /opt/homebrew/Cellar/log4cxx/1.5.0/include
    /opt/homebrew/Cellar/boost/1.89.0_1/include
    /opt/homebrew/Cellar/libpq/18.1/include
    ${Boost_INCLUDE_DIR}
)

include_directories(example2 PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    /opt/homebrew/Cellar/log4cxx/1.5.0/include
    /opt/homebrew/Cellar/boost/1.89.0_1/include
    /opt/homebrew/Cellar/libpq/18.1/include
    ${Boost_INCLUDE_DIR}
)

include_directories(example3 PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    /opt/homebrew/Cellar/log4cxx/1.5.0/include
    /opt/homebrew/Cellar/boost/1.89.0_1/include
    /opt/homebrew/Cellar/libpq/18.1/include
    ${Boost_INCLUDE_DIR}
)

include_directories(example3 PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    /opt/homebrew/Cellar/log4cxx/1.5.0/include
    /opt/homebrew/Cellar/boost/1.89.0_1/include
    /opt/homebrew/Cellar/libpq/18.1/include
    ${Boost_INCLUDE_DIR}
)
#target_include_directories(example1 PRIVATE ${LIBPQ_INCLUDE_DIRS} Boost::program_options ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(example1 PRIVATE cppsqlx log4cxx ${LIBPQ_LINK_LIBRARIES} Boost::program_options )
target_link_libraries(example2 PRIVATE cppsqlx log4cxx ${LIBPQ_LINK_LIBRARIES} Boost::program_options )
target_link_libraries(example3 PRIVATE cppsqlx log4cxx ${LIBPQ_LINK_LIBRARIES} Boost::program_options )
target_link_libraries(example4 PRIVATE cppsqlx log4cxx ${LIBPQ_LINK_LIBRARIES} Boost::program_options )
