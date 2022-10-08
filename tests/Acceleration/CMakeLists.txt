add_executable(Acceleration
	main.cpp

	Buffers.cpp
)

###############################################################################
#Linker settings
target_link_libraries(Acceleration
	scopehal
	scopeprotocols
	Catch2::Catch2
	)
