add_executable( sparkle_client main.cpp )
if( UNIX AND NOT APPLE )
  set(rt_library rt )
endif()

target_link_libraries( sparkle_client
                       PRIVATE bitcoin bts_keyhotee bts_wallet bts_rpc bts_cli bts_blockchain bts_db bts_net bts_client bts_utilities fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} )
target_include_directories( sparkle_client 
                            PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include" )

IF(WIN32 AND "${TARGET_ARCHITECTURE}" STREQUAL "x86")
  SET_TARGET_PROPERTIES(sparkle_client PROPERTIES LINK_FLAGS "/LARGEADDRESSAWARE")
ENDIF()

if (ENABLE_INSTALLER)
  install(TARGETS sparkle_client RUNTIME DESTINATION bin)
endif(ENABLE_INSTALLER)
