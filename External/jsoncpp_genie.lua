if PsyProjectExternalLib( "jsoncpp", "C++" ) then
	configuration "*"
		kind ( EXTERNAL_PROJECT_KIND )
		files { "./jsoncpp/src/lib_json/**.h", "./jsoncpp/src/lib_json/**.cpp" }
		includedirs { "./jsoncpp/include" }
end
