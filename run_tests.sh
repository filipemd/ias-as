#!/bin/sh

if [ "$1" = "windows" ]; then
    IAS_AS="wine build/ias-as.exe"
else
    IAS_AS="build/ias-as"
fi

for asm_source in tests/*.asm
do
    if [ -f "$asm_source" ]; then
        tmp_result="${asm_source%.asm}.hex.tmp"
        expected_result="${asm_source%.asm}.hex"

        eval "$IAS_AS \"$asm_source\" \"$tmp_result\""
        if [ ! -e "$tmp_result" ]; then
            echo "An error occurred when assembling $asm_source."
            exit 1
        fi

        if cmp -s "$expected_result" "$tmp_result"; then
            echo "Test $asm_source passed."
        else
            echo "Test $asm_source did not pass."
            echo "Difference between files:"
            diff "$expected_result" "$tmp_result"
        fi

        rm "$tmp_result"
    fi
done
