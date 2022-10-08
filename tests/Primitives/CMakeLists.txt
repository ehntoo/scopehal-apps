add_executable(Primitives
	main.cpp

	BlackmanHarrisWindow.cpp
	Convert8BitSamples.cpp
	Convert16BitSamples.cpp
	Sampling.cpp
)

###############################################################################
#Linker settings
target_link_libraries(Primitives
	scopehal
	scopeprotocols
	Catch2::Catch2
	)
