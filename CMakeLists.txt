project (unity)
cmake_minimum_required(VERSION 2.8)

include (cmake/Documentation.cmake)

#
# Base bits
#
set (PROJECT_NAME "unity")
set (UNITY_MAJOR 5)
set (UNITY_MINOR 12)
set (UNITY_MICRO 0)
set (UNITY_VERSION "${UNITY_MAJOR}.${UNITY_MINOR}.${UNITY_MICRO}")
set (UNITY_API_VERSION "5.0")

set (CMAKE_CXX_FLAGS "-DGNOME_DESKTOP_USE_UNSTABLE_API -std=c++0x -fno-permissive")
set (CMAKE_CXX_FLAGS_DEBUG "-g3")
set (CMAKE_CXX_FLAGS_RELEASE "")

if (BUILD_GLES)
	SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNUX_OPENGLES_20 -DUSE_GLES")
endif (BUILD_GLES)

#
# Niceties
#
set (ARCHIVE_NAME unity-${UNITY_VERSION})

add_custom_target (pre-distcheck
  COMMAND echo ""
       && echo "• Releasing Unity ${UNITY_VERSION}"
       && cd ${CMAKE_SOURCE_DIR}
       && echo "• Generating ChangeLog"
       && bzr log --gnu-changelog > ChangeLog
       && echo "• Generating AUTHORS"
       && bzr log --long --levels=0 | grep -e "^\\s*author:" -e "^\\s*committer:" | cut -d ":" -f 2 | sed -r -f AUTHOR-glue | sort -u > AUTHORS
       && echo "• Committing Release"
       && bzr commit -m"Release ${UNITY_VERSION}" --unchanged
       && echo "• Tagging Release"
       && bzr tag ${UNITY_VERSION}
       && echo "• Running Distcheck"
  )
  
add_custom_target (dist
  COMMAND bzr export --root=${ARCHIVE_NAME} ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
add_dependencies(dist pre-distcheck)

add_custom_target (distcheck
  COMMAND cd ${CMAKE_BINARY_DIR}
       && rm -rf ${ARCHIVE_NAME}
       && tar xf ${ARCHIVE_NAME}.tar.bz2
       && mkdir ${ARCHIVE_NAME}/build
       && cd ${ARCHIVE_NAME}/build
       && cmake -DCMAKE_INSTALL_PREFIX=../install -DGSETTINGS_LOCALINSTALL=ON .. -DCMAKE_MODULE_PATH=/usr/share/cmake
       && make -j4
       && make -j4 install
       && make check
  )
add_dependencies(distcheck dist)

add_custom_target (post-distcheck
  COMMAND echo "• Signing Tarball"
       && gpg --armor --sign --detach-sig ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
       && echo "• Pushing Branch"
       && bzr push
       && echo "• Publishing Tarball"
       && lp-project-upload unity ${UNITY_VERSION} ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
       && echo ""
       && echo "• Unity ${UNITY_VERSION} Released."
  )
add_dependencies(post-distcheck distcheck)

add_custom_target (release)
add_dependencies (release post-distcheck)

#
# config.h
#
set (PREFIXDIR "${CMAKE_INSTALL_PREFIX}")
set (DATADIR "${CMAKE_INSTALL_PREFIX}/share")
set (PKGDATADIR "${DATADIR}/unity/${UNITY_MAJOR}")
set (GETTEXT_PACKAGE "unity")
set (LOCALE_DIR "${DATADIR}/locale")
set (VERSION "${UNITY_VERSION}")
set (BUILDDIR "${CMAKE_BINARY_DIR}")

configure_file (${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_BINARY_DIR}/config.h)

#
# i18n
#
find_package (Gettext REQUIRED)

add_custom_command (OUTPUT ${CMAKE_SOURCE_DIR}/po/unity.pot
		COMMAND xgettext -c --files-from ${CMAKE_SOURCE_DIR}/po/POTFILES.in --keyword=_ -o ${CMAKE_SOURCE_DIR}/po/unity.pot --copyright-holder="Canonical Ltd" --msgid-bugs-address="ayatana-dev@lists.launchpad.net" --no-wrap --no-location
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		)

if (${GETTEXT_FOUND} STREQUAL "TRUE")
        set (HAVE_GETTEXT true)
        file (GLOB _translations ${CMAKE_SOURCE_DIR}/po/*.po)
        GETTEXT_CREATE_TRANSLATIONS (${CMAKE_SOURCE_DIR}/po/${PROJECT_NAME}.pot
                ALL ${_translations})
endif (${GETTEXT_FOUND} STREQUAL "TRUE")

#
# Enable or disable boot logging
# 
option (BOOT_LOGGER "Enable startup performance logging" OFF)
if (BOOT_LOGGER)
    SET (BOOT_LOGGER_FLAG "-DENABLE_LOGGER")
endif (BOOT_LOGGER)

SET (MAINTAINER_CFLAGS "-Werror -Wall -Wcast-align -Wno-uninitialized -Wempty-body -Wformat-security -Winit-self -Warray-bounds")
option (DISABLE_MAINTAINER_CFLAGS "Disable maintainer CFlags" OFF)
if (DISABLE_MAINTAINER_CFLAGS)
  SET (MAINTAINER_CFLAGS "")
endif (DISABLE_MAINTAINER_CFLAGS)

#
# Compiz Plugins
#
set (UNITY_PLUGIN_DEPS "compiz;nux-2.0>=2.0.0;libbamf3;dee-1.0;gio-2.0;gio-unix-2.0;dbusmenu-glib-0.4;x11;libstartup-notification-1.0;gthread-2.0;indicator3-0.4>=0.4.90;atk;unity-misc>=0.4.0;gconf-2.0;libutouch-geis;gtk+-3.0>=3.1;sigc++-2.0;json-glib-1.0;libnotify;gnome-desktop-3.0;gdu;xfixes")

add_subdirectory(plugins/unityshell)
add_subdirectory(plugins/gtkloader)
add_subdirectory(plugins/networkarearegion)
add_subdirectory(plugins/unitydialog)
add_subdirectory(plugins/unity-mt-grab-handles)

# subdirs
add_subdirectory(doc)

# Check for google test and build it locally
set(
  GTEST_ROOT_DIR
  "/usr/src/gtest" # Default value, adjustable by user with e.g., ccmake
  CACHE
  PATH
  "Path to Google test srcs"
)

find_path(GTEST_INCLUDE_DIR gtest/gtest.h)
if (GTEST_INCLUDE_DIR)
  #FIXME - hardcoded is bad!
  add_subdirectory(
    ${GTEST_ROOT_DIR}
    gtest
  )
endif(GTEST_INCLUDE_DIR)

add_subdirectory(services)
add_subdirectory(tests)
add_subdirectory(tools)
add_subdirectory(UnityCore)
add_subdirectory(guides)
add_subdirectory(standalone-clients EXCLUDE_FROM_ALL)


#
# GSettings Schema
#
SET (UNITY_SCHEMAS "com.canonical.Unity.gschema.xml")

# Have an option to not install the schema into where GLib is
option (GSETTINGS_LOCALINSTALL "Install GSettings Schemas locally instead of to the GLib prefix" OFF)
if (GSETTINGS_LOCALINSTALL)
    SET (GSETTINGS_DIR "${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/")
else (GSETTINGS_LOCALINSTALL)
    execute_process (COMMAND ${PKG_CONFIG_EXECUTABLE} glib-2.0 --variable prefix  OUTPUT_VARIABLE _glib_prefix OUTPUT_STRIP_TRAILING_WHITESPACE)
    SET (GSETTINGS_DIR "${_glib_prefix}/share/glib-2.0/schemas/")
endif (GSETTINGS_LOCALINSTALL)

# Run the validator and error if it fails
execute_process (COMMAND ${PKG_CONFIG_EXECUTABLE} gio-2.0 --variable glib_compile_schemas  OUTPUT_VARIABLE _glib_comple_schemas OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process (COMMAND ${_glib_comple_schemas} --dry-run --schema-file=${CMAKE_CURRENT_SOURCE_DIR}/${UNITY_SCHEMAS} ERROR_VARIABLE _schemas_invalid OUTPUT_STRIP_TRAILING_WHITESPACE)

if (_schemas_invalid)
  message (SEND_ERROR "Schema validation error: ${_schemas_invalid}")
endif (_schemas_invalid)

# Actually install and recomple schemas
message (STATUS "GSettings schemas will be installed into ${GSETTINGS_DIR}")
install (FILES ${UNITY_SCHEMAS} DESTINATION ${GSETTINGS_DIR} OPTIONAL)
install (CODE "message (STATUS \"Compiling GSettings schemas\")")
install (CODE "execute_process (COMMAND ${_glib_comple_schemas} ${GSETTINGS_DIR})")

# For testing
configure_file (${CMAKE_SOURCE_DIR}/${UNITY_SCHEMAS} ${CMAKE_BINARY_DIR}/settings/${UNITY_SCHEMAS})
execute_process (COMMAND ${_glib_comple_schemas} ${CMAKE_BINARY_DIR}/settings)

#
# docs
#

# check if doxygen is even installed
find_package(Doxygen)
if (DOXYGEN_FOUND STREQUAL "NO")
  message("Doxygen not found. Documentation will not be built")
endif (DOXYGEN_FOUND STREQUAL "NO")

if (DOXYGEN_FOUND STREQUAL "YES")
  set(TOP_SRCDIR ${CMAKE_CURRENT_SOURCE_DIR})

  # prepare doxygen configuration file
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  # add doxygen as target
  add_custom_target(doxygen ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  # cleanup $build/api-doc on "make clean"
  set_property(DIRECTORY APPEND PROPERTY
         ADDITIONAL_MAKE_CLEAN_FILES api-doc)

endif (DOXYGEN_FOUND STREQUAL "YES")
