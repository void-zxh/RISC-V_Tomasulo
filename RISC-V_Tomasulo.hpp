#include "Instruction.hpp"
#include "exceptions.hpp"
#include "Add_Reservation.hpp"
#include "Load_Buffer.hpp"
#include "Re-order_Buffer.hpp"
#include "Prediction.hpp"
#ifndef RISCV_TOMASULO_HPP
#define RISCV_TOMASULO_HPP

class RISCV_Tomasulo
{
private:
    Add_Reservation AR;
    Load_Buffer LR;
    Adder AD;
    Mem_unit MEM;
    Reorder_Buffer rob;
    Prediction pdt;

    uint load(uint pos, int li)
    {
        uint re = 0;
        for (int i = li - 1; i >= 0; i--)
            re = (re << 8) + memory[pos + i];
        return re;
    }

    void store(uint pos, uint x, int p)
    {
        for (int i = 0; i < p; i++)
        {
            memory[pos + i] = x & 255u;
            x >>= 8;
        }
    }

    void getSRC_OTHERS(Reorder_Buffer &rob, ReservationUnit &rc, uint rs1, uint rs2)
    {
        if (rs1)
        {
            if (reg[rs1].Qi != N0)
            {
                if (rob.q[reg[rs1].Qi - R0].Qi != N0)
                    rc.Qj = rob.q[reg[rs1].Qi - R0].Qi;
                else
                    rc.Vj = rob.q[reg[rs1].Qi - R0].num;
            }
            else
                rc.Vj = reg[rs1].num;
        }

        if (rs2)
        {
            if (reg[rs2].Qi != N0)
            {
                if (rob.q[reg[rs2].Qi - R0].Qi != N0)
                    rc.Qk = rob.q[reg[rs2].Qi - R0].Qi;
                else
                    rc.Vk = rob.q[reg[rs2].Qi - R0].num;
            }
            else
            {
                rc.Vk = reg[rs2].num;
            }
        }
    }

    void getSRC_JS(Reorder_Buffer &rob, ROBunit &rc, uint rs1, uint rs2)
    {
        if (rs1)
        {
            if (reg[rs1].Qi != N0)
            {
                if (rob.q[reg[rs1].Qi - R0].Qi != N0)
                    rc.Qs = rob.q[reg[rs1].Qi - R0].Qi;
                else
                    rc.Vs = rob.q[reg[rs1].Qi - R0].num;
            }
            else
                rc.Vs = reg[rs1].num;
        }

        if (rs2)
        {
            if (reg[rs2].Qi != N0)
            {
                if (rob.q[reg[rs2].Qi - R0].Qi != N0)
                    rc.Qi = rob.q[reg[rs2].Qi - R0].Qi;
                else
                    rc.num = rob.q[reg[rs2].Qi - R0].num;
            }
            else
                rc.num = reg[rs2].num;
        }
    }

    void ISSUE()
    {
        Instruction inst;
        ReservationUnit ci;
        Reservationtype ty;
        ROBunit ri;
        uint com;
        if (suspend)
            return;
        com = load(nowpc, 4);
        inst.getDEC(com);
        ci.used = true;
        ci.work = false;
        ci.spare = false;
        ci.Op = inst.type;
        ri.used = true;
        ri.Op = inst.type;

        if (com == 0x0ff00513)
        {
            suspend = true;
            return;
        }

        switch (inst.type)
        {
        case LUI:
        case AUIPC:
        case JAL:
        case JALR:
        case SB:
        case SH:
        case SW:
            if (rob.full())
                return;
            break;

        case ADDI:
        case SLTI:
        case SLTIU:
        case XORI:
        case ORI:
        case ANDI:
        case SLLI:
        case SRLI:
        case SRAI:
        case ADD:
        case SUB:
        case SLL:
        case SLT:
        case SLTU:
        case XOR:
        case SRL:
        case SRA:
        case OR:
        case AND:
        case BEQ:
        case BNE:
        case BLT:
        case BGE:
        case BLTU:
        case BGEU:
            if (rob.full() || AR.full())
                return;
            break;

        case LB:
        case LH:
        case LW:
        case LBU:
        case LHU:
            if (rob.full() || LR.full())
                return;
            break;
        }

        switch (inst.type)
        {
        case LUI:
            ri.num = inst.imm;
            ri.rs = inst.rd;
            break;

        case AUIPC:
            ri.num = nowpc + inst.imm;
            ri.rs = inst.rd;
            break;

        case JAL:
            ri.num = nowpc + 4;
            ri.rs = inst.rd;
            break;

        case JALR:
            getSRC_JS(rob, ri, inst.rs1, 0);
            ri.Vs += inst.imm;
            ri.num = nowpc + 4;
            ri.rs = inst.rd;
            break;

        case SB:
        case SH:
        case SW:
            getSRC_JS(rob, ri, inst.rs1, inst.rs2);
            ri.Vs += inst.imm;
            break;

        case LB:
        case LH:
        case LW:
        case LBU:
        case LHU:
            getSRC_OTHERS(rob, ci, inst.rs1, 0);
            ci.Vj += inst.imm;
            ty = LR.push(ci);
            ri.Qi = ty;
            ri.rs = inst.rd;
            break;

        case ADDI:
        case SLTI:
        case SLTIU:
        case XORI:
        case ORI:
        case ANDI:
        case SLLI:
        case SRLI:
        case SRAI:
            getSRC_OTHERS(rob, ci, inst.rs1, 0);
            ci.Vk = inst.imm;
            ty = AR.push(ci);
            ri.Qi = ty;
            ri.rs = inst.rd;
            break;

        case ADD:
        case SUB:
        case SLL:
        case SLT:
        case SLTU:
        case XOR:
        case SRL:
        case SRA:
        case OR:
        case AND:
        case BEQ:
        case BNE:
        case BLT:
        case BGE:
        case BLTU:
        case BGEU:
            getSRC_OTHERS(rob, ci, inst.rs1, inst.rs2);
            ty = AR.push(ci);
            ri.Qi = ty;
            ri.rs = inst.rd;
            break;
        }

        if (inst.type >= 28 && inst.type <= 33) //B类指令
        {
            ri.resultpc = nowpc;
            ri.resultimm = inst.imm;
            if (pdt.getPrediction(nowpc, inst))
                nowpc += inst.imm, ri.pred = true;
            else
                nowpc += 4, ri.pred = false;
        }
        else if (inst.type == JAL)
            nowpc += inst.imm;
        else if (inst.type == JALR)
            suspend = true;
        else
            nowpc += 4;

        rob.set(ri);
    }

    void EX_ADD()
    {
        ReservationUnit ci;
        Reservationtype ty;
        if (!AR.idle() || !AD.is_empty)
            return;
        ci = AR.pop(ty);
        switch (ci.Op)
        {
        case ADD:
        case ADDI:
            AD.result = (ci.Vj + ci.Vk);
            break;
        case SUB:
            AD.result = (ci.Vj - ci.Vk);
            break;
        case SLL:
        case SLLI:
            AD.result = (ci.Vj << (ci.Vk & 31));
            break;
        case SLT:
        case SLTI:
            AD.result = (int(ci.Vj) < int(ci.Vk));
            break;
        case SLTU:
        case SLTIU:
            AD.result = (ci.Vj < ci.Vk);
            break;
        case XOR:
        case XORI:
            AD.result = (ci.Vj ^ ci.Vk);
            break;
        case SRL:
        case SRLI:
            AD.result = (ci.Vj >> (ci.Vk & 31));
            break;
        case SRA:
        case SRAI:
            AD.result = (int(ci.Vj) >> (ci.Vk & 31));
            break;
        case OR:
        case ORI:
            AD.result = (ci.Vj | ci.Vk);
            break;
        case AND:
        case ANDI:
            AD.result = (ci.Vj & ci.Vk);
            break;

        case BEQ:
            AD.result = (ci.Vj == ci.Vk);
            break;
        case BNE:
            AD.result = (ci.Vj != ci.Vk);
            break;
        case BLT:
            AD.result = (int(ci.Vj) < int(ci.Vk));
            break;
        case BGE:
            AD.result = (int(ci.Vj) >= int(ci.Vk));
            break;
        case BLTU:
            AD.result = (ci.Vj < ci.Vk);
            break;
        case BGEU:
            AD.result = (ci.Vj >= ci.Vk);
            break;
        }
        AD.type = ty;
        AD.is_empty = false;
    }

    void EX_LOAD()
    {
        ReservationUnit ci;
        Reservationtype ty;
        if (!LR.idle() || !MEM.is_empty)
            return;
        ci = LR.find(ty);
        while (!rob.check(ty, ci.Vj))
        {
            ci = LR.find_next(ty);
            if (ty == N0)
                return;
        }
        LR.pop(ty);
        switch (ci.Op)
        {
        case LB:
        case LBU:
            MEM.result = load(ci.Vj, 1);
            break;
        case LH:
        case LHU:
            MEM.result = load(ci.Vj, 2);
            break;
        case LW:
            MEM.result = load(ci.Vj, 4);
            break;
        }
        MEM.type = ty;
        MEM.is_empty = false;
    }

    void BROADCAST()
    {
        if (!AD.is_empty)
        {
            AR.update(AD.type, AD.result);
            LR.update(AD.type, AD.result);
            rob.update(AD.type, AD.result);
            AD.is_empty = true;
        }
        if (!MEM.is_empty)
        {
            MEM.is_empty = true;
            if (MEM.type != N0)
            {
                AR.update(MEM.type, MEM.result);
                LR.update(MEM.type, MEM.result);
                rob.update(MEM.type, MEM.result);
            }
        }
    }

    void COMMIT()
    {
        Reservationtype ty;
        ROBunit ci;
        if (rob.empty())
            return;
        ci = rob.front(ty);
        switch (ci.Op)
        {
        case ADD:
        case SUB:
        case SLL:
        case SLT:
        case SLTU:
        case XOR:
        case SRL:
        case SRA:
        case OR:
        case AND:
        case ADDI:
        case SLTI:
        case SLTIU:
        case XORI:
        case ORI:
        case ANDI:
        case SLLI:
        case SRLI:
        case SRAI:
        case LUI:
        case AUIPC:
        case JAL:
        case LB:
        case LH:
        case LW:
        case LBU:
        case LHU:
            if (ci.Qi)
                return;
            reg[ci.rs].num = ci.num;
            if (reg[ci.rs].Qi == ty)
                reg[ci.rs].Qi = N0;
            rob.pop();
            break;

        case SB:
        case SH:
        case SW:
            if (ci.Qi || ci.Qs)
                return;
            if (!MEM.is_empty)
                return;
            switch (ci.Op)
            {
            case SB:
                store(ci.Vs, ci.num, 1);
                break;

            case SH:
                store(ci.Vs, ci.num, 2);
                break;

            case SW:
                store(ci.Vs, ci.num, 4);
                break;
            }
            MEM.type = N0;
            MEM.is_empty = false;
            rob.pop();
            break;

        case JALR:
            if (ci.Qs)
                return;
            nowpc = ci.Vs;
            suspend = false;
            reg[ci.rs].num = ci.num;
            if (reg[ci.rs].Qi == ty)
                reg[ci.rs].Qi = N0;
            rob.pop();
            break;

        case BEQ:
        case BNE:
        case BLT:
        case BGE:
        case BLTU:
        case BGEU:
            if (ci.Qi)
                return;
            pdt.update(ci);
            if (bool(ci.num) != ci.pred)
            {
                rob.clear();
                AR.clear();
                LR.clear();
                if (MEM.type)
                    MEM.is_empty = true;
                AD.is_empty = true;
                suspend = false;

                for (int ii = 0; ii < 32; ii++) //清空Register
                    reg[ii].Qi = N0;

                if (ci.num)
                    nowpc = ci.resultpc + ci.resultimm;
                else
                    nowpc = ci.resultpc + 4;
            }
            else
                rob.pop();
        }
    }

public:
    void input()
    {
        char s[105];
        memset(s, 0, sizeof(s));
        int addr = 0;
        while (~scanf("%s", s))
        {
            if (s[0] == '@')
                addr = decodeHEX(s + 1);
            else
            {
                memory[addr] = uint(decodeHEX(s));
                addr++;
            }
        }
    }

    void run()
    {
        while (1)
        {
            ISSUE();
            EX_ADD();
            EX_LOAD();
            BROADCAST();
            COMMIT();
            if (rob.empty() && suspend)
                break;
        }
    }

    uint output()
    {
        pdt.out();
        return (reg[10].num & 255u);
    }
};

#endif