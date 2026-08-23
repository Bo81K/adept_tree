add_library(test_common STATIC
    TestDatabaseUtility.h
    TestDatabaseUtility.cpp
)

target_include_directories(test_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(test_common PUBLIC
    adept_core
)

add_executable(TreeFilterTest
    TreeFilterTest.cpp
)

target_link_libraries(TreeFilterTest PRIVATE
    adept_core
    Qt5::Test
)

add_test(NAME TreeFilterTest COMMAND TreeFilterTest)

add_executable(TreeRepositoryTest
    TreeRepositoryTest.cpp
)

target_link_libraries(TreeRepositoryTest PRIVATE
    test_common
    Qt5::Test
)

add_test(NAME TreeRepositoryTest COMMAND TreeRepositoryTest)

add_executable(TreeServiceTest
    TreeServiceTest.cpp
)

target_link_libraries(TreeServiceTest PRIVATE
    test_common
    Qt5::Test
)

add_test(NAME TreeServiceTest COMMAND TreeServiceTest)
