project (test)
cmake_minimum_required (VERSION 2.6)

include (../sagase-cmake/Sagase.cmake)

include (FindQt4)
find_package (Qt4 4.6 COMPONENTS QtCore QtGui QtNetwork REQUIRED)

if (NOT QT4_FOUND)
    message (FATAL_ERROR "Qt4 not found")
endif (NOT QT4_FOUND)

include (${QT_USE_FILE})

include_directories (.)

qt4_generate_moc (xmlrpc.hpp moc_xmlrpc.cpp)
qt4_generate_moc (capabilities.hpp moc_capabilities.cpp)
qt4_generate_moc (application.hpp moc_application.cpp)
qt4_generate_moc (llstack/llprovider.hpp llstack/moc_llprovider.cpp)

add_executable (test 
    tag.cpp 
    xmlrpc.cpp
    capabilities.cpp 
    application.cpp
    llstack/lluuid.cpp
    llstack/llprovider.cpp 
    llstack/moc_llprovider.cpp
    llstack/llmessage.cpp
    moc_xmlrpc.cpp
    moc_capabilities.cpp
    moc_application.cpp
    main.cpp)
    
if (MSVC)
    target_link_libraries (test ${QT_LIBRARIES})# debug ${OGRE_DEBUG_LIBRARIES})
else ()
    target_link_libraries (test ${QT_LIBRARIES})
endif ()
