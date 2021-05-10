# RISC-V-Simulator-With-Predictor

Base on previous project [RISC-V Assembler](https://github.com/Anderwei/RISC-V-Assembler)  

## Diff
### Added Feature
* Branch prediction  
> ~~Althought it is a post-Predictor~~  
* Register Table
* Advanced Debug Feature
### Added Class
* RegisterTable  
-- for save / load register  
-- range from x0 ~ x31
* BranchPredictor  
-- Perform as 2-bit Branch History Table  

## Build

#### Envirement:
> C++ Standard:
> * stdc++14
>
> OS:
> * CentOS Linux release 8.3.2011  
>
> Library : 
> * libstdc++ == 8.3.1-5.1.el8.x86_64  
> * gcc-c++ == 8.3.1-5.1.el8.x86_64  

#### compile argument
> g++ -g main.cpp -std=c++14 -Wall -o main

## Feature

### Entry Setting
give the ability to set the entry amount of BHT
```
Please input entry(entry > 0):
10
```

### Advanced Debug Info

Press Enter for next instruction  
Input `!` for skipping to the end  
  
#### Debug Info:
```
[ Instruction Address ] [    Instruction    ]  
[  Entry  ][   Selector / 2-bit counter   ]  
[  guess  ][  result  ]  
----------  
[ BHT Table ]
  
[ Register Table]
```
```
0x0000              addi x2,x0,0          // v=0
entry: 1   (00, SN, SN, SN, SN)
guess : N  result: N
----------
BHT After Update:
Entry 0 : (00, SN, SN, SN, SN)
Entry 1 : (00, SN, SN, SN, SN)
Entry 2 : (00, SN, SN, SN, SN)
Entry 3 : (00, SN, SN, SN, SN)
Entry 4 : (00, SN, SN, SN, SN)
Entry 5 : (00, SN, SN, SN, SN)
Entry 6 : (00, SN, SN, SN, SN)
Entry 7 : (00, SN, SN, SN, SN)
Entry 8 : (00, SN, SN, SN, SN)
Entry 9 : (00, SN, SN, SN, SN)

  register  |          int           |         hex        |
     x0     |  0                     |  0000000000000000  |
     x1     |  0                     |  0000000000000000  |
     x2     |  0                     |  0000000000000000  |
     x3     |  0                     |  0000000000000000  |
     x4     |  0                     |  0000000000000000  |
     x5     |  0                     |  0000000000000000  |
     x6     |  0                     |  0000000000000000  |
```

