if build_core_opt
    # There seems to be no possible way to have a soversion included in the
    # import lib .lib in Windows. This causes problems with delvewheel when trying
    # to package the import libs in Python wheels. Just remove it for Windows....
    if host_machine.system() == 'windows'
        sbio_version_kwargs = {}
    else
        sbio_version_kwargs = {'version': '1', 'soversion': '1'}
    endif

    if sbio_build_xtc2
        libxtc2slim = shared_library(
            'xtc2slim',
            [
                'xtc2.cc',
                'traversal.cc',
            ],
            cpp_args: ['-DSBIO_BUILD_API'],
            include_directories: sbio_headers,
            install: true,
            install_dir: libsbio_install_dir,
            install_rpath: lib_install_rpath,
            kwargs: sbio_version_kwargs,
        )

        xtc2slim_dep = declare_dependency(
            link_with: [
                libxtc2slim,
            ],
            include_directories: sbio_headers,
        )

        # NOTE: ADD TO DEPS LIST
        deps_for_sbio += xtc2slim_dep
    endif
endif # build_core_opt


installed_formats_xtc2_headers = [
    'traversal.hh',
    'xtc2.hh',
    'xtc2_traits.hh',
]

if not sbio_as_wheel_opt and build_core_opt
    install_headers(
        installed_formats_xtc2_headers,
        install_dir: header_install_dir / 'sbio/formats/xtc2',
    )
else
    py3.install_sources(
        installed_formats_xtc2_headers,
        pure: false,
        subdir: 'sbio/include/sbio/formats/xtc2',
    )
endif # not sbio_as_wheel_opt and build_core_opt
