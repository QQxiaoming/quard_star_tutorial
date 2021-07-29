workspace(name = "com_google_quic_trace")

http_archive(
    name = "com_google_protobuf",
    sha256 = "1b666f3990f66890ade9faa77d134f0369c19d3721dc8111643b56685a717319",
    strip_prefix = "protobuf-3.5.1",
    urls = ["https://github.com/google/protobuf/releases/download/v3.5.1/protobuf-all-3.5.1.zip"],
)

http_archive(
    name = "com_github_gflags_gflags",
    sha256 = "4e44b69e709c826734dbbbd5208f61888a2faf63f239d73d8ba0011b2dccc97a",
    strip_prefix = "gflags-2.2.1",
    urls = ["https://github.com/gflags/gflags/archive/v2.2.1.zip"],
)

bind(
    name = "gflags",
    actual = "@com_github_gflags_gflags//:gflags",
)
