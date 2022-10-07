add_executable(Acceleration
	main.cpp

	Buffers.cpp
)

catch_discover_tests(Acceleration)

###############################################################################
#Linker settings
target_link_libraries(Acceleration
	scopehal
	scopeprotocols
	Catch2::Catch2
	)
