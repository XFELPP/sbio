installed_core_headers = [
    'access_style.hh',
    'broker_group.hh',
    'broker.hh',
    'datasource.hh',
    'execution.hh',
    'io.hh',
    'result.hh',
    'storage.hh',
    'storage_view.hh',
    'stream.hh',
    'sync.hh',
    'topology.hh',
    'transaction.hh',
    'types.hh',
    'utility.hh',
]

if not sbio_as_wheel_opt and build_core_opt
    install_headers(
        installed_core_headers,
        install_dir: header_install_dir / 'sbio/core',
    )
else
    py3.install_sources(
        installed_core_headers,
        pure: false,
        subdir: 'sbio/include/sbio/core',
    )
endif # not sbio_as_wheel_opt and build_core_opt
