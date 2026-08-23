add_library(adept_core STATIC
    data/TreeNode.h
    data/TreeNode.cpp
    data/TreeRepository.h
    data/TreeRepository.cpp
    service/TreeFilter.h
    service/TreeFilter.cpp
    service/TreeExporter.h
    service/TreeExporter.cpp
    service/TreeService.h
    service/TreeService.cpp
)

target_include_directories(adept_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(adept_core PUBLIC
    Qt5::Core
    Qt5::Sql
)

add_executable(adept-tree
    main.cpp
    ui/MainWindow.h
    ui/MainWindow.cpp
    ui/MainWindow.ui
    ui/TreeWidget.h
    ui/TreeWidget.cpp
    ui/FilterWidget.h
    ui/FilterWidget.cpp
    ui/StatusWidget.h
    ui/StatusWidget.cpp
)

target_link_libraries(adept-tree PRIVATE
    adept_core
    Qt5::Widgets
)
