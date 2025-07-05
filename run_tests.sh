#/bin/sh

for asm_source in tests/*.asm
do
    # Vê se arquivo existe
    if [ -f "$asm_source" ]; then
        tmp_result="${asm_source%.asm}.hex.tmp"
        expected_result="${asm_source%.asm}.hex"

        build/ias-as $asm_source $tmp_result
        if [ ! -e "$tmp_result" ]; then
            echo "An error occoured when assembling $asm_source."
            exit 1
        fi

        if cmp -s "$expected_result" "$tmp_result"; then
            echo "Test $asm_source passed."
        else
            echo "Test $asm_source did not passed."
            echo "Difference beetwen files: "
            diff "$expected_result" "$tmp_result"
        fi

        rm $tmp_result
    fi
done