installed_execution_headers = [
    'mpi.hh',
    'mpi_threaded.hh',
    'serial.hh',
    'threaded.hh',
]

if not sbio_as_wheel_opt and build_core_opt
    install_headers(
        installed_execution_headers,
        install_dir: header_install_dir / 'sbio/execution',
    )
else
    py3.install_sources(
        installed_execution_headers,
        pure: false,
        subdir: 'sbio/include/sbio/execution',
    )
endif # not sbio_as_wheel_opt and build_core_opt
