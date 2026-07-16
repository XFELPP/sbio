if build_core_opt
    if sbio_build_xtc1
        libxtc1slim = shared_library(
            'xtc1slim',
            [
                'xtc1.cc',
                'traversal.cc',
            ],
            include_directories: sbio_headers,
            install: true,
            install_dir: libsbio_install_dir,
            install_rpath: lib_install_rpath,
            version: '1',
            soversion: '1',
        )

        xtc1slim_dep = declare_dependency(
            link_with: [
                libxtc1slim,
            ],
            include_directories: sbio_headers,
        )

        # NOTE: ADD TO DEPS LIST
        deps_for_sbio += xtc1slim_dep
    endif
endif # build_core_opt

installed_formats_xtc1_headers = [
    'registry.hh',
    'traversal.hh',
    'xtc1.hh',
    'xtc1_schemas.hh',
    'xtc1_traits.hh',
]

if not sbio_as_wheel_opt and build_core_opt
    install_headers(
        installed_formats_xtc1_headers,
        install_dir: header_install_dir / 'sbio/formats/xtc1',
    )
else
    py3.install_sources(
        installed_formats_xtc1_headers,
        pure: false,
        subdir: 'sbio/include/sbio/formats/xtc1',
    )
endif # not sbio_as_wheel_opt and build_core_opt
