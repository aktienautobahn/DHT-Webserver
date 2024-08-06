cmake_minimum_required (VERSION 3.5)
project (RN-Praxis)
set (CMAKE_C_STANDARD 11)

add_executable (webserver webserver.c http.c util.c data.c stream_sock.c node.c sockets_setup.c chord_processor.c)
target_compile_options (webserver PRIVATE -Wall -Wextra -Wpedantic)



find_package(OpenSSL REQUIRED)
target_link_libraries(webserver PRIVATE ${OPENSSL_LIBRARIES} -lm)

#Find OpenSSL
set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

#include OpenSSL headers
include_directories(${OPENSSL_INCLUDE_DIR})

# Packaging
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES
  ${CMAKE_BINARY_DIR}
  /\\..*$ .git .venv .vscode .gitignore /test/ requirements.txt rn.lua test.md tox.ini)
set(CPACK_VERBATIM_VARIABLES YES)
include(CPack)
