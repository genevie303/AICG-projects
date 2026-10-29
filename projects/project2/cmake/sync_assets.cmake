if(NOT DEFINED SOURCE_ASSET_DIR OR NOT DEFINED OUTPUT_ASSET_DIR)
    message(FATAL_ERROR "Source and output asset directories are required")
endif()

file(GLOB_RECURSE asset_files LIST_DIRECTORIES FALSE "${SOURCE_ASSET_DIR}/*")
foreach(asset IN LISTS asset_files)
    file(RELATIVE_PATH relative_path "${SOURCE_ASSET_DIR}" "${asset}")
    set(output_path "${OUTPUT_ASSET_DIR}/${relative_path}")
    get_filename_component(output_dir "${output_path}" DIRECTORY)
    file(MAKE_DIRECTORY "${output_dir}")

    if(relative_path MATCHES "^shaders/")
        # Recreate the link after an editor replaces the source file on save.
        # A separate volume cannot support a hard link, so copy in that case.
        file(CREATE_LINK "${asset}" "${output_path}" COPY_ON_ERROR)
    else()
        file(COPY "${asset}" DESTINATION "${output_dir}")
    endif()
endforeach()
