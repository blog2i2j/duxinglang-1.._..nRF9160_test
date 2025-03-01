# Install script for directory: D:/software/nrf9160/ncs1.2.0/zephyr

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/Zephyr-Kernel")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/zephyr/arch/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/zephyr/lib/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/zephyr/soc/arm/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/zephyr/boards/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/zephyr/ext/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/zephyr/subsys/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/zephyr/drivers/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/modules/nrf/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/modules/mcuboot/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/modules/mcumgr/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/modules/nrfxlib/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/modules/canopennode/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/modules/civetweb/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/modules/fatfs/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/modules/nordic/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/modules/st/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/modules/libmetal/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/modules/lvgl/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/modules/mbedtls/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/modules/open-amp/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/modules/loramac-node/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/modules/openthread/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/modules/segger/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/modules/tinycbor/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/modules/littlefs/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/modules/mipi-sys-t/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/modules/nrf_hw_models/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/zephyr/kernel/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/zephyr/cmake/flash/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/zephyr/cmake/usage/cmake_install.cmake")
  include("D:/software/nrf9160/my_projects/my_test/hello_world/build_nrf9160_pca10090ns/zephyr/cmake/reports/cmake_install.cmake")

endif()

