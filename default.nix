{ stdenv, lib, fetchFromGitHub, ncurses }:

stdenv.mkDerivation rec {
    pname = "meowdo";
    version = "1.0.0";

    src = fetchFromGitHub {
        owner = "Sycorlax";
        repo = "Meowdo";
        rev = "v${version}";
        sha256 = "sha256-xOdaK53KEAwjlzdYsagoZYetNRpK64+HfzvioI+PPA8=";
    };

    buildInputs = [ ncurses ];

    buildPhase = '' # The Makefile is a little weird, with the main task as 'meowdo'.
        runHook preBuild
        make meowdo
        runHook postBuild
    '';

    installPhase = ''
        runHook preInstall

        mkdir -p $out/bin
        cp meowdo $out/bin/

        runHook postInstall
    '';

    meta = with lib; {
        description = "a cute, keyboard-driven todo list with a cat sidekick.";
        longDescription = ''
            meowdo is a cute, keyboard-driven todo list with a cat sidekick. It lives inyour terminal, supports tags, search, pinning, and remembers everything in ~/.local/share/meowdo/todos.txt 
        ''
        homepage = https://github.com/Sycorlax/Meowdo/;
        license = licenses.gpl3;
    };
}
