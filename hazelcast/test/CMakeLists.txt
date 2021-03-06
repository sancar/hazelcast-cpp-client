cmake_minimum_required (VERSION 3.1)
project (HazelcastClientTest)

FILE(GLOB_RECURSE HZ_TEST_SOURCES "./*cpp")
FILE(GLOB_RECURSE HZ_TEST_HEADERS "./*h")

include_directories(${CMAKE_SOURCE_DIR}/hazelcast/include ${CMAKE_SOURCE_DIR}/hazelcast/test)

#set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/java/)

SET(HZ_EXE_NAME clientTest_${HZ_LIB_TYPE}_${HZ_BIT})

add_executable(${HZ_EXE_NAME} ${HZ_TEST_SOURCES} ${HZ_TEST_HEADERS} )

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

	target_link_libraries(${HZ_EXE_NAME} ${HZ_LIB_NAME} )

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

	set_target_properties(${HZ_EXE_NAME} PROPERTIES COMPILE_FLAGS "${HZ_BIT_FLAG} ${HZ_CODE_COVERAGE_COMPILE_FLAGS} ${HZ_VALGRIND_COMPILE_FLAGS}" LINK_FLAGS "${HZ_BIT_FLAG} ${HZ_CODE_COVERAGE_LINK_FLAGS} ${HZ_VALGRIND_LINK_FLAGS}")

	target_link_libraries(${HZ_EXE_NAME} ${HZ_LIB_NAME} )
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")


IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	IF(${HAZELCAST_LIBRARY_TYPE} MATCHES "STATIC")
		message( STATUS "Shall use the STATIC Hazelcast library for the tests.")
		add_definitions(-DHAZELCAST_USE_STATIC)
	ELSE(${HAZELCAST_LIBRARY_TYPE} MATCHES "SHARED")
		message( STATUS "Shall use the SHARED Hazelcast library for the tests.")
		add_definitions(-DHAZELCAST_USE_SHARED)
	ENDIF(${HAZELCAST_LIBRARY_TYPE} MATCHES "STATIC")

	set_target_properties(${HZ_EXE_NAME} PROPERTIES COMPILE_FLAGS " /EHsc ")
	target_link_libraries(${HZ_EXE_NAME} ${HZ_LIB_NAME}  )

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
