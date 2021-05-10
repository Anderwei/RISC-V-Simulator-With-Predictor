            addi x2,x0,0          // v=0
            addi x3,x0,16              // Loop bound for LoopI
            addi x4,x0,0               // i=0
LoopI:      beq x4,x3,EndLoopI    // Exit LoopI if i==16
            addi x5,x0,0               // j=0
LoopJ:      beq x5,x3,EndLoopJ    // Exit LoopJ if j==16
            add x6,x5,x4          // j+i
            andi x6,x6,3          // (j+i)%4
            bne x6,x0,Endif       // Skip if (j+i)%4!=0
            add x2,x2,x5          // v+=j
Endif:      addi x5,x5,1          // j++
            beq x0,x0,LoopJ       // Go back to LoopJ
EndLoopJ:   addi x4,x4,1          // i++
            beq x0,x0,LoopI       // Go back to LoopI
EndLoopI:   addi x0,x0,0