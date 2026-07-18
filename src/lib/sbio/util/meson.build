installed_util_headers = [
    'mpi.hh',
    'rc.hh',
    'string.hh',
]

if not sbio_as_wheel_opt and build_core_opt
    install_headers(
        installed_util_headers,
        install_dir: header_install_dir / 'sbio/util',
    )
else
    py3.install_sources(
        installed_util_headers,
        pure: false,
        subdir: 'sbio/include/sbio/util',
    )
endif # not sbio_as_wheel_opt and build_core_opt
