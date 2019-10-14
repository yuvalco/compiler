.entry LIST
.extern W
.define sz = 2x
MAIN: mov r3, LIST[sz1]
LOOP: jmp
 prn #-5
 mov STR[5], STR[2]
 sub r1, #a

 cmp K, #s
 bne W
L1: inc L3
.entry LOOP
 bne LOOP
END: stop
.define len = 4
STR: .string "abcdef"
LIST: .data 6, -9Z, len
K: .data 22x
.extern L3
