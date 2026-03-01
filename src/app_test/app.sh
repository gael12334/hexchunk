#
# Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
#

files=("app.c" "../app.c" "../stream.c")
output="app.elf"

gcc ${files[@]} -o $output -ggdb
if [ $? -eq 0 ]; then
  chmod +x $output

  if [[ "$#" -gt 0 && "$1" == "run" ]]; then
    "./${output}"
  fi
fi
