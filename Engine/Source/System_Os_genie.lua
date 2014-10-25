project "Engine_System_Os"
  kind "StaticLib"
  language "C++"
  files {
    "./Shared/System/Os/**.h", 
    "./Shared/System/Os/**.inl", 
    "./Shared/System/Os/**.cpp", 
  }
  includedirs {
    "./Shared/",
    "../../External/jsoncpp/include/",
    "../../External/libb64/include/",
    boostInclude,
  }

  links {
    -- Engine libs.
    "Engine_System",
  }

  configuration "linux"
    files {
      "./Platforms/Linux/System/Os/*.h", 
      "./Platforms/Linux/System/Os/*.inl", 
      "./Platforms/Linux/System/Os/*.cpp", 
    }
    includedirs {
      "./Platforms/Linux/",
    }

  configuration "windows"
    files {
      "./Platforms/Windows/System/Os/*.h", 
      "./Platforms/Windows/System/Os/*.inl", 
      "./Platforms/Windows/System/Os/*.cpp", 
    }
    includedirs {
      "./Platforms/Windows/",
   }

   libdirs {
     boostLib
   }