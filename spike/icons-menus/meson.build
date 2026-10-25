project(
	'haiku-react-spike-icons-menus', 'cpp',
	default_options: [
		'cpp_std=c++20',
		'warning_level=2',
		'buildtype=debugoptimized'
	]
)

cpp = meson.get_compiler('cpp')

executable(
	'icons-menus-spike',
	'Spike.cpp',
	include_directories: include_directories('/boot/system/develop/headers/private/interface'),
	dependencies: [
		cpp.find_library('be'),
		cpp.find_library('translation'),
		cpp.find_library('columnlistview')
	]
)
