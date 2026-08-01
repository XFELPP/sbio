subdir('xtc1')
subdir('xtc2')

installed_formats_headers = [
    'format_traits.hh',
]

if not sbio_as_wheel_opt and build_core_opt
    install_headers(
        installed_formats_headers,
        install_dir: header_install_dir / 'sbio/formats',
    )
else
    py3.install_sources(
        installed_formats_headers,
        pure: false,
        subdir: 'sbio/include/sbio/formats',
    )
endif # not sbio_as_wheel_opt and build_core_opt
