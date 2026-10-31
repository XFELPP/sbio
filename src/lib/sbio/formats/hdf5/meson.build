if build_core_opt
    # There seems to be no possible way to have a soversion included in the
    # import lib .lib in Windows. This causes problems with delvewheel when trying
    # to package the import libs in Python wheels. Just remove it for Windows....
    if host_machine.system() == 'windows'
        sbio_version_kwargs = {}
    else
        sbio_version_kwargs = {'version': '1', 'soversion': '1'}
    endif

    if sbio_build_hdf5
        deps_for_sbioH5driver = [hdf5_dep]
        libsbioH5driver = shared_library(
            'sbioH5driver',
            [
                'vfd_driver.cc',
            ],
            cpp_args: ['-DSBIO_BUILD_API'],
            include_directories: sbio_headers,
            dependencies: deps_for_sbioH5driver,
            install: true,
            install_dir: libsbio_install_dir,
            install_rpath: lib_install_rpath,
            kwargs: sbio_version_kwargs,
            link_args: fmt_delay_lib_link_args,
        )

        sbioh5driver_dep = declare_dependency(
            link_with: [
                libsbioH5driver,
            ],
            include_directories: sbio_headers,
        )

        # NOTE: ADD TO DEPS LIST
        deps_for_sbio += sbioh5driver_dep
    endif
endif # build_core_opt

installed_formats_hdf5_headers = [
    'cxi_spec.hh',
    'hdf5_format.hh',
    'vfd_driver.hh',
]

if not sbio_as_wheel_opt and build_core_opt
    install_headers(
        installed_formats_hdf5_headers,
        install_dir: header_install_dir / 'sbio/formats/hdf5',
    )
else
    py3.install_sources(
        installed_formats_hdf5_headers,
        pure: false,
        subdir: 'sbio/include/sbio/formats/hdf5',
    )
endif # not sbio_as_wheel_opt and build_core_opt
