# Warning! This code was automatically generated for projects
# without default 'CMakeLists.txt' file.

set(app_sources
	"main.cpp"
	"EVE_commands.cpp"
	"EVE_target.cpp"
	"tft.cpp"
	"tft_data.c"
)

idf_component_register(SRCS ${app_sources})
