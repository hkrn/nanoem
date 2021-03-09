/*
  Copyright (c) 2015-2020 hkrn All rights reserved

  This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
*/

extern crate cc;

fn main() {
    let mut build = cc::Build::new();
    let base = &format!(
        "{}/../../nanoem",
        std::env::var("CARGO_MANIFEST_DIR").unwrap()
    );
    build
        .file(format!("{}/nanoem.c", base))
        .file(format!("{}/ext/mutable.c", base))
        .file(format!("{}/ext/motion.c", base))
        .file(format!("{}/ext/motion.pb-c.c", base))
        .file("../../dependencies/protobuf-c/protobuf-c/protobuf-c.c")
        .include(base);
    if cfg!(target_os = "windows") {
        build.file(format!("{}/ext/mbwc.c", base));
    } else if cfg!(target_os = "macos") {
        println!("cargo:rustc-link-lib=framework=CoreFoundation");
        build.file(format!("{}/ext/cfstring.c", base));
        build.compiler("/usr/bin/clang");
    }
    build.compile("nanoem");
}
