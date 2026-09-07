#include <stdio.h>
#include <stdint.h>

struct CPU
{
    uint8_t A;
    uint8_t F;
    uint8_t B;
    uint8_t C;

    uint8_t D;
    uint8_t E;
    uint8_t H;
    uint8_t L;

    uint16_t PC;
    uint16_t SP;
};

uint8_t fetch(struct CPU *cpu, uint8_t memory[])
{
    uint8_t value = memory[cpu->PC];
    cpu->PC++;
    return value;
}

void ld_a_n(struct CPU *cpu, uint8_t memory[])
{
    cpu->A = fetch(cpu, memory);
}

void ld_b_n(struct CPU *cpu, uint8_t memory[])
{
    cpu->B = fetch(cpu,memory);
}

void inc_a(struct CPU *cpu)
{
    cpu->A++;
}

uint16_t get_bc(struct CPU *cpu)
{
    return (cpu->B << 8) | cpu->C;
}

void set_bc(struct CPU *cpu, uint16_t value)
{
    cpu->B = value >> 8;
    cpu->C = value;
}

void ld_bc_nn(struct CPU *cpu, uint8_t memory[])
{
    uint8_t low = fetch(cpu, memory);
    uint8_t high = fetch(cpu, memory);

    set_bc(cpu, ((uint16_t)high << 8) | low);
}

int main(void)
{
    struct CPU cpu;

    uint8_t memory[65536];

    cpu.A = 42;
    cpu.PC = 0x0150;
    uint8_t program[] = {
        0x01, 0x34, 0x12,  // LD BC, 0x1234
        0x3E, 0x42,        // LD A, 0x42
        0x3C               // INC A
    };

    for (int i = 0; i < sizeof(program); i++)
    {
        memory[0x0150 + i] = program[i];
    }

    printf("PC     = 0x%04X\n", cpu.PC);

    while(cpu.PC!=0x0156){
         uint8_t opcode = fetch(&cpu, memory);
         printf("Opcode = 0x%02X\n", opcode);
         switch (opcode)
        {
            case 0x00:
                break;

            case 0x01:
                ld_bc_nn(&cpu,memory); //incompatible type
                break;

            case 0x3E:
                ld_a_n(&cpu,memory);
                printf("A      = 0x%02X\n", cpu.A);
                printf("PC     = 0x%04X\n", cpu.PC);
                break;

            case 0x3C:
                inc_a(&cpu);
                printf("A      = 0x%02X\n", cpu.A);
                printf("PC     = 0x%04X\n", cpu.PC);
                break;
            default:
                printf("Unknown opcode: 0x%02X\n", opcode);
                break;
        }
    }
    printf("-----------------\n");
    printf("A      = 0x%02X\n", cpu.A);
    printf("B      = 0x%02X\n", cpu.B);
    printf("BC     = 0x%04X\n", get_bc(&cpu)); //incompatible type
    printf("PC     = 0x%04X\n", cpu.PC);

    return 0;
}
