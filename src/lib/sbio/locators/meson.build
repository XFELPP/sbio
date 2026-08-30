installed_locators_headers = [
    'custom_lambda.hh',
    'path_pattern.hh',
    'single_file.hh',
]

if not sbio_as_wheel_opt and build_core_opt
    install_headers(
        installed_locators_headers,
        install_dir: header_install_dir / 'sbio/locators',
    )
else
    py3.install_sources(
        installed_locators_headers,
        pure: false,
        subdir: 'sbio/include/sbio/locators',
    )
endif # not sbio_as_wheel_opt and build_core_opt
