project(
	'haiku-react-spike', 'cpp',
	version: '0.0.1',
	default_options: [
		'cpp_std=c++20',
		'warning_level=2',
		'buildtype=debugoptimized'
	]
)

cpp = meson.get_compiler('cpp')

shared_module(
	'spike',
	'Spike.cpp',
	name_prefix: '',
	name_suffix: 'node',
	include_directories: include_directories(
		'/boot/system/develop/headers/node',
		'../node_modules/node-addon-api'
	),
	cpp_args: [
		'-DNAPI_VERSION=9',
		'-DNODE_ADDON_API_CPP_EXCEPTIONS',
		'-DNODE_ADDON_API_CPP_EXCEPTIONS_ALL',
		'-DNODE_ADDON_API_DISABLE_DEPRECATED'
	],
	dependencies: cpp.find_library('be')
)
