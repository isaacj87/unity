set(UNITY_SRC ../plugins/unityshell/src)

#
# Data
#
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/data/ubuntuone-installer.desktop
                ${CMAKE_BINARY_DIR}/tests/data/ubuntuone-installer.desktop)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/data/ubuntu-software-center.desktop
                ${CMAKE_BINARY_DIR}/tests/data/ubuntu-software-center.desktop)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/data/update-manager.desktop
                ${CMAKE_BINARY_DIR}/tests/data/update-manager.desktop)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/data/bzr-handle-patch.desktop
                ${CMAKE_BINARY_DIR}/tests/data/bzr-handle-patch.desktop)
#
# Unit tests
#
find_package (PkgConfig)
set (TEST_DEPS "${UNITY_PLUGIN_DEPS};unity>=4.99.0")
pkg_check_modules (TEST_UNIT_DEPS REQUIRED ${TEST_DEPS})

set (TESTDATADIR "${CMAKE_CURRENT_SOURCE_DIR}/data")

set (CFLAGS
     ${TEST_UNIT_DEPS_CFLAGS}
     ${TEST_UNIT_DEPS_CFLAGS_OTHER}
     ${MAINTAINER_CFLAGS}
     "-g"
     "-DTESTDATADIR=\"${TESTDATADIR}\""
     "-DGETTEXT_PACKAGE=\"unity\""
     "-DINDICATORDIR=\"${CMAKE_BINARY_DIR}/tests\""
     "-DINDICATORICONDIR=\"${CMAKE_BINARY_DIR}/tests\""
     "-I${CMAKE_CURRENT_BINARY_DIR}"
     )
add_definitions (${CFLAGS})

set (LIBS ${TEST_UNIT_DEPS_LIBRARIES} "-lunity-core-${UNITY_API_VERSION} -lcompiz_core -lm")
link_libraries (${LIBS})

set (LIB_PATHS ${TEST_UNIT_DEPS_LIBRARY_DIRS})
link_directories (${CMAKE_BINARY_DIR}/UnityCore ${LIB_PATHS})

include_directories (. .. ../services ../UnityCore ${UNITY_SRC} ${CMAKE_BINARY_DIR})
include_directories (${CMAKE_SOURCE_DIR}/plugins/unity-mt-grab-handles/src)

# We can't have convenience libs so we need to rebuild with what we need
# Please keep actual test files alphabetically at top and then files
# from ../${UNITY_SRC} or ../../services in alphabetically after that
find_program(GLIB_GENMARSHAL glib-genmarshal)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/panel-marshal.c
  COMMAND ${GLIB_GENMARSHAL} ARGS ../services/panel-marshal.list --body --prefix=panel_marshal > ${CMAKE_CURRENT_BINARY_DIR}/panel-marshal.c
  COMMAND ${GLIB_GENMARSHAL} ARGS ../services/panel-marshal.list --header --prefix=panel_marshal > ${CMAKE_CURRENT_BINARY_DIR}/panel-marshal.h
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ../services/panel-marshal.list
  COMMENT "Generating marshallers for panel-service")

add_executable (test-unit
                unit/TestMain.cpp
                unit/TestQuicklistMenuitems.cpp
                unit/TestPanelService.cpp
                unit/TestUBus.cpp
                unit/TestStaticCairoText.cpp
                ${UNITY_SRC}/CairoBaseWindow.cpp
                ${UNITY_SRC}/Introspectable.cpp
                ${UNITY_SRC}/QuicklistMenuItem.cpp
                ${UNITY_SRC}/QuicklistMenuItemCheckmark.cpp
                ${UNITY_SRC}/QuicklistMenuItemLabel.cpp
                ${UNITY_SRC}/QuicklistMenuItemRadio.cpp
                ${UNITY_SRC}/QuicklistMenuItemSeparator.cpp
                ${UNITY_SRC}/QuicklistView.cpp
                ${UNITY_SRC}/StaticCairoText.cpp
                ../services/panel-service.c
                ${CMAKE_CURRENT_BINARY_DIR}/panel-marshal.c
                ${UNITY_SRC}/ubus-server.cpp
                )
target_link_libraries (test-unit ${LIBS})
add_dependencies (test-unit unity-core-${UNITY_API_VERSION})
add_subdirectory (test-input-remover)
add_subdirectory (test-minimize-window-handler)
add_subdirectory (test-get-transients)

#
# GTest tests
#
find_program(GLIB_GENMARSHAL glib-genmarshal)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_glib_signals_utils_marshal.cpp
  COMMAND ${GLIB_GENMARSHAL} ARGS test_glib_signals_utils_marshal.list --body --prefix=test_signals > ${CMAKE_CURRENT_BINARY_DIR}/test_glib_signals_utils_marshal.cpp
  COMMAND ${GLIB_GENMARSHAL} ARGS test_glib_signals_utils_marshal.list --header --prefix=test_signals > ${CMAKE_CURRENT_BINARY_DIR}/test_glib_signals_utils_marshal.h
  COMMAND sed ARGS -i "s/glib-object/test_glib_signals_utils_marshal/" ${CMAKE_CURRENT_BINARY_DIR}/test_glib_signals_utils_marshal.cpp
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS test_glib_signals_utils_marshal.list
  COMMENT "Generating marshallers")

enable_testing()
# :(
find_library (GMOCK_LIB gmock)
find_library (GMOCK_MAIN_LIB gmock_main)
find_path(GTEST_SRC_DIR gtest.cc PATHS /usr/src/gtest/src)

if (GTEST_SRC_DIR AND
    GMOCK_LIB AND
    GMOCK_MAIN_LIB)

# The service that provides DBus services to test against
  add_executable(test-gtest-service
                 test_service_gdbus_wrapper.c
                 test_service_gdbus_wrapper.h
                 test_service_hud.c
                 test_service_hud.h
                 test_service_lens.c
                 test_service_lens.h
                 test_service_main.c
                 test_service_model.c
                 test_service_model.h)
  target_link_libraries(test-gtest-service ${LIBS})
  add_dependencies (test-gtest-service unity-core-${UNITY_API_VERSION} gtest)


# The actual test executable (xless) - do not put anything that requires X in here
  add_executable(test-gtest-xless
                 test_animator.cpp
                 test_launcher_model.cpp
                 test_glib_object.cpp
                 test_glib_object_utils.cpp
                 test_glib_object_utils.h
                 test_glib_signals.cpp
                 test_glib_signals_utils.cpp
                 test_glib_signals_utils.h
                 test_glib_variant.cpp
                 test_desktop_utilities.cpp
                 ${CMAKE_CURRENT_BINARY_DIR}/test_glib_signals_utils_marshal.cpp
                 test_indicator.cpp
                 test_indicator_appmenu.cpp
                 test_indicator_entry.cpp
                 test_indicators.cpp
                 test_favorite_store_gsettings.cpp
                 test_favorite_store_private.cpp
                 test_home_lens.cpp
                 test_launcher_entry_remote.cpp
                 test_shortcut_model.cpp
                 test_shortcut_private.cpp
                 test_introspection.cpp
                 test_main_xless.cpp
                 test_grabhandle.cpp
                 test_unityshell_private.cpp
		 test_showdesktop_handler.cpp
                 ${UNITY_SRC}/AbstractLauncherIcon.cpp
                 ${UNITY_SRC}/AbstractShortcutHint.h
                 ${UNITY_SRC}/Animator.cpp
                 ${UNITY_SRC}/DebugDBusInterface.cpp
                 ${UNITY_SRC}/XPathQueryPart.cpp
                 ${UNITY_SRC}/FavoriteStore.cpp
                 ${UNITY_SRC}/FavoriteStoreGSettings.cpp
                 ${UNITY_SRC}/IconTextureSource.cpp
                 ${UNITY_SRC}/LauncherModel.cpp
                 ${UNITY_SRC}/LauncherEntryRemote.cpp
                 ${UNITY_SRC}/FavoriteStorePrivate.cpp
                 ${UNITY_SRC}/MockLauncherIcon.h
                 ${UNITY_SRC}/MockShortcutHint.h
                 ${UNITY_SRC}/ShortcutModel.cpp
                 ${UNITY_SRC}/ShortcutHintPrivate.cpp
                 ${UNITY_SRC}/Introspectable.cpp
                 ${UNITY_SRC}/TextureCache.cpp
                 ${UNITY_SRC}/Timer.cpp
                 ${UNITY_SRC}/UnityshellPrivate.cpp
                 ${UNITY_SRC}/WindowManager.cpp
		 ${UNITY_SRC}/UnityShowdesktopHandler.cpp
                 ${CMAKE_SOURCE_DIR}/plugins/unity-mt-grab-handles/src/unity-mt-grab-handle.cpp
                 ${CMAKE_SOURCE_DIR}/plugins/unity-mt-grab-handles/src/unity-mt-grab-handle-group.cpp
                 ${CMAKE_SOURCE_DIR}/plugins/unity-mt-grab-handles/src/unity-mt-grab-handle-impl-factory.cpp
                 ${CMAKE_SOURCE_DIR}/plugins/unity-mt-grab-handles/src/unity-mt-grab-handle-layout.cpp
                 ${CMAKE_SOURCE_DIR}/plugins/unity-mt-grab-handles/src/unity-mt-texture.cpp
                 )
  target_link_libraries(test-gtest-xless gtest ${GMOCK_LIB} ${GMOCK_MAIN_LIB} ${LIBS})
  add_test(UnityGTestXless test-gtest-xless)
  add_dependencies(test-gtest-xless unity-core-${UNITY_API_VERSION} gtest)

# tests that require dbus, must not require X
  add_executable(test-gtest-dbus
                 test_categories.cpp
                 test_filesystem_lenses.cpp
                 test_filter.cpp
                 test_gdbus_proxy.cpp
                 test_hud.cpp
                 test_lens.cpp
                 test_main_dbus.cpp
                 test_model.cpp
                 test_utils.h
                 test_ratings_filter.cpp
                 test_results.cpp
                )
  target_link_libraries(test-gtest-dbus gtest ${LIBS})
  add_test(UnityGTestDBus test-gtest-dbus)
  add_dependencies(test-gtest-dbus unity-core-${UNITY_API_VERSION} test-gtest-service gtest)

# Tests that require X
  add_executable(test-gtest
                 test_dashview_impl.cpp
                 test_texture_cache.cpp
                 test_main.cpp
                 test_lensview_impl.cpp
                 test_icon_loader.cpp
                 test_single_monitor_launcher_icon.cpp
                 test_switcher_controller.cpp
                 test_switcher_model.cpp
                 ${UNITY_SRC}/AbstractLauncherIcon.cpp
                 ${UNITY_SRC}/AbstractPlacesGroup.cpp
                 ${UNITY_SRC}/BackgroundEffectHelper.cpp
                 ${UNITY_SRC}/CairoBaseWindow.cpp
                 ${UNITY_SRC}/DashViewPrivate.cpp
                 ${UNITY_SRC}/Decaymulator.cpp
                 ${UNITY_SRC}/DNDCollectionWindow.cpp
                 ${UNITY_SRC}/DndData.cpp
                 ${UNITY_SRC}/GeisAdapter.cpp
                 ${UNITY_SRC}/IconLoader.cpp
                 ${UNITY_SRC}/IconRenderer.cpp
                 ${UNITY_SRC}/IconTextureSource.cpp
                 ${UNITY_SRC}/Introspectable.cpp
                 ${UNITY_SRC}/LayoutSystem.cpp
                 ${UNITY_SRC}/Launcher.cpp
                 ${UNITY_SRC}/LauncherDragWindow.cpp
                 ${UNITY_SRC}/LauncherEntryRemote.cpp
                 ${UNITY_SRC}/LauncherHideMachine.cpp
                 ${UNITY_SRC}/LauncherHoverMachine.cpp
                 ${UNITY_SRC}/LauncherIcon.cpp
                 ${UNITY_SRC}/LauncherModel.cpp
                 ${UNITY_SRC}/SimpleLauncherIcon.cpp
                 ${UNITY_SRC}/SingleMonitorLauncherIcon.cpp
                 ${UNITY_SRC}/LensViewPrivate.cpp
                 ${UNITY_SRC}/StaticCairoText.cpp
                 ${UNITY_SRC}/SwitcherController.cpp
                 ${UNITY_SRC}/SwitcherModel.cpp
                 ${UNITY_SRC}/SwitcherView.cpp
                 ${UNITY_SRC}/Timer.cpp
                 ${UNITY_SRC}/Tooltip.cpp
                 ${UNITY_SRC}/PanelStyle.cpp
                 ${UNITY_SRC}/PointerBarrier.cpp
                 ${UNITY_SRC}/QuicklistView.cpp
                 ${UNITY_SRC}/QuicklistManager.cpp
                 ${UNITY_SRC}/QuicklistMenuItem.cpp
                 ${UNITY_SRC}/QuicklistMenuItemCheckmark.cpp
                 ${UNITY_SRC}/QuicklistMenuItemLabel.cpp
                 ${UNITY_SRC}/QuicklistMenuItemRadio.cpp
                 ${UNITY_SRC}/QuicklistMenuItemSeparator.cpp
                 ${UNITY_SRC}/SpacerLauncherIcon.cpp
                 ${UNITY_SRC}/TextureCache.cpp
                 ${UNITY_SRC}/UBusWrapper.cpp
                 ${UNITY_SRC}/UnityWindowStyle.cpp
                 ${UNITY_SRC}/UnityWindowView.cpp
                 ${UNITY_SRC}/ubus-server.cpp
                 ${UNITY_SRC}/UScreen.cpp
                 ${UNITY_SRC}/WindowManager.cpp
                )
  target_link_libraries(test-gtest gtest ${LIBS})
  add_test(UnityGTest test-gtest)
  add_dependencies(test-gtest unity-core-${UNITY_API_VERSION} gtest)

endif (GTEST_SRC_DIR AND
       GMOCK_LIB AND
       GMOCK_MAIN_LIB)

add_subdirectory (test-gesture-engine)

#
# check target
#
set (TEST_RESULT_DIR ${CMAKE_BINARY_DIR}/tests)
set (TEST_RESULT_XML ${TEST_RESULT_DIR}/test-results.xml)
set (TEST_RESULT_HTML ${TEST_RESULT_DIR}/test-results.html)
set (GTEST_TEST_COMMAND ./test-gtest)
set (GTEST_TEST_COMMAND_XLESS ./test-gtest-xless)
set (GTEST_TEST_COMMAND_DBUS dbus-test-runner --task ./test-gtest-service --task ./test-gtest-dbus)
set (GTEST_TEST_COMMAND_GESTURE_ENGINE ./test-gesture-engine/test-gesture-engine)

set (TEST_COMMAND
     gtester --verbose -k --g-fatal-warnings -o=${TEST_RESULT_XML} ./test-unit
  && ${GTEST_TEST_COMMAND}
  && ${GTEST_TEST_COMMAND_XLESS}
  && ${GTEST_TEST_COMMAND_GESTURE_ENGINE}
  && ${GTEST_TEST_COMMAND_DBUS}
    )

set (TEST_COMMAND_HEADLESS
     ${GTEST_TEST_COMMAND_XLESS}
  && ${GTEST_TEST_COMMAND_GESTURE_ENGINE}
  #&& ${GTEST_TEST_COMMAND_DBUS}
  && echo "Warning, DBus test cases are disabled!!")

if (GTEST_SRC_DIR)
  add_custom_target (check COMMAND  ${TEST_COMMAND} DEPENDS test-unit test-gtest test-gtest-xless test-gtest-dbus test-gesture-engine)
  add_custom_target (check-headless COMMAND ${TEST_COMMAND_HEADLESS} DEPENDS test-gtest-xless test-gtest-dbus test-gesture-engine)
  add_custom_target (check-report COMMAND ${TEST_UNIT_COMMAND} && gtester-report ${TEST_RESULT_XML} > ${TEST_RESULT_HTML})
  add_custom_target (gcheck COMMAND  ${DBUS_TEST_COMMAND} DEPENDS test-gtest test-gtest-xless)
else (GTEST_SRC_DIR)
  add_custom_target (check COMMAND  ${TEST_COMMAND} DEPENDS test-unit)
endif (GTEST_SRC_DIR)
# make target to allow devs to run "make autopilot" from build dir:
set (AUTOPILOTDIR "${CMAKE_CURRENT_SOURCE_DIR}/autopilot")
# Rules to install autopilot files and executable script:
install(CODE "execute_process(COMMAND python setup.py install --prefix ${CMAKE_INSTALL_PREFIX} WORKING_DIRECTORY ${AUTOPILOTDIR})")
add_custom_target (autopilot COMMAND cd ${AUTOPILOTDIR} && make check)
