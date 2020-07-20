#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "RISC-V_Tomasulo.hpp"
using namespace std;

RISCV_Tomasulo riscv;

int main()
{
    riscv.input();
    riscv.run();
    cout<<riscv.output()<<endl;
    return 0;
}