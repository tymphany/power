AUX_SOURCE_DIRECTORY(. src_files)

ADD_EXECUTABLE(bq-drv ${src_files})

find_package(Threads)

target_link_libraries(bq-drv ${CMAKE_THREAD_LIBS_INIT})

INSTALL(TARGETS bq-drv
	RUNTIME DESTINATION bin
)


