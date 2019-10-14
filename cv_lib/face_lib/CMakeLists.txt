project(face_lib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS "CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_CURRENT_BINARY_DIR})
message(STATUS "CMAKE_CURRENT_BINARY_DIR = ${CMAKE_CURRENT_BINARY_DIR}")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/ssd")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/utils")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../opencv3.4.3/include")
include_directories(/root/cqj/pytorch)
include_directories(/root/cqj/pytorch/build_android)
include_directories(/root/cqj/pytorch/third_party/protobuf/src)
include_directories(/root/cqj/pytorch/third_party/eigen)
include_directories(/root/cqj/pytorch/aten/src)
#include_directories(/root/cqj/pytorch/third_party/eigen/Eigen/src)

set(face_lib_SRCS 
    face/newface.cpp
	seeta/aligner.cpp
	seeta/blob.cpp
	seeta/common_net.cpp
	seeta/log.cpp
	seeta/net.cpp
	seeta/spatial_transform_net.cpp
	seeta/tform_maker_net.cpp
	#mtcnn/mtcnn.cpp
	#mtcnn/network.cpp
	#mtcnn/pBox.cpp
    ssd/ssd.cpp
	faceimp.cpp)





#opencv
add_library( 
     libopencv_java3 
	 SHARED 
	 IMPORTED )
set_target_properties(  
     libopencv_java3 
	 PROPERTIES IMPORTED_LOCATION
     ${CMAKE_CURRENT_SOURCE_DIR}/../opencv3.4.3/libs/armeabi-v7a/libopencv_java3.so )

	 
  
      
 
function(import_caffe2_lib path name)

  add_library(${name} STATIC IMPORTED)
  set_property(
      TARGET ${name}
      PROPERTY IMPORTED_LOCATION
      ${path}/${name}.a)
endfunction(import_caffe2_lib)

set(caffe2path /root/cqj/pytorch/build_android/lib)
# caffe2 lib

import_caffe2_lib(${caffe2path} libqnnpack)
import_caffe2_lib(${caffe2path} libtorch)
import_caffe2_lib(${caffe2path} libc10)
import_caffe2_lib(${caffe2path} libnnpack)
import_caffe2_lib(${caffe2path} libpytorch_qnnpack)
import_caffe2_lib(${caffe2path} libcpuinfo)
import_caffe2_lib(${caffe2path} libclog)
import_caffe2_lib(${caffe2path} libfoxi_loader)
import_caffe2_lib(${caffe2path} libprotobuf)
import_caffe2_lib(${caffe2path} libcaffe2_protos)
import_caffe2_lib(${caffe2path} libonnx_proto)
import_caffe2_lib(${caffe2path} libprotobuf-lite)
import_caffe2_lib(${caffe2path} libpthreadpool)


#compile
add_library(face_lib  SHARED  ${face_lib_SRCS})


target_link_libraries(face_lib
                      -Wl,--gc-sections
					  -Wl,--whole-archive
					  libtorch
					  libcaffe2_protos
					  -Wl,--no-whole-archive
					  libonnx_proto
					  libc10
					  libnnpack
					  libqnnpack
					  libpytorch_qnnpack
					  libcpuinfo
					  libclog
					  libprotobuf
					  libprotobuf-lite
					  libfoxi_loader
					  libpthreadpool
                      #m
					  #z
					  #android	
					  log
					  libopencv_java3
					  )


install(TARGETS 
        face_lib
		ARCHIVE DESTINATION face_lib
		LIBRARY DESTINATION face_lib)
install(FILES
    faceimp.h    
    DESTINATION face_include
)
