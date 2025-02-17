workspace(name = "topomesh")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# hmm
http_archive(
    name = "hmm",
    build_file = "@topomesh//third_party:BUILD.hmm",
    sha256 = "00792432998030f5c7cfb5ea0f33e41b02a69ba16c716165879fa681520b94b4",
    strip_prefix = "hmm-5ea611461e9f8b40f3d97e9f10fcf0f40592030b",
    urls = ["https://github.com/fogleman/hmm/archive/5ea611461e9f8b40f3d97e9f10fcf0f40592030b.zip"],
)
