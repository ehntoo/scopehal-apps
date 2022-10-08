###############################################################################
#C++ compilation
add_executable(impulse
	main.cpp
)

###############################################################################
#Linker settings
target_link_libraries(impulse
	scopehal
	scopeprotocols
	${LIBFFTS_LIBRARIES}
	)

