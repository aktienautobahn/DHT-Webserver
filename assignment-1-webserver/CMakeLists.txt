cmake_minimum_required (VERSION 3.5)
project (RN-Praxis)
set (CMAKE_C_STANDARD 11)
# add the executable
ADD_EXECUTABLE(webserver webserver.c webserver_parser.c webserver_response.c webserverMiddleware.c webserverRequestHandler.c webserver_fileSystem.c webserver_logger.c)

# Packaging
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES ${CMAKE_BINARY_DIR} /\\..*$ .git .venv .vscode .gitignore /test/ requirements.txt tox.ini)
set(CPACK_VERBATIM_VARIABLES YES)
include(CPack)
