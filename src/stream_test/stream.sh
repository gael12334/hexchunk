#
# Copyright (c) 2026 Gaël Fortier <gael.fortier.1@ens.etsmtl.ca>
#

gcc stream.c ../stream.c -o stream.elf -ggdb
if [ $? -eq 0 ]; then
  chmod +x stream.elf
  ./stream.elf
fi
