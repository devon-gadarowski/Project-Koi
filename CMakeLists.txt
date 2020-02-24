cmake_minimum_required(VERSION 3.6.0)

set(ANDROID_NDK /home/devon/AndroidSDK/ndk-bundle)
set(OCULUS_MOBILE_SDK /home/devon/OculusSDK/Mobile)
set(OCULUS_MOBILE_SDK_VERSION 1.25.0)

set(SYSTEM_FRAMEWORK cpp/system/MessageBus.cpp)
set(RENDERER cpp/renderer/framework/Context.cpp cpp/renderer/framework/Renderer.cpp cpp/renderer/framework/RenderFramework.cpp cpp/renderer/RenderSystem.cpp)
set(INPUT cpp/input/InputSystem.cpp)
set(CAMERA cpp/camera/CameraSystem.cpp)

set(SYSTEMS ${SYSTEM_FRAMEWORK} ${RENDERER} ${CONSOLE} ${INPUT} ${CAMERA})

include_directories(cpp/ cpp/3rd_party/ cpp/camera/ cpp/renderer/ cpp/renderer/framework cpp/system/)

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
