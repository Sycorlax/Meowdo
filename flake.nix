{
    description = "Meowdo flake";

    inputs = {
        # Note for the unfamiliar: The fact that this is using
        # unstable won't matter much, as it's more likely that
        # the flake.lock file remains mostly if not completely
        # unchanged over a long period of time, only needing
        # the occasional update.
        nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    };

    outputs = { self, nixpkgs }:

    let
        system = "x86_64-linux";
        pkgs = nixpkgs.legacyPackages.${system};
    in {
        packages.${system}.default = pkgs.stdenv.mkDerivation {
            pname = "meowdo";
            version = "dev-4"; # Development, revision (version line change) 4 of the flake
            src = self;

            nativeBuildInputs = [ pkgs.gcc pkgs.gnumake ];
            buildInputs = [ pkgs.ncurses ];

            buildPhase = ''
                make meowdo
            '';

            installPhase = ''
                mkdir -p $out/bin
                cp meowdo $out/bin/.
            '';
        };

        # optional devshell
        devShells.${system}.default = pkgs.mkShell {
            packages = [
                pkgs.gcc
                pkgs.ncurses
                pkgs.gnumake
            ];
        };
    };
}
