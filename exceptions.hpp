#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP
#include <iostream>
#include <cstring>
#include <algorithm>
typedef unsigned int uint;

enum instructiontype
{
    ADD,
    SUB,
    SLL,
    SLT,
    SLTU,
    XOR,
    SRL,
    SRA,
    OR,
    AND,
    JALR,
    LB,
    LH,
    LW,
    LBU,
    LHU,
    ADDI,
    SLTI,
    SLTIU,
    XORI,
    ORI,
    ANDI,
    SLLI,
    SRLI,
    SRAI,
    SB,
    SH,
    SW,
    BEQ,
    BNE,
    BLT,
    BGE,
    BLTU,
    BGEU,
    LUI,
    AUIPC,
    JAL,
    NOP,
    NONE
};

enum Reservationtype
{
    N0, //NULL
    A0, //Add_Reservation
    A1,
    A2,
    A3,
    L0, //Load_Buffer
    L1,
    L2,
    L3,
    L4,
    R0, //Re-order_Buffer
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7
};

uint cHEX(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    else
        return c - 'A' + 10;
}

uint decodeHEX(char *s)
{
    uint re = 0;
    for (int i = 0; s[i] != '\0'; i++)
        re = (re << 4) + cHEX(s[i]);
    return re;
}

uint sext(uint x, int bl) //有符号位拓展,bl为数的二进制位数
{
    bl--;
    if ((x >> bl) & 1)
        x |= ((0xffffffff >> bl) << bl);
    return x;
}

struct Register
{
    Reservationtype Qi;
    uint num;

    Register()
    {
        Qi = N0;
        num = 0;
    }
};

struct ROBunit
{
    Reservationtype Qi;
    instructiontype Op;
    uint rs;//寄存器位置
    uint num;//所存数据
    uint resultimm;
    uint resultpc;
    bool pred;
    Reservationtype Qs;
    uint Vs;
    bool used;

    ROBunit()
    {
        Qi = N0, num = 0, Qs = N0;
        Vs = 0;
        rs = 0;
        used = false;
    }
};

struct ReservationUnit
{
    bool used;
    bool work;
    bool spare;
    instructiontype Op;
    uint Vj;
    uint Vk;
    Reservationtype Qj;
    Reservationtype Qk;

    ReservationUnit()
    {
        used = false;
        Vj = 0, Vk = 0;
        Qj = N0, Qk = N0;
    }
};

struct Adder
{
    bool is_empty;
    Reservationtype type;
    uint result;

    Adder() { is_empty = true; }
};

struct Mem_unit
{
    bool is_empty;
    Reservationtype type;
    uint result;

    Mem_unit() { is_empty = true; }
};

unsigned char memory[3000005];
uint nowpc;
Register reg[32];
bool suspend;

#endif