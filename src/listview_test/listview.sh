#
# Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
#

files=("listview.c" "../listview.c" "../error.c")
output="listview.elf"

gcc ${files[@]} -o $output -ggdb
if [ $? -eq 0 ]; then
  chmod +x $output

  if [[ "$#" -gt 0 && "$1" == "run" ]]; then
    "./${output}"
  fi
fi
