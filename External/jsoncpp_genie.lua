project ( EXTERNAL_PROJECT_PREFIX .. "jsoncpp" )
	kind ( EXTERNAL_PROJECT_KIND )
	language "C++"
	files { "./jsoncpp/src/lib_json/**.h", "./jsoncpp/src/lib_json/**.cpp" }
	includedirs { "./jsoncpp/include" }