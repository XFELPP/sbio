# Get libraries for any data formats first, then core
# The libs/deps will be added to `deps_for_sbio`
subdir('formats')

# Mostly header only, we'll just build the lib here
if build_core_opt
    core_sources = ['io/posix.cc']

    if sbio_has_cuda
        core_sources += ['io/cufile.cc']
    endif

    if sbio_build_xtc1
        core_sources += ['formats/xtc1/xtc1_traits.cc']
    endif

    if sbio_build_xtc2
        core_sources += ['formats/xtc2/xtc2_traits.cc']
    endif

    if sbio_build_sbiofmt
        core_sources += ['formats/random/random_traits.cc']
    endif

    # There seems to be no possible way to have a soversion included in the
    # import lib .lib in Windows. This causes problems with delvewheel when trying
    # to package the import libs in Python wheels. Just remove it for Windows....
    if host_machine.system() == 'windows'
        sbio_version_kwargs = {}
    else
        sbio_version_kwargs = {'version': '1', 'soversion': '1'}
    endif

    libsbio = shared_library(
        'sbio',
        core_sources,
        cpp_args: ['-DSBIO_BUILD_API'],
        dependencies: deps_for_sbio,
        include_directories: sbio_headers,
        install: true,
        install_dir: libsbio_install_dir,
        install_rpath: lib_install_rpath,
        kwargs: sbio_version_kwargs,
    )

    sbio_dep = declare_dependency(
        link_with: [
            libsbio,
        ],
        dependencies: deps_for_sbio,
        include_directories: sbio_headers,
    )
else
    libsbio = cpp.find_library('sbio', required: true)
    found_libs = [libsbio]
    if sbio_build_xtc1
        libxtc1slim = cpp.find_library('xtc1slim', required: true)
        found_libs += [libxtc1slim]
    endif
    if sbio_build_xtc2
        libxtc2slim = cpp.find_library('xtc2slim', required: true)
        found_libs += [libxtc2slim]
    endif
    if sbio_build_sbiofmt
        libsbiofmt = cpp.find_library('sbiofmt', required: true)
        found_libs += [libsbiofmt]
    endif
    sbio_dep = declare_dependency(
        dependencies: deps_for_sbio + found_libs,
        include_directories: sbio_headers,
    )
endif

subdir('core')
subdir('execution')
subdir('io')
subdir('storage')
subdir('util')

installed_export_headers = [
    'export_macro.hh',
]

if not sbio_as_wheel_opt and build_core_opt
    install_headers(
        installed_export_headers,
        install_dir: header_install_dir / 'sbio/',
    )
else
    py3.install_sources(
        installed_export_headers,
        pure: false,
        subdir: 'sbio/include/sbio/',
    )
endif # not sbio_as_wheel_opt and build_core_opt
