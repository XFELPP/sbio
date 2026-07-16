installed_storage_headers = [
    'host_buffer.hh',
    'mpi_shared.hh',
    'threaded_local_buffer.hh',
]

if not sbio_as_wheel_opt and build_core_opt
    install_headers(
        installed_storage_headers,
        install_dir: header_install_dir / 'sbio/storage',
    )
else
    py3.install_sources(
        installed_storage_headers,
        pure: false,
        subdir: 'sbio/include/sbio/storage',
    )
endif # not sbio_as_wheel_opt and build_core_opt
