#include "exceptions.hpp"
#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

class Instruction
{
public:
	instructiontype type;
	uint imm;
	uint rs1;
	uint rs2;
	uint rd;

	Instruction()
	{
		type = NONE;
		imm = 0;
		rs1 = 0, rs2 = 0;
		rd = 0;
	}

	Instruction getDEC(uint num)
	{
		uint p1, p2, p3, p4;
		uint opcode;
		uint func3;
		imm = 0;
		rs1 = 0;
		rs2 = 0;
		rd = 0;
		uint func7;
		opcode = (num & 127);
		func3 = ((num >> 12) & 7);
		func7 = ((num >> 25) & 127);
		switch (opcode)
		{
		case 55: // 0110111
			type = LUI;
			break;
		case 23: // 0010111
			type = AUIPC;
			break;
		case 111: // 1101111
			type = JAL;
			break;
		case 103: // 1100111
			type = JALR;
			break;
		case 99: // 1100011
			switch (func3)
			{
			case 0: // 000
				type = BEQ;
				break;
			case 1: // 001
				type = BNE;
				break;
			case 4: // 100
				type = BLT;
				break;
			case 5: // 101
				type = BGE;
				break;
			case 6: // 110
				type = BLTU;
				break;
			case 7: // 111
				type = BGEU;
				break;
			}
			break;
		case 3: // 0000011
			switch (func3)
			{
			case 0: // 000
				type = LB;
				break;
			case 1: // 001
				type = LH;
				break;
			case 2: // 010
				type = LW;
				break;
			case 4: // 100
				type = LBU;
				break;
			case 5: // 101
				type = LHU;
				break;
			}
			break;
		case 35: // 0100011
			switch (func3)
			{
			case 0: // 000
				type = SB;
				break;
			case 1: // 001
				type = SH;
				break;
			case 2: // 010
				type = SW;
				break;
			}
			break;
		case 19: // 0010011
			switch (func3)
			{
			case 0: // 000
				type = ADDI;
				break;
			case 1: // 001
				type = SLLI;
				break;
			case 2: // 010
				type = SLTI;
				break;
			case 3: // 011
				type = SLTIU;
				break;
			case 4: // 100
				type = XORI;
				break;
			case 5: // 101
				if (func7 == 0)
					type = SRLI; // 0000000
				else
					type = SRAI; // 0100000
				break;
			case 6: // 110
				type = ORI;
				break;
			case 7: // 111
				type = ANDI;
				break;
			}
			break;
		case 51:
			switch (func3)
			{
			case 0: // 000
				if (func7 == 0)
					type = ADD; // 0000000
				else
					type = SUB; // 0100000
				break;
			case 1: // 001
				type = SLL;
				break;
			case 2: // 010
				type = SLT;
				break;
			case 3: // 011
				type = SLTU;
				break;
			case 4: // 100
				type = XOR;
				break; // 100
			case 5:	   // 101
				if (func7 == 0)
					type = SRL; // 0000000
				else
					type = SRA; // 0100000
				break;
			case 6: // 110
				type = OR;
				break;
			case 7: // 111
				type = AND;
				break;
			}
			break;
		default:
			type = NOP;
		}
		switch (type)
		{
		case LUI:
		case AUIPC:
			imm = (num >> 12) << 12;
			rd = (num >> 7) & 31;
			break;
		case JAL:
			p1 = (num >> 12) & 255;
			p2 = (num >> 20) & 1;
			p3 = (num >> 21) & 1023;
			p4 = (num >> 31) & 1;
			imm = (p3 << 1) + (p2 << 11) + (p1 << 12) + (p4 << 20);
			imm = sext(imm, 20);
			rd = (num >> 7) & 31;
			break;
		case JALR:
			imm = sext(((num >> 20) & 4095), 12);
			rd = (num >> 7) & 31;
			rs1 = (num >> 15) & 31;
			break;
		case BEQ:
		case BNE:
		case BLT:
		case BGE:
		case BLTU:
		case BGEU:
			p1 = (num >> 7) & 1;
			p2 = (num >> 8) & 15;
			p3 = (num >> 25) & 63;
			p4 = (num >> 31) & 1;
			imm = (p2 << 1) + (p3 << 5) + (p1 << 11) + (p4 << 12);
			imm = sext(imm, 12);
			rs1 = (num >> 15) & 31;
			rs2 = (num >> 20) & 31;
			break;
		case LB:
		case LH:
		case LW:
		case LBU:
		case LHU:
			imm = sext(((num >> 20) & 4095), 12);
			rd = (num >> 7) & 31;
			rs1 = (num >> 15) & 31;
			break;
		case SB:
		case SH:
		case SW:
			p1 = (num >> 7) & 31;
			p2 = (num >> 25) & 127;
			imm = p1 + (p2 << 5);
			imm = sext(imm, 12);
			rs1 = (num >> 15) & 31;
			rs2 = (num >> 20) & 31;
			break;
		case ADDI:
		case SLTI:
		case SLTIU:
		case XORI:
		case ORI:
		case ANDI:
			imm = sext(((num >> 20) & 4095), 12);
			rs1 = (num >> 15) & 31;
			rd = (num >> 7) & 31;
			break;
		case SLLI:
		case SRLI:
		case SRAI:
			rd = (num >> 7) & 31;
			rs1 = (num >> 15) & 31;
			imm = (num >> 20) & 31;
			imm = sext(imm, 5);
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
			rd = (num >> 7) & 31;
			rs1 = (num >> 15) & 31;
			rs2 = (num >> 20) & 31;
			break;
		}
		return *this;
	}
};

#endif
