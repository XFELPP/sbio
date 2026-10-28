# sbio
Stream Broker IO - Data Processing Framework for Streamed Data.

`sbio` provides a framework for interacting with data of a variety of formats, provided it can be described by a simple model: if there's a reasonable answer to the question "what is the next unit of data", then `sbio` can work with it.

The framework tackles the issue of reading data by providing a number of composable pieces. For developers and implementers, this gives very precise control over synchronization, memory access and state management. For end-users, the result is a simple system where the code looks the same whether running one process on CPU, hybrid MPI and thread-based parallel processing, or even from entirely within a CUDA kernel.

## Who is this for?

`sbio` is intended primarily for use as a library, rather than a complete end-user application. That said, it has many features that facilitate use in exploratory research/data science. I.e., it does, in many ways, come batteries included. However, especially due to its generic nature, it may not have all the targetted niceties and affordances a final application would. For that reason, it is a recommended component for IO-needs, but not the entire soluution. See also, the `XAlgosPP` project - in addition to complementing `sbio` nicely, it demonstrates how it can be incorporated into larger projects. The two projects together handle IO, and coordination of data analysis algorithms.

## Features

`sbio` provides:
- A modular core architecture to easily adapt to varied use cases.
- A high-performance C++ backend, along with approachable Python bindings. The latter are designed for progressive disclosure in a tiered approach: start with the simpler and more abstracted, and progress to constructs closer to the underlying C++ as needed.
- Support for multiple architectures, operating systems, and Python versions.
- Self-contained synchronization and memory model. Regardless of the programming model being used, if it is a supported `sbio` backend, no manual synchronization, or memory copies, are required of application developers. `sbio` handles this internally, and will likewise, not interact with any synchronization operations used within your own code. E.g., no need to think of barriers in MPI, or worry about breaking early from a loop.

## Components Overview

`sbio` relies on composition to describe how it is that data should be read. This begins with selection of the data format, followed by the IO driver, and the execution policy. Execution policies are often further decomposed into distribution and memory managers which facilitates reuse and allows novel combinations.

Currently, the following `IOPolicy` drivers are provided:
- `SyncPOSIXIO` - For synchronous IO with POSIX APIs. A bit of a misnomer, this also provides equivalent behaviour on Windows.
- `cuFileIO` - For cuFile-based reads to facilitate direct-to-GPU strategies.
- While not released, a complete implementation of `io_uring` has also been tested. Release is waiting on a refactor which will occur after the completion of the asynchronous core APIs which make it truly advantageous. Interested parties can ask, however, if earlier access would be useful.

`FormatTraits` descriptions are currently provided for the following data formats:
- `XTC1`
- `XTC2`
- `HDF5` driver layer. HDF5 is special in that it isn't "integrated" in its entirety. `sbio` deals with data formats, not file types, and HDF5, in the general sense, is too broad. The driver layer is generic, and specifications for different formats on top of it must be provided. This currently includes:
  - `CXI` files.
- `sbio randfmt` - Unlikely to be of general interest, there is also a custom data format used internally.

The above formats and IO drivers can be combined with the following distribution managers:
- `Serial` - A single process serial distribution manager.
- `Threaded` - A multi-thread distribution manager.
- `MPIThreaded` - A hybrid manager for using multiple MPI ranks each managing multiple threads.
- `MPI` - An MPI rank-based distribution manager.

The distribution managers can be paired with the two available memory managers to work with CPU or GPU memory:
- `HostMemory`
- `CudaMemory`

Various combinations of these components should provide coverage of many possible use cases. For any of the given data formats, there are likely 8-12 strategies that can be explored based on available resources and the structure of existing software to be interacted with.

## Project State and Roadmap

`sbio` is still in alpha state, although the feature set is rapidly approaching a reasonable level of maturity to transition towards a beta phase. More specifically, in the context of the XFELPP projects, this means that major interface changes are no longer expected, and true SemVer guarantees/meanings can be applied to every tagged release.

There are still a few major outstanding items to complete:
- The corresponding write APIs for the existing read ones.
- Asynchronous IO model.

These will be approached in coming releases and updates.
