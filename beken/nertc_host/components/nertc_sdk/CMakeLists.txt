if (CONFIG_NERTC_SDK)
armino_build_get_property(armino_soc ARMINO_SOC)
set(incs include)
endif()

armino_component_register(INCLUDE_DIRS "${incs}")

if (CONFIG_NERTC_SDK)
	get_filename_component(nertc_lib_name ${CMAKE_CURRENT_SOURCE_DIR}/libs/${ARMINO_SOC}/libnertc_sdk.a NAME)
	message(STATUS "nertc sdk name:${nertc_lib_name}")
	add_prebuilt_library(${nertc_lib_name} "${CMAKE_CURRENT_SOURCE_DIR}/libs/${ARMINO_SOC}/libnertc_sdk.a")
	target_link_libraries(${COMPONENT_LIB} INTERFACE ${nertc_lib_name})
endif()
