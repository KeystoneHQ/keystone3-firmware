{
  description = "Keystone3 firmware flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
    rust-overlay = {
      url = "github:oxalica/rust-overlay";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    {
      self,
      nixpkgs,
      rust-overlay,
    }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs {
        inherit system;
        overlays = [ rust-overlay.overlays.default ];
      };
      rustToolchain = pkgs.rust-bin.nightly."2025-07-01".default.override {
        targets = [ "thumbv7em-none-eabihf" ];
        extensions = [
          "rust-src"
          "rust-analyzer"
        ];
      };
      python = pkgs.python314.withPackages (ps: [
        ps.pyyaml
        ps.pillow
      ]);
      buildScript =
        name: args:
        pkgs.writeShellScriptBin name ''
          exec ${python}/bin/python3 build.py ${args}
        '';
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        buildInputs = [
          rustToolchain
          python
          (pkgs.writeShellScriptBin "rustup" ''
            if [ "$1" = "run" ]; then
              shift; shift; exec "$@"
            fi
            echo "rustup shimmed by nix devShell"
          '')
          pkgs.gcc-arm-embedded
          pkgs.cmake
          pkgs.gnumake
          pkgs.clang
          pkgs.pkg-config
          pkgs.libclang.lib
          pkgs.zlib
          (buildScript "build-multi" "")
          (buildScript "build-btc-only" "-t btc_only")
          (buildScript "build-cypherpunk" "-t cypherpunk")
          (buildScript "build-multi-production" "-e production")
          (buildScript "build-btc-only-production" "-e production -t btc_only")
          (buildScript "build-cypherpunk-production" "-e production -t cypherpunk")
          (pkgs.writeShellScriptBin "build-firmware-maker" ''
            cd tools/code/firmware-maker
            cargo build --release
          '')
          (pkgs.writeShellScriptBin "make-keystone3-bin" ''
            set -e
            fmm="tools/code/firmware-maker/target/release/fmm"
            if [ ! -f "$fmm" ]; then
              echo "firmware-maker not built yet, run: build-firmware-maker"
              exit 1
            fi
            if [ ! -f "build/mh1903_full.bin" ]; then
              echo "build/mh1903_full.bin not found, run a production build first"
              exit 1
            fi
            "$fmm" --source build/mh1903_full.bin --destination build/keystone3.bin
            echo "Created build/keystone3.bin"
          '')
          (pkgs.writeShellScriptBin "verify-firmware" ''
            set -e
            fmc_dir="tools/code/firmware-checker"
            if [ ! -f "$fmc_dir/target/release/fmc" ]; then
              echo "Building firmware-checker..."
              cd "$fmc_dir"
              cargo build --release
              cd -
            fi
            "$fmc_dir/target/release/fmc" --source build/keystone3.bin
          '')
        ];

        shellHook = ''
          export LIBCLANG_PATH="${pkgs.libclang.lib}/lib"
          export LD_LIBRARY_PATH="${pkgs.zlib}/lib''${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
          export CARGO_NET_GIT_FETCH_WITH_CLI=true
          export RUSTUP_TOOLCHAIN=nightly-2025-07-01
        '';
      };
    };
}
