if build_python_opt
    # Figure out dependencies
    deps_for_py_mod = deps_for_sbio + [pybind11_dep, py3_dep, sbio_dep]

    py_mod = py3.extension_module(
        'pyio',
        [
            'pyio.cc',
        ],
        install: true,
        install_dir: py_install_dir / 'sbio/io',
        include_directories: sbio_headers,
        dependencies : deps_for_py_mod,
        install_rpath: py_to_lib_install_rpath,
        build_rpath: py_to_lib_install_rpath,
        # link_args: py_mod_link_args,
    )
endif # build_python_opt

installed_py_bindings_io_headers = [
    'pyio.hh',
]

if not sbio_as_wheel_opt and build_core_opt
    install_headers(
        installed_py_bindings_io_headers,
        install_dir: header_install_dir / 'pysbio/io',
    )
else
    py3.install_sources(
        installed_py_bindings_io_headers,
        pure: false,
        subdir: 'sbio/include/pysbio/io',
    )
endif # not sbio_as_wheel_opt and build_core_opt
