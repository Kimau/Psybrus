project "Engine_System_Scene"
	kind "StaticLib"
	language "C++"
	files {
    "./Shared/System/Scene/**.h", 
    "./Shared/System/Scene/**.inl", 
    "./Shared/System/Scene/**.cpp", 
    "./Platforms/Windows/System/Scene/**.h", 
    "./Platforms/Windows/System/Scene/**.inl", 
    "./Platforms/Windows/System/Scene/**.cpp", 
  }
	includedirs {
    "./Shared/",
    "./Platforms/Windows/",
    psybrusSDK .. "/External/bullet-2.81-rev2613/src",
    psybrusSDK .. "/External/freetype/include",
    psybrusSDK .. "/External/jsoncpp/include/",
    psybrusSDK .. "/External/libb64/include/",
    boostInclude,
  }

  -- GLEW config.
  defines { "GLEW_STATIC" }


	configuration "windows"
	    libdirs {
           boostLib
        }

   		links {
        -- Engine libs.
        "Engine_System",
        "Engine_System_Content",
        "Engine_System_Debug",
        "Engine_System_File",
        "Engine_System_Network",
        "Engine_System_Os",
        "Engine_System_Renderer",
        "Engine_System_Sound",

   			-- External libs.
        "External_BulletPhysics",
        "External_freetype",
        "External_jsoncpp",
   		}

   	configuration "vs2012"
   		links {
   		}