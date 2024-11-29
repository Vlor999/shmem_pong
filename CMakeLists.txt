cmake_minimum_required(VERSION 3.10)
####
# The version number in previous line may be reduced if needed. I used
# the default in cmake tutorial (oct 2021)

project(Ensipong)
enable_testing()


#######
# Detect SDL using standard CMake FindSDL
#####
find_package(SDL2 REQUIRED)
set(SDL_INCLUDE_CFLAGS -I${SDL2_INCLUDE_DIR})


# Debug build

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${SDL_INCLUDE_CFLAGS} -Wall  -Wextra -Werror -std=gnu17")

#########
# Vous devez editer les lignes suivantes pour y mettre vos logins
#########
set(VARIANTE_LOGINS willem adnet adnetw)

#########

list(SORT VARIANTE_LOGINS)
if (VARIANTE_LOGINS MATCHES "login[123]")
  message(FATAL_ERROR "** ERREUR **: Vous devez modifier CMakeLists.txt pour y mettre vos logins")
endif()

#########

##
# Si vous utilisez plusieurs fichiers, en plus de shmem_pong.c, pour votre
# shell il faut les ajouter ici
##
add_executable(ensipong src/shmem_pong.c)
target_link_libraries(ensipong ${SDL2_LIBRARIES} -lrt)

##
# Programme de test
##
add_test(pong ../tests/fourpong.pl)

##
# Ajout d'une cible pour lancer les tests de manière verbeuse
##
add_custom_target(check ctest -V)
