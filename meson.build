project(
	'haiku-react', 'cpp',
	version: '0.1.0',
	meson_version: '>=1.3',
	default_options: [
		'cpp_std=c++20',
		'warning_level=2',
		'buildtype=debugoptimized'
	]
)

cpp = meson.get_compiler('cpp')
node = find_program('node')
buildPaths = files('scripts/node-build-paths.cjs')

nodeIncludeDir = get_option('node_include_dir')
if nodeIncludeDir == ''
	nodeIncludeDir = run_command(node, buildPaths, 'headers', meson.project_source_root(), check: true).stdout().strip()
endif
nodeAddonApiDir = run_command(node, buildPaths, 'addon-api', meson.project_source_root(), check: true).stdout().strip()

subdir('native')

shared_module(
	'haiku_react',
	haikuReactSources,
	name_prefix: '',
	name_suffix: 'node',
	include_directories: [
		include_directories(nodeIncludeDir),
		include_directories(nodeAddonApiDir)
	],
	cpp_args: [
		'-DNAPI_VERSION=9',
		'-DNODE_ADDON_API_CPP_EXCEPTIONS',
		'-DNODE_ADDON_API_CPP_EXCEPTIONS_ALL',
		'-DNODE_ADDON_API_DISABLE_DEPRECATED'
	],
	dependencies: cpp.find_library('be')
)
