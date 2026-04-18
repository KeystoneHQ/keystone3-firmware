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
        extensions = [ "rust-src" "rust-analyzer" ];
      };
      python = pkgs.python314.withPackages (ps: [
        ps.pyyaml
        ps.pillow
      ]);
      buildScript = name: args: pkgs.writeShellScriptBin name ''
        exec ${python}/bin/python3 build.py ${args}
      '';
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        buildInputs = [
          rustToolchain
          python
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
        ];

        shellHook = ''
          export LIBCLANG_PATH="${pkgs.libclang.lib}/lib"
          export LD_LIBRARY_PATH="${pkgs.zlib}/lib''${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
          export CARGO_NET_GIT_FETCH_WITH_CLI=true
        '';
      };
    };
}
