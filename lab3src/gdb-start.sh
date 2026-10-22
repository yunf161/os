qemu-system-i386 -drive file=mbr.bin,format=raw,index=0,if=floppy -s -S


sleep 1


gnome-terminal --tab --title="GDB" -- gdb -ex "target remote localhost:1234" \
    -ex "symbol-file mbr.bin" \
    -ex "set architecture i8086" \
    -ex "set printregisters 16" \
    -ex "b *0x7c00" \
    -ex "c

    add-symbol-file mbr.symbol 0x7c00