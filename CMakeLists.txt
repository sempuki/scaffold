project (scaffold)
cmake_minimum_required (VERSION 2.6)

include (FindQt4)
include (FindBoost)

find_package (Qt4 4.6 COMPONENTS QtCore QtGui QtNetwork REQUIRED)
find_package (Boost 1.43)

if (NOT QT4_FOUND)
    message (FATAL_ERROR "Qt4 not found")
endif (NOT QT4_FOUND)

if (NOT Boost_FOUND)
    message (FATAL_ERROR "Boost not found, define BOOST_ROOT to you boost location")
else()
    message (STATUS "Found the following Boost include path:")
    message (STATUS "    " ${Boost_INCLUDE_DIRS})
endif (NOT Boost_FOUND)

include (${QT_USE_FILE})

include_directories (${Boost_INCLUDE_DIRS} .)

qt4_generate_moc (xmlrpc.hpp moc_xmlrpc.cpp)
qt4_generate_moc (capabilities.hpp moc_capabilities.cpp)
qt4_generate_moc (application.hpp moc_application.cpp)
qt4_generate_moc (llplugin/provider.hpp llplugin/moc_provider.cpp)
qt4_generate_moc (viewerplugin/logic.hpp viewerplugin/moc_logic.cpp)
qt4_generate_moc (viewerplugin/ui_login.hpp viewerplugin/moc_ui_login.cpp)

add_executable (scaffold 
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
    viewerplugin/logic.cpp
    viewerplugin/ui_login.cpp
    viewerplugin/moc_logic.cpp
    viewerplugin/moc_ui_login.cpp
    moc_xmlrpc.cpp
    moc_capabilities.cpp
    moc_application.cpp
    main.cpp)
    
if (MSVC)
    target_link_libraries (scaffold ${QT_LIBRARIES})# debug ${OGRE_DEBUG_LIBRARIES})
else ()
    target_link_libraries (scaffold ${QT_LIBRARIES})
endif ()
