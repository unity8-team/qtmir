set(
  SCREENMODEL_TEST_SOURCES
  screenmodel_test.cpp
  ${CMAKE_SOURCE_DIR}/src/common/debughelpers.cpp
  # to be moc-ed
  stub_screen.h
  testable_screenmodel.h
)

include_directories(
  ${CMAKE_SOURCE_DIR}/tests/framework
  ${CMAKE_SOURCE_DIR}/src/platforms/mirserver
  ${CMAKE_SOURCE_DIR}/src/common
  ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
  ${MIRSERVER_INCLUDE_DIRS}
  ${MIRRENDERERGLDEV_INCLUDE_DIRS}
)

add_executable(ScreenModelTest ${SCREENMODEL_TEST_SOURCES})

target_link_libraries(
  ScreenModelTest
  qpa-mirserver

  -L${CMAKE_BINARY_DIR}/tests/framework
  qtmir-test-framework-static

  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(ScreenModel, ScreenModelTest)
