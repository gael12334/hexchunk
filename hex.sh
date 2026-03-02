#
# Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
#

files=("src/hex.c" "src/stream.c" "src/app.c" "src/path.c" "src/main.c")
output="hex.elf"

gcc ${files[@]} -o $output -ggdb
if [ $? -eq 0 ]; then
  chmod +x $output

  if [[ "$#" -gt 0 && "$1" == "run" ]]; then
    "./${output}"
  fi
fi
