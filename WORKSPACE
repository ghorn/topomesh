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

# hmstl
http_archive(
    name = "hmstl",
    build_file = "@topomesh//third_party:BUILD.hmstl",
    sha256 = "d7632bc6247a2d4d00fce0d88d1a0c03ef63bf3aee0ab545ad125d73a686bc08",
    strip_prefix = "hmstl-74c678e60393d98f8e2f70dc30546ebb6ce68985",
    urls = ["https://github.com/ghorn/hmstl/archive/74c678e60393d98f8e2f70dc30546ebb6ce68985.zip"],
)

# hmstl dependency libtrix
http_archive(
    name = "libtrix",
    build_file = "@topomesh//third_party:BUILD.libtrix",
    sha256 = "9da9d8fc79339eaa7387d8c2c3457889ca259a74eac30193e668a0d4b5f836d7",
    strip_prefix = "libtrix-fb23d3df2c4c607a28fbefbbda21fa43b1522b3f",
    urls = ["https://github.com/anoved/libtrix/archive/fb23d3df2c4c607a28fbefbbda21fa43b1522b3f.zip"],
)
