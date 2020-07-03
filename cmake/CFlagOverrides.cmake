
if (MSVC AND NOT USE_MSVC_RUNTIME_LIBRARY_DLL)
	foreach (flag CMAKE_C_FLAGS_DEBUG_INIT CMAKE_C_FLAGS_MINSIZEREL_INIT CMAKE_C_FLAGS_RELEASE_INIT CMAKE_C_FLAGS_RELWITHDEBINFO_INIT CMAKE_CXX_FLAGS_DEBUG_INIT CMAKE_CXX_FLAGS_MINSIZEREL_INIT CMAKE_CXX_FLAGS_RELEASE_INIT CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT)
        if (${flag} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}")
        endif()
        if (${flag} MATCHES "/MDd")
            string(REGEX REPLACE "/MDd" "/MTd" ${flag} "${${flag}}")
        endif()
    endforeach()
	foreach (flag CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
        if (${flag} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}")
        endif()
        if (${flag} MATCHES "/MDd")
            string(REGEX REPLACE "/MDd" "/MTd" ${flag} "${${flag}}")
        endif()
    endforeach()
elseif (MSVC AND USE_MSVC_RUNTIME_LIBRARY_DLL)
	foreach (flag CMAKE_C_FLAGS_DEBUG_INIT CMAKE_C_FLAGS_MINSIZEREL_INIT CMAKE_C_FLAGS_RELEASE_INIT CMAKE_C_FLAGS_RELWITHDEBINFO_INIT CMAKE_CXX_FLAGS_DEBUG_INIT CMAKE_CXX_FLAGS_MINSIZEREL_INIT CMAKE_CXX_FLAGS_RELEASE_INIT CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT)
        if (${flag} MATCHES "/MT")
            string(REGEX REPLACE "/MT" "/MD" ${flag} "${${flag}}")
        endif()
        if (${flag} MATCHES "/MTd")
            string(REGEX REPLACE "/MTd" "/MDd" ${flag} "${${flag}}")
        endif()
    endforeach()
	foreach (flag CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
        if (${flag} MATCHES "/MT")
            string(REGEX REPLACE "/MT" "/MD" ${flag} "${${flag}}")
        endif()
        if (${flag} MATCHES "/MTd")
            string(REGEX REPLACE "/MTd" "/MDd" ${flag} "${${flag}}")
        endif()
    endforeach()
endif()