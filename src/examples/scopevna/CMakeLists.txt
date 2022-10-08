###############################################################################
#C++ compilation
add_executable(scopevna
	main.cpp
)

###############################################################################
#Linker settings
target_link_libraries(scopevna
	scopehal
	scopeprotocols
	${LIBFFTS_LIBRARIES}
	)

