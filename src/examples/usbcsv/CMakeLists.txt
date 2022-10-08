###############################################################################
#C++ compilation
add_executable(usbcsv
	main.cpp
)

###############################################################################
#Linker settings
target_link_libraries(usbcsv
	scopehal
	scopeprotocols
	)

