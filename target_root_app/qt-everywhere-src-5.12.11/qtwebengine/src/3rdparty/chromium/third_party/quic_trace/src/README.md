# QUIC trace utilities

This repository contains a format for recording a trace of QUIC connection,
together with the tools for analyzing the resulting traces.  This format of
traces is currently partially supported by Chromium, and we hope for more
implementations to adopt it in the future.

The primary focus of this format is debugging congestion-control related
issues, but other transport aspects (e.g. flow control) and transport-related
aspects of the application (e.g. annotations for the types of data carried by
the streams) are also within the scope.

## How to record this format

The traces are represented as a Protocol Buffer, which is completely described
in `lib/quic_trace.proto`.  Projects that use Bazel can embed this repository
directly and use the provided Bazel rules.

## How to render the resulting trace

Currently, we only support a simple visualization that uses gnuplot.

1. Build the helper tool by running `bazel build
   //tools/quic_trace_to_time_sequence_gnuplot`.
1. Generate the graph by running `tools/time_sequence_gnuplot.sh trace_file
   trace.png`.
