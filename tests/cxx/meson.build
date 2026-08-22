#### Testing ####
#################
if build_core_opt and build_tests_opt
    gtest_dep = dependency('gtest', main: true, static: true, required: false)

    if not gtest_dep.found()
        gtest_proj = subproject('gtest', default_options: ['default_library=static'])
        gtest_dep = gtest_proj.get_variable('gtest_main_dep')
    endif

    deps_for_tests = [sbio_dep, sbiofmt_dep, ncarray_dep, gtest_dep]
    # if sbio_has_cuda
    #     deps_for_tests += cuda_dep
    # endif

    sbiofmt_test_sources = [
        'test_random_patterns.cc',
        'test_indexing_modes.cc',
        'test_broker_group.cc',
        'test_execution_policies.cc',
    ]
    test_sbiofmt = executable(
        'test_sbiofmt',
        sbiofmt_test_sources,
        dependencies: deps_for_tests,
        install: true,
        include_directories: sbio_headers,
    )
    test('Running test suite using sbio rndfmt.', test_sbiofmt, verbose: true)

endif # build_core_opt and build_tests_opt
