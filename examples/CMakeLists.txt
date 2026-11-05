cmake_minimum_required(VERSION 3.14)

# -----------------------------------------------------------------
# 1. DÉFINITION DES EXÉCUTABLES
# -----------------------------------------------------------------
set(EXAMPLES_LIST example1 example2 example3 example4 example5)

foreach(example ${EXAMPLES_LIST})
    add_executable(${example} ${example}.cpp)
endforeach()

# Optimisation interprocédurale (LTO) uniquement sur les cibles spécifiées
set_target_properties(example1 example2 PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)

# -----------------------------------------------------------------
# 2. DÉPENDANCES EXTERNES
# -----------------------------------------------------------------
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

find_package(log4cxx REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPQ REQUIRED libpq)

# -----------------------------------------------------------------
# 3. CONFIGURATION DES REPERTOIRES ET LIAISONS (Boucle Moderne)
# -----------------------------------------------------------------
foreach(example ${EXAMPLES_LIST})
    
    # Remplacement des chemins Homebrew en dur par les variables CMake portables
    target_include_directories(${example} PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${LIBPQ_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
        ${json_SOURCE_DIR} 
    )
    
    # Liaison de toutes les dépendances. 
    # cppsqlx transmettra automatiquement l'accès à json.hpp car configuré en PUBLIC
    target_link_libraries(${example} PRIVATE 
        cppsqlx 
        log4cxx 
        ${LIBPQ_LINK_LIBRARIES} 
        Boost::program_options
        nlohmann_json::nlohmann_json
    )
endforeach()
