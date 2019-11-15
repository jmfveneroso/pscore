cmake_minimum_required (VERSION 3.0)
project (Pscore)
set (CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "-fPIC")

find_package(PythonInterp)
find_package(PythonLibs)
find_package(Boost COMPONENTS python)

set(
  INCLUDE_DIRS 
  /usr/include
  ${CMAKE_SOURCE_DIR}/include
  ${Boost_INCLUDE_DIRS} 
  ${PYTHON_INCLUDE_DIRS}
)

link_directories(
  /usr/lib
  /usr/local/lib
)

set(
  LIBS
  ${Boost_LIBRARIES}
  ${PYTHON_LIBRARIES}
)

include_directories(${INCLUDE_DIRS})
link_directories(${LINK_DIRS})

set(
  SRC
  src/pscore.cpp
)

# Create sybil library.
add_library(pscore_module ${SRC})
target_link_libraries(pscore_module)

PYTHON_ADD_MODULE(pscore src/python_wrapper.cpp)
target_link_libraries(pscore pscore_module ${LIBS})

include(GNUInstallDirs)
install(
  TARGETS pscore
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
