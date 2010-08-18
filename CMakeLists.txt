project (test)
cmake_minimum_required (VERSION 2.6)

include (Sagase.cmake)

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
qt4_generate_moc (llplugin/provider.hpp llplugin/moc_provider.cpp)

add_executable (test 
    tag.cpp 
    xmlrpc.cpp
    capabilities.cpp 
    application.cpp
    task.cpp
    userview.cpp
    llplugin/uuid.cpp
    llplugin/message.cpp
    llplugin/messageid.cpp
    llplugin/provider.cpp 
    llplugin/moc_provider.cpp
    llplugin/datacoding.cpp
    uiplugin/provider.cpp
    moc_xmlrpc.cpp
    moc_capabilities.cpp
    moc_application.cpp
    main.cpp)
    
if (MSVC)
    target_link_libraries (test ${QT_LIBRARIES})# debug ${OGRE_DEBUG_LIBRARIES})
else ()
    target_link_libraries (test ${QT_LIBRARIES})
endif ()
