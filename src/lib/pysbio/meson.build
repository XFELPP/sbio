if build_python_opt
    # Figure out dependencies
    deps_for_py_mod = deps_for_sbio + [pybind11_dep, py3_dep, sbio_dep]

    # Add DELAYLOAD for Windows as needed
    py_delay_lib_link_args = []
    if sbio_has_cuda and host_machine.system() == 'windows' and cpp.get_id() == 'msvc'
        py_delay_lib_link_args = [
            '/DELAYLOAD:nvcuda.dll',
            'delayimp.lib',
        ]
    endif

    py_mod = py3.extension_module(
        '_pysbio',
        [
            'pybroker_group.cc',
            'pydatasource.cc',
            'pyiteration.cc',
            'pysbio.cc',
            'pysourceset.cc',
            'utilities.cc',
        ],
        install: true,
        install_dir: pysbio_install_dir,
        include_directories: sbio_headers,
        dependencies : deps_for_py_mod,
        install_rpath: py_to_lib_install_rpath,
        build_rpath: py_to_lib_install_rpath,
        link_args: py_delay_lib_link_args,
    )

    if sbio_has_cuda
        # Provide the stub for Linux
        if host_machine.system() == 'linux'
            fs = import('fs')
            nvcc_bin = find_program('nvcc')
            nvcc_path = nvcc_bin.full_path()

            cuda_root = fs.parent(fs.parent(nvcc_path))

            # Check `lib64/stubs` and `lib/stubs` for driver stub
            cuda_stub_lib = cuda_root / 'lib64/stubs/libcuda.so'
            if not fs.exists(cuda_stub_lib)
                cuda_stub_lib = cuda_root / 'lib/stubs/libcuda.so'
            endif

            # For conda environments it's generally under targets
            if not fs.exists(cuda_stub_lib)
                cuda_stub_lib = cuda_root / 'targets/x86_64-linux/lib/stubs/libcuda.so'
            endif

            if fs.exists(cuda_stub_lib)
                message('**** Installing libcuda stub library: ' + cuda_stub_lib + ' ****')

                install_data(
                    cuda_stub_lib,
                    install_dir: py3.get_install_dir() / 'sbio/core/stubs'
                )
            else
                # The stub must be provided to prevent issues for CUDA-enabled builds
                # for users running without the CUDA driver (CPU-only mode).
                error('Cannot find CUDA driver stub (libcuda.so) from CUDA root: ' + cuda_root)
            endif
        endif # Linux guard
    endif # sbio_has_cuda
endif # build_python_opt

installed_py_bindings_headers = [
    'binding_builders.hh',
    'policies.hh',
    'pybroker_group.hh',
    'pydatasource.hh',
    'pyiteration.hh',
    'pysourceset.hh',
    'utilities.hh',
]

if not sbio_as_wheel_opt and build_core_opt
    install_headers(
        installed_py_bindings_headers,
        install_dir: header_install_dir / 'pysbio',
    )
else
    py3.install_sources(
        installed_py_bindings_headers,
        pure: false,
        subdir: 'sbio/include/pysbio',
    )
endif # not sbio_as_wheel_opt and build_core_opt

subdir('formats')
subdir('io')
subdir('execution')
subdir('brokers')
subdir('broker_groups')
subdir('core')
subdir('datasources')
