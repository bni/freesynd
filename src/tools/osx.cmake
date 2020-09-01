# Since as of CMake 2.8.4, built-in resource support does not correctly handle
# directories as resources, I am handling resource commands myself.

include (BundleUtilities)

add_executable (copy_but tools/copy_but.cpp)
get_target_property (COPY_BUT_EXE copy_but LOCATION)

function (get_resources_path var target)
	get_target_property (exe ${target} LOCATION)
	get_dotapp_dir (${exe} app_dir)
	set (${var} "${app_dir}/Contents/Resources" PARENT_SCOPE)
endfunction ()

function (get_framework_path var target)
	get_target_property (exe ${target} LOCATION)
	get_dotapp_dir (${exe} app_dir)
	set (${var} "${app_dir}/Contents/Frameworks" PARENT_SCOPE)
endfunction ()

function (get_exe_path var target)
	get_target_property (exe ${target} LOCATION)
	get_filename_component (exe_path ${exe} PATH)
	set (${var} "${exe_path}" PARENT_SCOPE)
endfunction ()

function (_prep_folder target folder)
	add_custom_command (TARGET ${target} PRE_LINK
		COMMAND ${CMAKE_COMMAND} -E make_directory ${folder}
		VERBATIM
	)
endfunction ()

function (copy_resource_file target comment source) # optional param: dest
	if ("${ARGV3}" STREQUAL "")
		get_filename_component (dest ${source} NAME)
	else ()
		set (dest ${ARGV3})
	endif ()
	get_resources_path (resources_dir ${target})
	_prep_folder (${target} ${resources_dir})
	add_custom_command (TARGET ${target} PRE_LINK
		COMMAND ${CMAKE_COMMAND} -E copy ${source} ${resources_dir}/${dest}
		COMMENT ${comment}
		VERBATIM
	)
endfunction ()

function (copy_resource_folder target comment source) # optional param: dest
	if ("${ARGV3}" STREQUAL "")
		get_filename_component (dest ${source} NAME)
	else ()
		set (dest ${ARGV3})
	endif ()
	get_resources_path (resources_dir ${target})
	_prep_folder (${target} ${resources_dir} ${comment})
	add_dependencies (${target} copy_but)
	add_custom_command (TARGET ${target} PRE_LINK
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${resources_dir}/${dest}
		COMMAND ${COPY_BUT_EXE} ${source} ${resources_dir}/${dest} .svn
		COMMENT ${comment}
		VERBATIM
	)
endfunction ()

function (include_libs target comment)
	get_framework_path (framework_dir ${target})
	_prep_folder (${target} ${framework_dir} ${comment})
	get_target_property (exe ${target} LOCATION)
	add_custom_command (TARGET ${target} POST_BUILD
		# The || clause is to get the target wiped out if this post-build
		# operation fails for any reason.  Otherwise, you may re-run make
		# and think that the target is fully built when it's not.
		COMMAND ${CMAKE_SOURCE_DIR}/src/tools/include_libs.sh ${exe} ${framework_dir} ../Frameworks || (rm ${exe} \; exit 1)
		COMMENT ${comment}
	)
endfunction ()

