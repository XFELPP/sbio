if build_core_opt and sbio_build_sbiofmt
    executable(
        'test_random_ds',
        'test_random_ds.cc',
        include_directories: sbio_headers,
        dependencies: [ncarray_dep, sbio_dep, sbiofmt_dep],
        install: true,
        install_dir: exe_prefix,
        install_rpath: exe_install_rpath,
    )
endif
