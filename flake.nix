{
  outputs =
    { nixpkgs, self }:
    let
      pkgs = import nixpkgs { system = "x86_64-linux"; };
    in
    {
      devShells.x86_64-linux.default = pkgs.mkShell {
        buildInputs = with pkgs; [
          meson
          ninja
          openssl
        ];
      };
    };
}
