project(mtcnn)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS "CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_CURRENT_BINARY_DIR})
message(STATUS "CMAKE_CURRENT_BINARY_DIR = ${CMAKE_CURRENT_BINARY_DIR}")

include_directories(/root/cqj/ncnn/build-android-armv7/install/include)

set(mtcnn_SRCS
    mtcnn.cpp
    mtcnn_imp.cpp
)

add_library(mtcnn  SHARED  ${mtcnn_SRCS})


add_library(ncnn STATIC IMPORTED )
set_target_properties(ncnn
        PROPERTIES IMPORTED_LOCATION
        /root/cqj/ncnn/build-android-armv7/install/lib/libncnn.a)

target_link_libraries(mtcnn  
                      #"-Wl ,--whole-archive" 
                      ncnn
                      #"-Wl,--no-whole-archive"
					  )


install(TARGETS 
        mtcnn 
		ARCHIVE DESTINATION mtcnn_lib
		LIBRARY DESTINATION mtcnn_lib)
install(FILES
    mtcnn_imp.h    
    DESTINATION mtcnn_include
)
