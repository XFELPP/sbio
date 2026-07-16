installed_io_headers = [
    'cufile.hh',
    'posix.hh',
]

if not sbio_as_wheel_opt and build_core_opt
    install_headers(
        installed_io_headers,
        install_dir: header_install_dir / 'sbio/io',
    )
else
    py3.install_sources(
        installed_io_headers,
        pure: false,
        subdir: 'sbio/include/sbio/io',
    )
endif # not sbio_as_wheel_opt and build_core_opt
