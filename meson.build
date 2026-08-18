project(
    'sbio',
    'c',
    'cpp',
    version: '0.1.4',
    meson_version: '>=1.10.0',
    default_options: [
        'c_std=c17',
        'cpp_std=c++23,c++latest',
        'cuda_std=c++20',
        'optimization=3',
        'pkgconfig.relocatable=true',
        'b_ndebug=if-release',
    ],
)

add_languages('c', 'cpp', native: true, required: true)
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

# Export a list of the enabled data formats for downstream projects to link properly
enabled_formats = []

if sbio_build_xtc1
    add_project_arguments('-DSBIO_HAS_XTC1', language: ['cpp', 'cuda'])
    message('Will build XTC1 data format support.')
    enabled_formats += ['xtc1']
endif
if sbio_build_xtc2
    add_project_arguments('-DSBIO_HAS_XTC2', language: ['cpp', 'cuda'])
    message('Will build XTC2 data format support.')
    enabled_formats += ['xtc2']
endif

sbio_enabled_formats = {
    'formats': enabled_formats,
}
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

#### CUDA setup
if sbio_has_cuda
    # CUDA driver and nvrtc are both for JIT
    cuda_dep = dependency('cuda', modules: ['cuda', 'cufile', 'cudart'])
    deps_for_sbio += cuda_dep
endif

#### ncarray
libncarray = cpp.find_library('ncarray', required: false)
if libncarray.found()
    ncarray_dep = libncarray
else
    message('ncarray not found via `find_library` - checking via Python pkg-config helper')
    nca_pkg_config_bin = find_program('nca-pkg-config', required: false)
    if nca_pkg_config_bin.found()
        nc_pc_path_cmd_res = run_command(
            nca_pkg_config_bin, '--pkg-config-path',
            check: false,
        )
        nc_pc_path = nc_pc_path_cmd_res.stdout().strip()
        if (nc_pc_path_cmd_res.returncode() == 0) and (nc_pc_path != '' and nc_pc_path != 'None')
            nca_inc_rel_pc = nc_pc_path + '/../include'
            nc_cflags = run_command(
                nca_pkg_config_bin, '--pkg-config-cflags',
                check: false,
            ).stdout().strip().replace('${includedir}', nca_inc_rel_pc)
            message('**** Using these NCArray compile args: ' + nc_cflags)

            nc_libs = run_command(
                nca_pkg_config_bin, '--pkg-config-libs',
                check: false,
            ).stdout().strip().replace('${pcfiledir}', nc_pc_path)
            message('**** Using these NCArray link args: ' + nc_libs)

            ncarray_dep = declare_dependency(
                compile_args: nc_cflags.split(),
                link_args: nc_libs.split(),
            )
        endif
    endif

    if not is_variable('ncarray_dep')
        message('ncarray not found via pkg-config helper - checking via Python lib and inc getters')
        nc_libs_cmd = '''
import glob
import os
from typing import List, Set

import ncarray

d: str = ncarray.get_lib_dir()
files: Set[str] = set(glob.glob(os.path.join(d, "*")))
libs: List[str] = [f for f in files if f.endswith((".so", ".dylib", ".dll", ".lib")) or ".so." in f]

if any(f.endswith(".lib") for f in libs):
    libs = [f for f in libs if not f.endswith(".dll")]
print(";".join(libs).replace("\r", ""))
'''

        nc_cmd_res = run_command(
            py3, '-c', nc_libs_cmd,
            check: false,
        )
        message('NCArray Python Lib Search RETURNCODE: ' + nc_cmd_res.returncode().to_string())
        message('NCArray Python Lib Search STDOUT: ' + nc_cmd_res.stdout())
        message('NCArray Python Lib Search STDERR: ' + nc_cmd_res.stderr())

        nc_libs_raw = nc_cmd_res.stdout().strip().split(';')

        if (nc_libs_raw.length() == 0) or (nc_libs_raw.length() == 1  and nc_libs_raw[0] == '')
            message('ncarray not found via Python - will use subproject.')
            ncarray_proj = subproject('ncarray')
            ncarray_dep = ncarray_proj.get_variable('ncarray_dep')
        else
            fs = import('fs')
            nc_link_args = []
            foreach lib : nc_libs_raw
                if lib != ''
                    if 'libpython' in lib or lib.endswith('/Python')
                        message('Skipping ' + lib + ' from ncarray list. Causes problem on macOS!')
                    else
                        nc_link_args += [lib]
                    endif # macOS Python wheel repair guard.....
                endif
            endforeach

            nc_inc_cmd_res = run_command(
                py3, '-c', 'import ncarray; print(ncarray.get_include())',
                check: false,
            )

            message('NCArray Python Include Search RETURNCODE: ' + nc_inc_cmd_res.returncode().to_string())
            message('NCArray Python Include Search STDOUT: ' + nc_inc_cmd_res.stdout())
            message('NCArray Python Include Search STDERR: ' + nc_inc_cmd_res.stderr())

            nc_inc = nc_inc_cmd_res.stdout().strip()

            ncarray_dep = declare_dependency(
                link_args: nc_link_args,
                include_directories: include_directories(nc_inc),
            )
        endif # if nca_libs_raw empty (Python search for ncarray)
    endif # nca-pkg-config helper
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
        if host_machine.system() == 'windows'
            mpi_dep = dependency('mpi', language: 'c', required: false)
        else
            mpi_dep = dependency('mpi', required: false)
        endif
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

if host_machine.system() == 'darwin'
    # macOS CI is the worst. Fails in all psosible configurations
    # BUILD ONLY WITH SUBPROJECT, STATIC ONLY, DONT ALLOW ANYTHING TO BE PULLED BY HOMEBREW
    fmt_proj = subproject('fmt', default_options: ['default_library=static'])
    fmt_dep = fmt_proj.get_variable('fmt_dep')
    deps_for_sbio += fmt_dep
    spdlog_proj = subproject('spdlog', default_options: ['default_library=static'])
    spdlog_dep = spdlog_proj.get_variable('spdlog_dep')
else
    spdlog_dep = dependency('spdlog', required: false, static: true)
    if not spdlog_dep.found()
        message('spdlog not found - using subproject')
        spdlog_proj = subproject('spdlog', default_options: ['default_library=static'])
        spdlog_dep = spdlog_proj.get_variable('spdlog_dep')
    endif
endif
deps_for_sbio += spdlog_dep

#### Path Setup ####
####################

py_install_dir = py3.get_install_dir(pure: false)

pysbio_install_dir = py_install_dir / 'sbio/core'
pysbio_fmts_install_dir = py_install_dir / 'sbio/formats'

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

# Headers are always fully qualified using `sbio/core/...` `sbio/formats/...`
# We therfore set the include path to the top of the lib sub-directory
sbio_headers = include_directories('src/lib')

# Traverse the main source first so the dependency objects are available
# for examples and tests
subdir('src')

# Build example programs
# subdir('examples')

# Build the test suite
# subdir('tests')

# Setup a pkgconfig file now after the build completed.
pkg = import('pkgconfig')

if build_core_opt and not sbio_as_wheel_opt
    pc_install_dir = get_option('libdir') / 'pkgconfig'
else
    pc_install_dir = py_install_dir / 'sbio/pkgconfig'
endif

pc_libs = [libsbio]

# if sbio_has_cuda
#     pc_libs += [libdevsbio]
# endif

if sbio_build_xtc1
    pc_libs += [libxtc1slim]
endif

if sbio_build_xtc2
    pc_libs += [libxtc2slim]
endif

feature_flags = []
if sbio_has_cuda
    feature_flags += ['-DSBIO_HAS_CUDA']
endif

if sbio_build_xtc1
    feature_flags += ['-DSBIO_HAS_XTC1']
endif

if sbio_build_xtc2
    feature_flags += ['-DSBIO_HAS_XTC2']
endif

if get_variable('found_mpi', false)
    feature_flags += ['-DSBIO_HAS_MPI']
endif

pc_headers = ['.', 'sbio']
pkg.generate(
    name: 'sbio',
    description: 'Stream Broker IO - Data Processing Framework for Streamed Data.',
    libraries: pc_libs,
    extra_cflags: feature_flags,
    subdirs: pc_headers,
    version: meson.project_version(),
    url: 'https://sbio.xfelpp.org',
    license: 'AGPL-3.0-only',
    install_dir: pc_install_dir,
)
