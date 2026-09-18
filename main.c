#include <stdio.h>
#include <stdint.h>

struct CPU
{
    uint8_t A;
    uint8_t F;// z n h c 0000
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
    uint8_t old_a = cpu->A;
    cpu->A++;
    // z flag
    if(cpu->A == 0x00){
        cpu->F |= 0x80;
    } else{
        cpu->F &= ~0x80; //same as cpu->F |= 0x70
    }
    // h flag
    if ((old_a & 0x0F) == 0x0F){
        cpu->F |= 0x20;
    } else {
        cpu->F &= ~0x20;
    }
    //n flag
    cpu->F &= ~0x40;
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

void cp_a_n(struct CPU *cpu, uint8_t memory[])
{
    uint8_t n = fetch(cpu, memory);
    uint8_t lower_a = cpu->A & 0x0F;
    uint8_t lower_n = n & 0x0F;
    if (cpu->A == n){
        cpu->F |= 0x80;
    } else {
        cpu->F &= ~0x80;
    }
    cpu->F |= 0x40;
    if(lower_a<lower_n){
        cpu->F |= 0x20;
    } else {
        cpu->F &= ~0x20;
    }
    if(cpu->A<n){
        cpu->F |= 0x10;
    } else {
        cpu->F &= ~0x10;
    }
}

void jr_z_r8(struct CPU *cpu, uint8_t memory[])
{
    int8_t offset = (int8_t) (fetch(cpu,memory));
    if(cpu->F & 0x80){
        cpu->PC += offset;
    }

}

int main(void)
{
    struct CPU cpu = {0};

    uint8_t memory[65536];

    cpu.A = 42;
    cpu.PC = 0x0150;
    uint8_t program[] = {
        0x01, 0x34, 0x12,  // LD BC, 0x1234
        0x3E, 0x42,        // LD A, 0x42
        //0x3C,              // INC A
        0xFE, 0x42,        // CP A, n
        0x28, 0x02,        // JR Z, r8
        0x06, 0x99         //LD B, n
    };

    for (int i = 0; i < sizeof(program); i++)
    {
        memory[0x0150 + i] = program[i];
    }

    printf("PC     = 0x%04X\n", cpu.PC);

    while(cpu.PC!=0x0160){
         uint8_t opcode = fetch(&cpu, memory);
         printf("Opcode = 0x%02X\n", opcode);
         switch (opcode)
        {
            case 0x00:
                break;

            case 0x01:
                ld_bc_nn(&cpu,memory);
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
            case 0xFE:
                cp_a_n(&cpu,memory);
                printf("A = 0x%02X, F = 0x%02X, PC = 0x%04X\n",
                    cpu.A, cpu.F, cpu.PC);
                break;
            case 0x28:
                jr_z_r8(&cpu, memory);
                printf("After JR: PC = 0x%04X\n", cpu.PC);
                break;
            case 0x06:
                ld_b_n(&cpu,memory);
                break;
            default:
                printf("Unknown opcode: 0x%02X\n", opcode);
                break;
        }
    }
    printf("-----------------\n");
    printf("A  = 0x%02X\n", cpu.A);
    printf("B  = 0x%02X\n", cpu.B);
    printf("F  = 0x%02X\n", cpu.F);
    printf("PC = 0x%04X\n", cpu.PC);

    return 0;
}
