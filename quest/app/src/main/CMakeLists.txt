cmake_minimum_required(VERSION 3.6.0)

set(PROJECT_ROOT /home/devon/Desktop/projectkoi/v4)

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

#file(GLOB SOURCES "${PROJECT_ROOT}/src/*.cpp")

add_library(projectkoi SHARED
	${PROJECT_ROOT}/src/ProjectKoi.cpp
	${PROJECT_ROOT}/src/RenderSystem.cpp
	${PROJECT_ROOT}/src/Context.cpp
	${PROJECT_ROOT}/src/Renderer.cpp
	${PROJECT_ROOT}/src/OVRContext.cpp
	${PROJECT_ROOT}/src/OVRRenderer.cpp
	${PROJECT_ROOT}/src/System.cpp
	${PROJECT_ROOT}/src/Scene3D.cpp
	${PROJECT_ROOT}/src/Camera.cpp
	${PROJECT_ROOT}/src/Utilities.cpp
	${PROJECT_ROOT}/src/Model.cpp)

target_include_directories(projectkoi PUBLIC ${PROJECT_ROOT}/include)
target_include_directories(projectkoi PUBLIC ${PROJECT_ROOT}/3rd_party)

add_library(native_app_glue STATIC $ENV{ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
target_include_directories(projectkoi PUBLIC $ENV{ANDROID_NDK}/sources/android/native_app_glue)

add_library(vrapi SHARED IMPORTED)
set_property(TARGET vrapi PROPERTY IMPORTED_LOCATION $ENV{OCULUS_MOBILE_SDK}/$ENV{OCULUS_MOBILE_SDK_VERSION}/VrApi/Libs/Android/${ANDROID_ABI}/${CMAKE_BUILD_TYPE}/libvrapi.so)
target_include_directories(projectkoi PUBLIC $ENV{OCULUS_MOBILE_SDK}/$ENV{OCULUS_MOBILE_SDK_VERSION}/VrApi/Include)

add_library(vulkan_wrapper STATIC $ENV{ANDROID_NDK}/sources/third_party/vulkan/src/common/vulkan_wrapper.cpp)
target_include_directories(projectkoi PUBLIC ${ANDROID_NDK}/sources/third_party/vulkan/src/common)

target_link_libraries(
	projectkoi
	android
	native_app_glue
	vulkan_wrapper
	vrapi
	log)
