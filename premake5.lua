workspace "tECS"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
project "tECS"
	location "tECS"
	kind "SharedLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files {
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/include/**.h",
	}

	includedirs {
		"%{prj.name}/src",
	}

	filter "system:windows"
		cppdialect "C++20"
		staticruntime "Off"
		systemversion "latest" 
		
		defines {
			"TECS_BUILD_DLL",
		}

		postbuildcommands {
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")
		}

	filter "system:linux"
		cppdialect "gnu++20"
		staticruntime "Off"
		systemversion "latest" 

		defines {
			"TECS_BUILD_DLL",
		}

		postbuildcommands {
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")
		}

	filter "configurations:Debug"
		defines "_DEBUG"
		symbols "on"
		runtime "Debug"

	filter "configurations:Release"
		defines "_RELEASE"
		optimize "on"
		runtime "Release"

	filter "configurations:Dist"
		defines "_DIST"
		optimize "on"
		runtime "Release"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"

	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files {
		"%{prj.name}/**.h",
		"%{prj.name}/**.cpp"
	}

	includedirs {
		"tECS/include",
		"tECS/src"
	}

	links {
		"tECS"
	}

	filter "system:windows"
		cppdialect "C++20"
		staticruntime "off"
		systemversion "latest"
		
		defines {
			"TECS_PLATFORM_WINDOWS"
		}

	filter "system:linux"
		cppdialect "gnu++20"
		staticruntime "off"
		systemversion "latest"

		defines {
			"TECS_PLATFORM_LINUX"
		}

	filter "configurations:Debug"
		defines "_DEBUG"
		symbols "on"

	filter "configurations:Release"
		defines "_RELEASE"
		optimize "on"

	filter "configurations:Dist"
		defines "_DIST"
		optimize "on"