add_executable (embed tools/embed.c)
function (embed_data input name output)
	get_target_property (GENERATOR_EXE embed LOCATION)
	add_custom_command (
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${output}
		COMMAND ${GENERATOR_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/${input} ${name} ${output}
		DEPENDS embed
		MAIN_DEPENDENCY ${input}
	)
endfunction ()
