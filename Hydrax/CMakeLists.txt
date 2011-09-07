
cmake_minimum_required(VERSION 2.6)
#cmake_policy(SET CMP0003 NEW)

project(Hydrax)


set(HEADER_FILES
	CfgFileManager.h
	GPUNormalMapManager.h
	MaterialManager.h
	TextureManager.h
	DecalsManager.h
	Help.h
	Mesh.h
	Enums.h
	Hydrax.h
	Prerequisites.h
	GodRaysManager.h
	Image.h
	RttManager.h
	Noise/Noise.h
	Noise/Perlin/Perlin.h
	Noise/FFT/FFT.h
	Modules/Module.h
	Modules/ProjectedGrid/ProjectedGrid.h
	Modules/RadialGrid/RadialGrid.h
	Modules/SimpleGrid/SimpleGrid.h
)
set(SOURCE_FILES
	CfgFileManager.cpp
	Help.cpp
	Prerequisites.cpp
	DecalsManager.cpp
	Hydrax.cpp
	RttManager.cpp
	Enums.cpp
	Image.cpp
	TextureManager.cpp
	GodRaysManager.cpp
	MaterialManager.cpp
	GPUNormalMapManager.cpp
	Mesh.cpp
	Noise/Noise.cpp
	Noise/Perlin/Perlin.cpp
	Noise/FFT/FFT.cpp
	Modules/Module.cpp
	Modules/ProjectedGrid/ProjectedGrid.cpp
	Modules/RadialGrid/RadialGrid.cpp
	Modules/SimpleGrid/SimpleGrid.cpp
)


SET(HYDRAX_VERSION 0)
SET(HYDRAX_VERSION_MAJOR 5)

include_directories (. ${OGRE_INCLUDE_DIRS})
add_library(Hydrax STATIC ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(Hydrax PROPERTIES
	COMPILE_DEFINITIONS HYDRAX_LIB
)
