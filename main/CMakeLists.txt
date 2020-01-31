cmake_minimum_required(VERSION 3.6.0)

set(ANDROID_NDK /home/devon/AndroidSDK/ndk-bundle)
set(OCULUS_MOBILE_SDK /home/devon/OculusSDK/Mobile)
set(OCULUS_MOBILE_SDK_VERSION 1.25.0)

set(SYSTEM_FRAMEWORK cpp/system/MessageBus.cpp)

set(RENDERER cpp/renderer/framework/Instance.cpp cpp/renderer/framework/PhysicalDevice.cpp cpp/renderer/framework/LogicalDevice.cpp cpp/renderer/framework/GraphicsContext.cpp cpp/renderer/framework/Utilities.cpp cpp/renderer/RenderSystem.cpp)

set(CONSOLE cpp/console/ConsoleSystem.cpp cpp/console/framework/Commands.cpp)

set(SYSTEMS ${SYSTEM_FRAMEWORK} ${RENDERER} ${CONSOLE})

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")
add_library(projectkoi SHARED cpp/ProjectKoi.cpp ${SYSTEMS})

add_library(native_app_glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
target_include_directories(projectkoi PUBLIC ${ANDROID_NDK}/sources/android/native_app_glue)

add_library(vulkan_wrapper STATIC ${ANDROID_NDK}/sources/third_party/vulkan/src/common/vulkan_wrapper.cpp)
target_include_directories(projectkoi PUBLIC ${ANDROID_NDK}/sources/third_party/vulkan/src/common)

add_library(vrapi SHARED IMPORTED)
set_property(TARGET vrapi PROPERTY IMPORTED_LOCATION ${OCULUS_MOBILE_SDK}/${OCULUS_MOBILE_SDK_VERSION}/VrApi/Libs/Android/${ANDROID_ABI}/${CMAKE_BUILD_TYPE}/libvrapi.so)
target_include_directories(projectkoi PUBLIC ${OCULUS_MOBILE_SDK}/${OCULUS_MOBILE_SDK_VERSION}/VrApi/Include)

target_link_libraries(
	projectkoi
	android
	native_app_glue
	vulkan_wrapper
	vrapi
	log)
