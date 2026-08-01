if build_python_opt
    # Figure out dependencies
    deps_for_py_mod = deps_for_sbio + [pybind11_dep, py3_dep, sbio_dep]

    py_mod = py3.extension_module(
        'pyformat_traits',
        [
            'pyformat_traits.cc',
        ],
        install: true,
        install_dir: pysbio_fmts_install_dir,
        include_directories: sbio_headers,
        dependencies : deps_for_py_mod,
        install_rpath: py_to_lib_install_rpath,
        build_rpath: py_to_lib_install_rpath,
        # link_args: py_mod_link_args,
    )

    py_mod = py3.extension_module(
        'pyxtc2',
        [
            'pyxtc2.cc',
        ],
        install: true,
        install_dir: pysbio_fmts_install_dir,
        include_directories: sbio_headers,
        dependencies : deps_for_py_mod,
        install_rpath: py_to_lib_install_rpath,
        build_rpath: py_to_lib_install_rpath,
        # link_args: py_mod_link_args,
    )
endif # build_python_opt

installed_py_bindings_formats_headers = [
    'pyformat_traits.hh',
]

if not sbio_as_wheel_opt and build_core_opt
    install_headers(
        installed_py_bindings_formats_headers,
        install_dir: header_install_dir / 'pysbio/formats',
    )
else
    py3.install_sources(
        installed_py_bindings_formats_headers,
        pure: false,
        subdir: 'sbio/include/pysbio/formats',
    )
endif # not sbio_as_wheel_opt and build_core_opt
