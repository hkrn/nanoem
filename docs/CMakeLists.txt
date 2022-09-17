# conf.py
string(REGEX REPLACE "^v([0-9]+\\.[0-9]+).+" "\\1" NANOEM_VERSION ${NANOEM_VERSION_GIT})
if("${NANOEM_VERSION}" STREQUAL  "${NANOEM_VERSION_GIT}")
  configure_file(conf.py.in ${CMAKE_CURRENT_SOURCE_DIR}/conf.py @ONLY)
endif()
