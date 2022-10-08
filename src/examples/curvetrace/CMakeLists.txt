###############################################################################
#C++ compilation
add_executable(curvetrace
	main.cpp
)

###############################################################################
#Linker settings
target_link_libraries(curvetrace
	scopehal
	scopeprotocols
	)

