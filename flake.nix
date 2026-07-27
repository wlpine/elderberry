{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
    scenefx = {
      url = "github:wlrfx/scenefx";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs = {
    self,
    flake-parts,
    ...
  } @ inputs:
    flake-parts.lib.mkFlake {inherit inputs;} {
      imports = [
        inputs.flake-parts.flakeModules.easyOverlay
      ];

      flake = {
        hmModules.elderberry = import ./nix/hm-modules.nix self;
        nixosModules.elderberry = import ./nix/nixos-modules.nix self;
      };

      perSystem = {
        config,
        pkgs,
        ...
      }: let
        inherit (pkgs) callPackage;
        elderberry = callPackage ./nix {
          scenefx = inputs.scenefx.packages.${pkgs.stdenv.hostPlatform.system}.default;
        };
        shellOverride = old: {
          nativeBuildInputs = old.nativeBuildInputs ++ [];
          buildInputs = old.buildInputs ++ [];
        };
      in {
        packages.default = elderberry;
        overlayAttrs = {
          inherit (config.packages) elderberry;
        };
        packages = {
          inherit elderberry;
          hm-options-json = pkgs.callPackage (import ./nix/generate-options.nix self) {
            module = ./nix/hm-modules.nix;
            optionPrefix = "wayland.windowManager.elderberry.";
          };
          nixos-options-json = pkgs.callPackage (import ./nix/generate-options.nix self) {
            module = ./nix/nixos-modules.nix;
            optionPrefix = "programs.elderberry.";
          };
        };
        devShells.default = elderberry.overrideAttrs shellOverride;
        formatter = pkgs.alejandra;
      };
      systems = [
        "x86_64-linux"
        "aarch64-linux"
      ];
    };
}
