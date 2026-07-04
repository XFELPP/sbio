project(
    'sbio',
    'cpp',
    version: '0.1.0',
    default_options: [
        'cpp_std=c++23,c++latest',
        'cuda_std=c++20',
        'optimization=3',
    ],
)

add_languages('cpp', native: true, required: true)
sbio_has_cuda = add_languages('cuda', required: false)

if sbio_has_cuda
    # Source code guards CUDA specific features behind SBIO_HAS_CUDA
    add_project_arguments('-DSBIO_HAS_CUDA', language: ['cpp', 'cuda'])

    cuda = meson.get_compiler('cuda')
    cuda_version = cuda.version()
endif


cpp = meson.get_compiler('cpp')

# Check compiler versions (for C++23 suppport especially)
if cpp.get_id() == 'gcc'
    gcc_version = cpp.version()
    if not gcc_version.version_compare('>=13')
        if gcc_version.version_compare('>=12')
            warning('GCC >=13 is recommended, however, GCC 12+ should work.')
        else
            error('Builds with GCC require version >=12')
        endif
    endif

    if sbio_has_cuda
        # Too new a compiler for CUDA support also causes problems
        if cuda_version.version_compare('<12.4') and not gcc_version.version_compare('<13')
            error('For CUDA < 12.4 builds, must use GCC < 13')
        elif cuda_version.version_compare('<12.8') and not gcc_version.version_compare('<14')
            error('For CUDA < 12.8 builds, must use GCC < 14')
        elif not gcc_version.version_compare('<15')
            error('For CUDA 12.8+ builds, must use GCC < 15')
        endif
    endif
elif cpp.get_id() == 'clang'
    clang_version = cpp.version()
    if not clang_version.version_compare('>=16')
        error('Builds with clang require version >=16')
    endif

    if sbio_has_cuda
        # Too new a compiler for CUDA support also causes problems
        if cuda_version.version_compare('<12.4') and not clang_version.version_compare('<17')
            error('For CUDA < 12.4 builds, must use clang < 17')
        elif cuda_version.version_compare('<12.8') and not clang_version.version_compare('<19')
            error('For CUDA < 12.8 builds, must use clang < 19')
        elif not clang_version.version_compare('<20')
            error('For CUDA 12.8+ builds, must use clang < 20')
        endif
    endif
elif cpp.get_id() == 'msvc'
    msvc_version = cpp.version()
    if not msvc_version.version_compare('>=19.30')
        error('Builds with msvc require version >=19.30')
    endif

    if sbio_has_cuda
        # Too new a compiler for CUDA support also causes problems
        if cuda_version.version_compare('<12.4') and not msvc_version.version_compare('<19.38')
            error('For CUDA < 12.4 builds, must use msvc < 19.38 (VS 2022 < 17.8)')
        elif cuda_version.version_compare('<12.8') and not msvc_version.version_compare('<19.41')
            error('For CUDA < 12.8 builds, must use msvc < 19.41 (VS 2022 < 17.11)')
        elif not msvc_version.version_compare('<19.50')
            error('For CUDA 12.8+ builds, must use msvc < 19.50 (VS 2022 < 18.0)')
        endif
    endif
endif

# For Windows adjust for limit of 65,536 COFF sections
if cpp.get_id() == 'msvc'
    add_project_arguments('/bigobj', '/DNOMINMAX', language: 'cpp')
    if sbio_has_cuda
        add_project_arguments(
            '-Xcompiler=/bigobj',
            '-Xcompiler=/DNOMINMAX',
            '-D_SOURCE_LOCATION_',
            language: 'cuda',
        )
    endif
endif

#### Fetch Build Options ####
#############################
# Core options
build_core_opt = get_option('build_core')
sbio_data_formats_opt = get_option('sbio_data_formats')
build_mpi_opt = get_option('build_mpi')
build_python_opt = get_option('build_python')
sbio_cuda_archs_opt = get_option('sbio_cuda_archs')
sbio_as_wheel_opt = get_option('sbio_as_wheel')

if not build_python_opt and sbio_as_wheel_opt
    warning('Requested wheel-build, but not Python bindings. Overriding and will build Python bindings!')
    build_python_opt = true
endif

# Add-ons and development
build_examples_opt = get_option('build_examples')

#### Parse CUDA architectures and set CUDA args ####
####################################################

# NOTE: I think there may be a clang-frontend for CUDA... These may not all work if
# using that instead of nvcc specifically!
sbio_cuda_args = [
    # On Windows, inspite of overrides and global settings, C++14 gets chosen
    # as the CUDA standard unless this gets added here
    '--std=c++20',
    '--extended-lambda',
    '--expt-relaxed-constexpr',
    # silence warning: declared but not referenced (e.g. pybind, stl...)
    '-Xcudafe', '--diag_suppress=445',
    # silence warning: long double treated as double
    '-Xcudafe', '--diag_suppress=20208',
    # silence warning: float overflow in CUDA cmath header (Windows)
    # TODO: Try to limit this to includes/callsites eventually
    '-Xcudafe', '--diag_suppress=221',
    # silence warning: field of class type without a DLL interface (Windows)
    # TODO: Try to limit this to includes/callsites eventually (tried a bit already)
    '-Xcudafe', '--diag_suppress=1394',
    # silence warning: __host__ called from __host__ __device__ (Linux - use of dispatch in GPUEngine)
    # TODO: Try to limit this to includes/callsites eventually (tried a bit already)
    '-Xcudafe', '--diag_suppress=20011',
    # silence warning: unrecognized GCC pragma (Linux)
    '-Xcudafe', '--diag_suppress=1675',
]
if sbio_has_cuda
    if cuda_version.version_compare('>=12.8')
        sbio_cuda_args += [
            '-static-global-template-stub=false',
            '-device-entity-has-hidden-visibility=false',
        ]
    endif
endif

sbio_cuda_args += ['-Xfatbin', '-compress-all']

if sbio_cuda_archs_opt == 'all'
    # Setup a fat binary for Turing, Ampere, Ada and Hopper
    sbio_cuda_args += [
        '-gencode', 'arch=compute_75,code=sm_75',
        '-gencode', 'arch=compute_80,code=sm_80',
        '-gencode', 'arch=compute_86,code=sm_86',
        '-gencode', 'arch=compute_89,code=sm_89',
        '-gencode', 'arch=compute_90,code=[sm_90,compute_90]',
    ]
else
    # Parse comma-separated architectures ('80', or '75,90')
    # Last value will generate [sm_ARCH,compute_ARCH]
    selected_cuda_archs = sbio_cuda_archs_opt.split(',')

    num_archs = selected_cuda_archs.length()
    last_arch = selected_cuda_archs[num_archs - 1].strip()
    foreach arch : selected_cuda_archs
        arch = arch.strip()
        if arch != ''
            if arch == last_arch
                # Generate PTX in this case, for forward-compatibility
                sbio_cuda_args += [
                    '-gencode', 'arch=compute_' + arch + ',code=[sm_' + arch + ',compute_' + arch + ']'
                ]
            else
                sbio_cuda_args += [
                    '-gencode', 'arch=compute_' + arch + ',code=sm_' + arch
                ]
            endif
        endif
    endforeach
endif

#### Parse Chosen Data Formats to Build ####
############################################
sbio_build_xtc1 = false
sbio_build_xtc2 = false

if sbio_data_formats_opt == 'all'
    sbio_build_xtc1 = true
    sbio_build_xtc2 = true
else
    selected_formats = sbio_data_formats_opt.split(',')
    foreach data_fmt : selected_formats
        data_fmt = data_fmt.strip()
        if data_fmt == 'xtc1'
            sbio_build_xtc1 = true
        elif data_fmt == 'xtc2'
            sbio_build_xtc2 = true
        else
            warning('Provided unrecognized data format which will be skipped: ' + data_fmt)
        endif
    endforeach
endif

if sbio_build_xtc1
    add_project_arguments('-DSBIO_HAS_XTC1', language: ['cpp', 'cuda'])
    message('Will build XTC1 data format support.')
endif
if sbio_build_xtc2
    add_project_arguments('-DSBIO_HAS_XTC2', language: ['cpp', 'cuda'])
    message('Will build XTC2 data format support.')
endif

#### Main Python Setup ####
###########################

pymod = import('python')

py3 = pymod.find_installation(pure: false)
py3_dep = py3.dependency()

min_python_ver = '3.8'
py_ver = py3.language_version()
if py_ver.version_compare(f'<@min_python_ver@')
    error(f'Minimum supported Python version is @min_python_ver@, found @py_ver@')
endif

# Define installation directory relative to prefix
python_site_packages = 'lib' / 'python' + py_ver / 'site-packages'

#### Core Dependencies ####
###########################
deps_for_sbio = []

#### ncarray
libncarray = cpp.find_library('ncarray', required: false)
if libncarray.found()
    ncarray_dep = libncarray
else
    message('ncarray not found via `find_library` - checking via Python')
    nc_libs_raw = run_command(
        py3, '-c',
        'import glob, ncarray; print("\\n".join(glob.glob(f"{ncarray.get_lib_dir()}/libnc*")))',
        check: false,
    ).stdout().strip().split('\n')

    if (nc_libs_raw.length() == 0) or (nc_libs_raw.length() == 1  and nc_libs_raw[0] == '')
        message('ncarray not found via Python - will use subproject.')
        ncarray_proj = subproject('ncarray')
        ncarray_dep = ncarray_proj.get_variable('ncarray_dep')
    else
        fs = import('fs')
        nc_link_args = []
        foreach lib : nc_libs_raw
            libdir = fs.parent(lib)
            libname = fs.name(lib)
            nc_link_args += [
                '-L' + libdir,
                '-l:' + libname,
            ]
        endforeach

        nc_inc = run_command(
            py3, '-c', 'import ncarray; print(ncarray.get_include())',
            check: false,
        ).stdout().strip()

        ncarray_dep = declare_dependency(
            link_args: nc_link_args,
            include_directories: include_directories(nc_inc),
        )
    endif # if nca_libs_raw empty (Python search for ncarray)
endif
deps_for_sbio += ncarray_dep

#### MPI
if build_mpi_opt == 'none'
    message('Skipping MPI support.')
else
    try_sys = false
    try_mpich = false
    try_ompi = false
    if build_mpi_opt == 'auto'
        try_sys = true
        try_mpich = true
        try_ompi = true
    endif
    if build_mpi_opt == 'mpich'
        try_mpich = true
    endif
    if build_mpi_opt == 'ompi'
        try_ompi = true
    endif

    found_mpi = false
    bundle_mpi = false # Only bundle if using subproject
    if try_sys
        mpi_dep = dependency('mpi', language: 'cpp', required: false)
        if mpi_dep.found()
            found_mpi = true
        endif
    endif
    if not found_mpi and try_mpich
        message('Will try building MPICH subproject.')
        mpi_proj = subproject('mpich')
        mpi_dep = mpi_proj.get_variable('mpich_dep')
        bundle_mpi = true
        found_mpi = true
    endif
    if not found_mpi and try_ompi
        message('Will try building OpenMPI subproject.')
        mpi_proj = subproject('ompi')
        mpi_dep = mpi_proj.get_variable('ompi_dep')
        bundle_mpi = true
        found_mpi = true
    endif

    if not found_mpi
        warning('MPI was requested, but was not built!')
    else
        add_project_arguments('-DSBIO_HAS_MPI', language: ['cpp', 'cuda'])
        deps_for_sbio += mpi_dep
    endif
endif # MPI builds

#### OpenMP
#openmp_dep = dependency('openmp', required: true)

#### pybind11
if build_python_opt
    pybind11_dep = dependency('pybind11', required: false)
    if not pybind11_dep.found()
        message('pybind11 not found, but required for Python bindings - will build subproject.')
        pybind11_proj = subproject('pybind11')
        pybind11_dep = pybind11_proj.get_variable('pybind11_dep')
    endif # pybind11 not found
endif


#### XAlgosPP
libxalgospp = cpp.find_library('xalgospp', required: false)
if libxalgospp.found()
    xalgospp_dep = libxalgospp
else
    message('XAlgosPP not found via `find_library` - checking via Python')
    xalg_libs_raw = run_command(
        py3, '-c',
        'import glob, xalgospp; print("\\n".join(glob.glob(f"{xalgospp.get_lib_dir()}/lib*xalgo*")))',
        check: false,
    ).stdout().strip().split('\n')

    if (xalg_libs_raw.length() == 0) or (xalg_libs_raw.length() == 1  and xalg_libs_raw[0] == '')
        message('XAlgosPP not found via Python - will use subproject.')
        xalg_proj = subproject('xalgospp')
        xalgospp_dep = xalg_proj.get_variable('xalgospp_dep')
    else
        fs = import('fs')
        xalg_link_args = []
        foreach lib : xalg_libs_raw
            libdir = fs.parent(lib)
            libname = fs.name(lib)
            xalg_link_args += [
                '-L' + libdir,
                '-l:' + libname,
            ]
        endforeach

        xalg_inc = run_command(
            py3, '-c', 'import xalgospp; print(xalgospp.get_include())',
            check: false,
        ).stdout().strip()

        xalgospp_dep = declare_dependency(
            link_args: xalg_link_args,
            include_directories: include_directories(xalg_inc),
        )
    endif # if sbio_libs_raw empty (Python search for XAlgosPP)
endif
deps_for_sbio += xalgospp_dep

## TODO: See if possible to make XAlgosPP headers not need transitive includes.
##       This may not be feasible - and with the wheel-based provisions, the
##       pure dependency mechanism from the meson object would not suffice,
##       so, we may require these vendored subprojects long term.
################################################################################
cpp_httplib_dep = dependency('cpp-httplib', required: false)
if not cpp_httplib_dep.found()
    message('cpp-httplib not found - using subproject')
    # cpp-httplib deps:
    brotli_proj = subproject('google-brotli')
    zlib_proj = subproject('zlib')

    # Now build
    cpp_httplib_proj = subproject('cpp-httplib')
    cpp_httplib_dep = cpp_httplib_proj.get_variable('cpp_httplib_dep')
endif
deps_for_sbio += cpp_httplib_dep

eigen_dep = dependency('eigen', required: false)
if not eigen_dep.found()
    message('eigen not found - using subproject')
    eigen_proj = subproject('eigen')
    eigen_dep = dependency('eigen3')
endif
deps_for_sbio += eigen_dep

rapidjson_dep = dependency('rapidjson', required: false)
if not rapidjson_dep.found()
    message('rapidjson not found - using subproject')
    rapidjson_proj = subproject('rapidjson')
    rapidjson_dep = rapidjson_proj.get_variable('rapidjson_dep')
endif
deps_for_sbio += rapidjson_dep

spdlog_dep = dependency('spdlog', required: false)
if not spdlog_dep.found()
    message('spdlog not found - using subproject')
    spdlog_proj = subproject('spdlog')
    spdlog_dep = spdlog_proj.get_variable('spdlog_dep')
endif
deps_for_sbio += spdlog_dep
################################################################################

#### Path Setup ####
####################

py_install_dir = py3.get_install_dir(pure: false)

pysbio_install_dir = py_install_dir / 'sbio/core'

if sbio_as_wheel_opt
    # Install C++ libraries inside the wheel structure
    # Package and library setup will be:
    # site-packages/
    #    | - sbio
    #           | - lib
    #           |    | - libsbio.so
    #           |
    #           | - core
    #           |    | - _pysbio.so
    #           |
    #           | - __init__.py (utility functions and expose core lib)

    libsbio_install_dir = py_install_dir / 'sbio/lib'
    header_install_dir = py_install_dir / 'sbio/include'

    # Relative path from 'sbio/core' to 'sbio/lib'
    py_to_lib_path = '../lib'

    exe_prefix = py_install_dir
    lib_prefix = pysbio_install_dir
else
    # Normal install path
    libsbio_install_dir = get_option('libdir')
    header_install_dir = get_option('includedir')

    # Relative path from '$PREFIX/lib/pythonMAJ.MIN/site-packages/sbio/core' to '$PREFIX/lib'
    py_to_lib_path = '../../../../'

    exe_prefix = get_option('prefix') / 'bin'
    lib_prefix = pysbio_install_dir
endif

# For both the wheel and non-wheel case, executables relative path, for
# $PREFIX/bin to $PREFIX/lib is
exe_to_libsbio = '..' / libsbio_install_dir

lib_install_rpath = '$ORIGIN'

abs_exe_rpath = exe_prefix / exe_to_libsbio
abs_lib_rpath = lib_prefix / py_to_lib_path

# Setup RPATH for various platforms, for any executables that may need it
if host_machine.system() == 'darwin'
    exe_install_rpath = '@executable_path' + ':' + ('@executable_path' / exe_to_libsbio) + ':' + abs_exe_rpath
    py_to_lib_install_rpath = '@loader_path' + ':' + ('@loader_path' / py_to_lib_path)  + ':' + abs_lib_rpath
else
    exe_install_rpath = '$ORIGIN' + ':' + ('$ORIGIN' / exe_to_libsbio)  + ':' + abs_exe_rpath
    py_to_lib_install_rpath = '$ORIGIN' + ':' + ('$ORIGIN' / py_to_lib_path) + ':' + abs_lib_rpath
endif

#### SBIO ####
###############

installed_headers = []

installed_core_headers = [
    'src/lib/sbio/core/access_style.hh',
    'src/lib/sbio/core/broker_group.hh',
    'src/lib/sbio/core/broker.hh',
    'src/lib/sbio/core/datasource.hh',
    'src/lib/sbio/core/execution.hh',
    'src/lib/sbio/core/io.hh',
    'src/lib/sbio/core/pipeline.hh',
    'src/lib/sbio/core/storage.hh',
    'src/lib/sbio/core/stream.hh',
    'src/lib/sbio/core/sync.hh',
    'src/lib/sbio/core/utility.hh',
]

# installed_cuda_headers = [
#     'src/lib/sbio/cuda/execution.hh',
#     'src/lib/sbio/cuda/storage.hh',
# ]

installed_formats_headers = [
    'src/lib/sbio/formats/format_traits.hh',
]

installed_formats_xtc1_headers = [
    'src/lib/sbio/formats/xtc1/registry.hh',
    'src/lib/sbio/formats/xtc1/traversal.hh',
    'src/lib/sbio/formats/xtc1/xtc1.hh',
    'src/lib/sbio/formats/xtc1/xtc1_schemas.hh',
    'src/lib/sbio/formats/xtc1/xtc1_traits.hh',
]

installed_formats_xtc2_headers = [
    'src/lib/sbio/formats/xtc2/traversal.hh',
    'src/lib/sbio/formats/xtc2/xtc2.hh',
    'src/lib/sbio/formats/xtc2/xtc2_traits.hh',
]

installed_mpi_headers = [
    'src/lib/sbio/mpi/execution.hh',
    'src/lib/sbio/mpi/storage.hh',
]

installed_py_bindings_headers = [
    'src/lib/pysbio/pydatasource.hh',
    'src/lib/pysbio/pydetector.hh',
    'src/lib/pysbio/utilities.hh',
]

#### Core Library and Definitions

# Headers are always fully qualified using `sbio/core/...` `sbio/formats/...`
# We therfore set the include path to the top of the lib sub-directory
sbio_headers = include_directories('src/lib')

if build_core_opt
    core_sources = ['src/lib/sbio/posix_io.cc']

    if sbio_build_xtc1
        libxtc1slim = shared_library(
            'xtc1slim',
            [
                'src/lib/sbio/formats/xtc1/xtc1.cc',
                'src/lib/sbio/formats/xtc1/traversal.cc',
            ],
            include_directories: sbio_headers,
            install: true,
            install_dir: libsbio_install_dir,
            install_rpath: lib_install_rpath,
            version: '1',
            soversion: '1',
        )

        xtc1slim_dep = declare_dependency(
            link_with: [
                libxtc1slim,
            ],
            include_directories: sbio_headers,
        )
        deps_for_sbio += xtc1slim_dep

        core_sources += ['src/lib/sbio/formats/xtc1/xtc1_traits.cc']
    endif

    if sbio_build_xtc2
        libxtc2slim = shared_library(
            'xtc2slim',
            [
                'src/lib/sbio/formats/xtc2/xtc2.cc',
                'src/lib/sbio/formats/xtc2/traversal.cc',
            ],
            include_directories: sbio_headers,
            install: true,
            install_dir: libsbio_install_dir,
            install_rpath: lib_install_rpath,
            version: '1',
            soversion: '1',
        )

        xtc2slim_dep = declare_dependency(
            link_with: [
                libxtc2slim,
            ],
            include_directories: sbio_headers,
        )
        deps_for_sbio += xtc2slim_dep

        core_sources += ['src/lib/sbio/formats/xtc2/xtc2_traits.cc']
    endif

    libsbio = shared_library(
        'sbio',
        core_sources,
        dependencies: deps_for_sbio,
        include_directories: sbio_headers,
        install: true,
        install_dir: libsbio_install_dir,
        install_rpath: lib_install_rpath,
        version: '1',
        soversion: '1',
    )

    sbio_dep = declare_dependency(
        link_with: [
            libsbio,
        ],
        dependencies: deps_for_sbio,
        include_directories: sbio_headers,
    )

    if not sbio_as_wheel_opt
        install_headers(
            installed_headers,
            install_dir: header_install_dir / 'sbio',
        )
        install_headers(
            installed_core_headers,
            install_dir: header_install_dir / 'sbio/core',
        )
        # install_headers(
        #     installed_cuda_headers,
        #     install_dir: header_install_dir / 'sbio/cuda',
        # )
        install_headers(
            installed_formats_headers,
            install_dir: header_install_dir / 'sbio/formats',
        )
        # install_headers(
        #     installed_formats_xtc1_headers,
        #     install_dir: header_install_dir / 'sbio/formats/xtc1',
        # )
        install_headers(
            installed_formats_xtc2_headers,
            install_dir: header_install_dir / 'sbio/formats/xtc2',
        )
        install_headers(
            installed_mpi_headers,
            install_dir: header_install_dir / 'sbio/mpi',
        )

        # Python bindings headers - we'll always provide headers even without bindings
        install_headers(
            installed_py_bindings_headers,
            install_dir: header_install_dir / 'pysbio',
        )
    endif # not nca_as_wheel_opt
else
    libsbio = cpp.find_library('sbio', required: true)
    found_libs = [libsbio]
    if sbio_build_xtc1
        libxtc1slim = cpp.find_library('xtc1slim', required: true)
        found_libs += [libxtc1slim]
    endif
    if sbio_build_xtc2
        libxtc2slim = cpp.find_library('xtc2slim', required: true)
        found_libs += [libxtc2slim]
    endif
    sbio_dep = declare_dependency(
        dependencies: deps_for_sbio + found_libs,
        include_directories: sbio_headers,
    )
endif # end build_core_opt

#### Python module ####
#######################

if build_python_opt
    # Figure out dependencies
    deps_for_py_mod = deps_for_sbio + [pybind11_dep, py3_dep, sbio_dep]

    # If building bindings as a wheel, install the headers into the package itself
    # If not building as a wheel, then these should be installed in prefix/include when
    # building the core libraries
    if sbio_as_wheel_opt
        py3.install_sources(
            installed_headers,
            pure: false,
            subdir: 'sbio/include/sbio',
        )
        py3.install_sources(
            installed_core_headers,
            pure: false,
            subdir: 'sbio/include/sbio/core',
        )
        # py3.install_sources(
        #     installed_cuda_headers,
        #     pure: false,
        #     subdir: 'sbio/include/sbio/cuda',
        # )
        py3.install_sources(
            installed_formats_headers,
            pure: false,
            subdir: 'sbio/include/sbio/formats',
        )
        # py3.install_sources(
        #     installed_formats_xtc1_headers,
        #     pure: false,
        #     subdir: 'sbio/include/sbio/formats/xtc1',
        # )
        py3.install_sources(
            installed_formats_xtc2_headers,
            pure: false,
            subdir: 'sbio/include/sbio/formats/xtc2',
        )
        py3.install_sources(
            installed_mpi_headers,
            pure: false,
            subdir: 'sbio/include/sbio/mpi',
        )

        # Also add the headers for the bindings
        py3.install_sources(
            installed_py_bindings_headers,
            pure: false,
            subdir: 'sbio/include/pysbio',
        )

        # if bundle_mpi
        #  ... do something to relink mpirun ....
        # endif # bundle_mpi

    endif # sbio_as_wheel_opt

    py_mod = py3.extension_module(
        '_pysbio',
        [
            'src/lib/pysbio/pydatasource.cc',
            'src/lib/pysbio/pydetector.cc',
            'src/lib/pysbio/pysbio.cc',
            'src/lib/pysbio/utilities.cc',
        ],
        install: true,
        install_dir: pysbio_install_dir,
        include_directories: sbio_headers,
        dependencies : deps_for_py_mod,
        install_rpath: py_to_lib_install_rpath,
        build_rpath: py_to_lib_install_rpath,
        # link_args: py_mod_link_args,
    )
endif # build_python_opt
