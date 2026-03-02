#
# Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
#

files=("hex.c" "../hex.c" "../stream.c" "../app.c" "../path.c")
output="hex.elf"

gcc ${files[@]} -o $output -ggdb
if [ $? -eq 0 ]; then
  chmod +x $output

  if [[ "$#" -gt 0 && "$1" == "run" ]]; then
    "./${output}"
  fi
fi
