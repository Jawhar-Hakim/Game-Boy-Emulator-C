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

void jr_nz_r8(struct CPU *cpu, uint8_t memory[])
{
    int8_t offset = (int8_t) (fetch(cpu,memory));
    if(!(cpu->F & 0x80)){
        cpu->PC += offset;
    }
}

void jr_c_r8(struct CPU *cpu, uint8_t memory[])
{
    int8_t offset = (int8_t) (fetch(cpu,memory));
    if(cpu->F & 0x10){
        cpu->PC += offset;
    }
}

void jr_nc_r8(struct CPU *cpu, uint8_t memory[])
{
    int8_t offset = (int8_t) (fetch(cpu,memory));
    if(!(cpu->F & 0x10)){
        cpu->PC += offset;
    }
}

void jp_a16(struct CPU *cpu, uint8_t memory[])
{
    uint16_t address = (fetch(cpu,memory)) | (fetch(cpu,memory) << 8);
    cpu->PC = address;
}

void jp_nz_a16(struct CPU *cpu, uint8_t memory[])
{
    uint16_t address = (fetch(cpu,memory)) | (fetch(cpu,memory) << 8);
    if(!(cpu->F & 0x80)){
        cpu->PC = address;
    }
}

void jp_z_a16(struct CPU *cpu, uint8_t memory[])
{
    uint16_t address = (fetch(cpu,memory)) | (fetch(cpu,memory) << 8);
    if((cpu->F & 0x80)){
        cpu->PC = address;
    }
}

void jp_c_a16(struct CPU *cpu, uint8_t memory[])
{
    uint16_t address = (fetch(cpu,memory)) | (fetch(cpu,memory) << 8);
    if((cpu->F & 0x10)){
        cpu->PC = address;
    }
}

void jp_nc_a16(struct CPU *cpu, uint8_t memory[])
{
    uint16_t address = (fetch(cpu,memory)) | (fetch(cpu,memory) << 8);
    if(!(cpu->F & 0x10)){
        cpu->PC = address;
    }
}

void push_bc(struct CPU *cpu, uint8_t memory[])
{
    uint16_t value = get_bc(cpu);
    cpu->SP--;
    memory[cpu->SP] = value >> 8;
    cpu->SP--;
    memory[cpu->SP] = value & 0x00FF;
}

void pop_bc(struct CPU *cpu, uint8_t memory[])
{
    cpu->C = memory[cpu->SP];
    cpu->SP++;
    cpu->B = memory[cpu->SP];
    cpu->SP++;
}
void push_de(struct CPU *cpu, uint8_t memory[])
{
    cpu->SP--;
    memory[cpu->SP] = cpu->D;
    cpu->SP--;
    memory[cpu->SP] = cpu->E;
}

void pop_de(struct CPU *cpu, uint8_t memory[])
{
    cpu->E = memory[cpu->SP];
    cpu->SP++;
    cpu->D = memory[cpu->SP];
    cpu->SP++;
}

void push_hl(struct CPU *cpu, uint8_t memory[])
{
    cpu->SP--;
    memory[cpu->SP] = cpu->H;
    cpu->SP--;
    memory[cpu->SP] = cpu->L;
}

void pop_hl(struct CPU *cpu, uint8_t memory[])
{
    cpu->L = memory[cpu->SP];
    cpu->SP++;
    cpu->H = memory[cpu->SP];
    cpu->SP++;
}

void push_af(struct CPU *cpu, uint8_t memory[])
{
    cpu->SP--;
    memory[cpu->SP] = cpu->A;
    cpu->SP--;
    memory[cpu->SP] = cpu->F;
}

void pop_af(struct CPU *cpu, uint8_t memory[])
{
    cpu->F = memory[cpu->SP] & 0xF0;
    cpu->SP++;
    cpu->A = memory[cpu->SP];
    cpu->SP++;
}

void push_16(struct CPU *cpu, uint8_t memory[], uint16_t value)
{
    cpu->SP--;
    memory[cpu->SP] = value >> 8;
    cpu->SP--;
    memory[cpu->SP] = value;
}

uint16_t pop_16(struct CPU *cpu, uint8_t memory[])
{
    uint16_t value = memory[cpu->SP];
    cpu->SP++;
    value |= (memory[cpu->SP] << 8);
    cpu->SP++;
    return value;
}

void call_a16(struct CPU *cpu, uint8_t memory[])
{
    uint16_t value = fetch(cpu,memory);
    value |= fetch(cpu,memory) << 8;
    push_16(cpu, memory, cpu->PC);
    cpu->PC = value;
}

void ret(struct CPU *cpu, uint8_t memory[])
{
    cpu->PC = pop_16(cpu,memory);
}

void inc_bc(struct CPU *cpu)
{
    uint16_t bc = get_bc(cpu);
    bc++;
    set_bc(cpu, bc);
}

void dec_bc(struct CPU *cpu)
{
    uint16_t bc = get_bc(cpu);
    bc--;
    set_bc(cpu, bc);
}

int main(void)
{
    struct CPU cpu = {0};

    uint8_t memory[65536];

    cpu.A = 42;
    cpu.PC = 0x0150;
    cpu.SP = 0xFFFE;
    uint8_t program[] = {
        0x01, 0x34, 0x12,  // LD BC, 0x1234
        0x3E, 0x42,        // LD A, 0x42
        //0x3C,              // INC A
        0xFE, 0x99,        // CP A, n
        //0x28, 0x02,        // JR Z, r8
        0x20, 0x02,         //JR NZ, r8
        0x06, 0x77,        //LD B, n
        0xC5,              // push bc
        0xC1,              // pop bc
        0xD5,              // push de
        0xD1,              // pop de
        0xE5,              // push hl
        0xE1,              // pop hl
        0xF5,              // push af
        0xF1,              // pop af
        0x03,              // inc bc
        0x0B,              // dec bc

    };

    for (int i = 0; i < sizeof(program); i++)
    {
        memory[0x0150 + i] = program[i];
    }

    printf("PC     = 0x%04X\n", cpu.PC);

    while(cpu.PC < 0x0150 + sizeof(program)){
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
            case 0x20:
                jr_nz_r8(&cpu, memory);
                break;
            case 0xC5:
                push_bc(&cpu,memory);
                break;
            case 0xC1:
                pop_bc(&cpu,memory);
                break;
            case 0xD5:
                push_de(&cpu,memory);
                break;
            case 0xD1:
                pop_de(&cpu,memory);
                break;
            case 0xE5:
                push_hl(&cpu,memory);
                break;
            case 0xE1:
                pop_hl(&cpu,memory);
                break;
            case 0xF5:
                push_af(&cpu,memory);
                break;
            case 0xF1:
                pop_af(&cpu,memory);
                break;
            case 0x03:
                inc_bc(&cpu);
                break;
            case 0x0B:
                dec_bc(&cpu);
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
