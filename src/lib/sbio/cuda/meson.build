installed_cuda_headers = [
    'utilities.hh',
]

if not sbio_as_wheel_opt and build_core_opt
    install_headers(
        installed_cuda_headers,
        install_dir: header_install_dir / 'sbio/cuda/',
    )
else
    py3.install_sources(
        installed_cuda_headers,
        pure: false,
        subdir: 'sbio/include/sbio/cuda/',
    )
endif # not sbio_as_wheel_opt and build_core_opt
