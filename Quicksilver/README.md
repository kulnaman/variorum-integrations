# VARIORUM and Quicksilver Integration

## Introduction

This project demonstrates how Variorum can be used as a power and energy instrumentation tool through source-code annotation APIs. These APIs serve as wrappers for Variorum calls, offering a straightforward interface that automatically logs data and writes it to a file for each MPI node.

The project was compiled and tested on the Lassen Systems using Variorum version 0.8 and Quicksilver's commit [eb68bb8](https://github.com/LLNL/Quicksilver/commit/eb68bb8d6fc53de1f65011d4e79ff2ed0dd60f3b).

## Building

### Requirements:
* Variorum (required)
* CUDA (optional)
* MPI (optional)
* Rankstr (optional): To gather data across all MPI nodes.
* Jansson (required): For parsing Variorum results.

Modifications have been made to Quicksilver's Makefile (LLNL CORAL EA nodes section) to include all relevant flags.

## API

Currently, there are two APIs:
* `VARIORUM_ANNOTATE_GET_POWER_INFO`
* `VARIORUM_ANNOTATE_GET_ENERGY_INFO`

These APIs capture and record the calling function's file name, function name, and line number along with power or energy data. The results are saved to CSV files named `{node_hostname}_power_data.csv` or `{node_hostname}_energy_data.csv`.

Quicksilver 
-----------

Introduction
------------

Quicksilver is a proxy application that represents some elements of
the Mercury workload by solving a simpliﬁed dynamic monte carlo
particle transport problem.  Quicksilver attempts to replicate the
memory access patterns, communication patterns, and the branching or
divergence of Mercury for problems using multigroup cross sections.
OpenMP and MPI are used for parallelization.  A GPU version is
available.  Unified memory is assumed.

Performance of Quicksilver is likely to be dominated by latency bound
table look-ups, a highly branchy/divergent code path, and poor
vectorization potential.

For more information, visit the
[LLNL co-design pages.](https://codesign.llnl.gov/quicksilver.php)


Building Quicksilver
--------------------

Instructions to build Quicksilver can be found in the
Makefile. Quicksilver is a relatively easy to build code with no
external dependencies (except MPI and OpenMP).  You should be able to
build Quicksilver on nearly any system by customizing the values of
only four variables in the Makefile:

* CXX The name of the C++ compiler (with path if necessary)
  Quicksilver uses C++11 features, so a C++11 compliant compiler
  should be used.

* CXXFLAGS Command line switches to pass to the C++ compiler when
  compiling objects *and* when linking the executable.

* CPPFLAGS Command line switches to pass to the compiler *only* when
  compiling objects

* LDFLAGS Command line switches to pass to the compiler *only*
  when linking the executable

Sample definitions for a number of common systems are provided.

Quicksilver recognizes a number of pre-processor macros that enable or
disable various code features such as MPI, OpenMP, etc.  These are
described in the Makefile.


Running Quicksilver
-------------------

Quicksilver’s behavior is controlled by a combination of command line
options and an input file.  All of the parameters that can be set on
the command line can also be set in the input file.  The input file
values will override the command line.  Run `$ qs –h` to see
documentation on the available command line switches.  Documentation
of the input file parameters is in preparation.

Quicksilver also has the property that the output of every run is a
valid input file.  Hence you can repeat any run for which you have the
output file by using that output as an input file.


License and Distribution Information
------------------------------------

Quicksilver is available [on github](https://github.com/LLNL/Quicksilver)


Quicksilver is open source software with a BSD license.  See
[LICENSE.md](https://github.com/LLNL/Quicksilver/blob/master/LICENSE.md)

This work was performed under the auspices of the U.S. Department of
Energy by Lawrence Livermore National Laboratory under Contract
DE-AC52-07NA27344.

LLNL-CODE-684037
