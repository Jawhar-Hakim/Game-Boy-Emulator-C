//IMPORTS
#include <stdio.h>
#include <stdin.h>

//DEFINES
#define FLAG_Z 0x80
#define FLAG_N 0x40
#define FLAG_H 0x20
#define FLAG_C 0x10
#define FLAG_MASK 0xF0

//CPU
struct CPU{
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
}

//HELPERS
uint8_t fetch(struct CPU *cpu, uint8_t memory[])
{
  uint8_t value = memory[cpu->PC];
  cpu->PC++;
  return value;
}

uint16_t get_bc(struct CPU *cpu)
{
  return (cpu->B << 8) | cpu->C;
}

uint16_t get_hl(uint8_t high, uint8_t low)
{
  //example: get_hl(cpu->B, cpu->C)
  return ((uint16_t)high << 8) | low;
}

void set_bc(struct CPU *cpu, uint16_t value)
{
  cpu->C = value;
  cpu->B = value >> 8;
}
void set_hl(uint8_t *high, uint8_t *low, uint16_t value)
{
  //example: set_hl(&cpu->B, &cpu->C, value)
  //or: set_hl(&cpu->B, &cpu->C, fetch(cpu, memory) | ((uint16_t)fetch(cpu,memory) << 8))
  *high = value >> 8;
  *low = value;
}
/*
uint16_t get_hl(struct CPU *cpu)
{
  return (cpu->H << 8) | cpu->C;
}*/

//LOADS
void ld_nn_n(uint8_t *nn, struct CPU *cpu, uint8_t memory[])
{
  // example: ld_nn_n(&cpu->B,cpu,memory);
  *nn = fetch(cpu,memory);
}

void ld_r1_r2(uint8_t *r1, uint8_t r2)
{
  //example: ld_r1_r2(&cpu->A,cpu->A)
  *r1 = r2;
}

void ld_r1_address(uint8_t *r1, uint16_t address, uint8_t memory[])
{
  //example: ld_r1_address(&cpu->A, get_bc(cpu), memory);
  //or: ld_r1_address(&cpu->A, fetch(cpu,memory) | (fetch(cpu,memory) << 8), memory);
  //or: ld_r1_address(&cpu->A, 0xFF00 | cpu->C, memory);
  //or: ld_r1_address(&cpu->A, 0xFF00 | fetch(cpu,memory), memory)
  *r1 = memory[address]
}

void ld_address_r1(uint16_t address, uint8_t r1, uint8_t memory[])
{
  //example: ld_address_r1(get_bc(cpu),cpu->A,memory)
  //or: ld_address_r1(fetch(cpu,memory) | (fetch(cpu,memory) << 8), cpu->A, memory)
  //or: ld_address_r1(0xFF00 | cpu->C, cpu->A, memory)
  //or: ld_address_r1(0xFF00 | fetch(cpu,memory), cpu->A, memory)
  memory[address] = r1;
}

void ld_a_hl_inc(struct CPU *cpu, uint8_t memory[])
{
    uint16_t hl = get_hl(cpu->H, cpu->L);
    cpu->A = memory[hl];
    hl++;
    cpu->H = hl >> 8;
    cpu->L = hl & 0xFF;
}

void ld_a_hl_dec(struct CPU *cpu, uint8_t memory[])
{
    uint16_t hl = get_hl(cpu->H,cpu->L);
    cpu->A = memory[hl];
    hl--;
    cpu->H = hl >> 8;
    cpu->L = hl & 0xFF;
}

void ld_hl_inc_a(struct CPU *cpu, uint8_t memory[])
{
    uint16_t hl = get_hl(cpu->H,cpu->L);

    memory[hl] = cpu->A;

    hl++;

    cpu->H = hl >> 8;
    cpu->L = hl & 0xFF;
}

void ld_hl_dec_a(struct CPU *cpu, uint8_t memory[])
{
    uint16_t hl = get_hl(cpu->H, cpu->L);

    memory[hl] = cpu->A;

    hl--;

    cpu->H = hl >> 8;
    cpu->L = hl & 0xFF;
}

void ld_sp_hl(struct CPU *cpu)
{
  cpu->SP = get_hl(cpu->H, cpu->L);
}

//STACK
void push_nn(uint8_t high, uint8_t low, struct CPU *cpu, uint8_t memory[])
{
  //example: push_nn(cpu->A,cpu->F, cpu, memory)
  cpu->SP--;
  memory[cpu->SP] = high;
  cpu->SP--;
  memory[cpu->SP] = low;
}

void pop_nn(uint8_t *high, uint8_t *low, struct CPU *cpu, uint8_t memory[])
{
  //example: pop_nn(&cpu->A,&cpu->F,cpu,memory);
  *low = memory[cpu->SP];
  cpu->SP++;
  *high = memory[cpu->SP];
  cpu->SP++;
}

//8BIT ALU
void add_a_n(struct CPU *cpu, uint8_t n)
{
  //example: add_a_n(cpu, cpu->B);
  //or: add_a_n(cpu, fetch(cpu,memory))
  //or: add_a_n(cpu, memory[hl])
  uint16_t result = cpu->A + n;
  if (result == 0x00){
    cpu->F |= FLAG_Z;
  } else{
    cpu->F &= ~FLAG_Z
  }
  cpu->F &= ~FLAG_N;
  if ((cpu->A & 0x0F)+(n & 0x0F) & 0x10) {
    cpu->F |=FLAG_H;
  } else{
    cpu->F &= ~FLAG_H;
  }
  if (result > 0xFF){
    cpu->F |= FLAG_C;
  } else{
    cpu->F &= ~FLAG_C;
  }
}

void inc_u8(uint8_t *u8)
{
  //example: inc_u8(&cpu->A);
  if (*u8 == 0xFF){
    cpu->F |= FLAG_Z;
  } else{
    cpu->F &= ~FLAG_Z;
  }
  cpu->F &= ~FLAG_N;
  if ((*u8 & 0x0F)==0x0F){
    cpu->F |= FLAG_H;
  } else{
    cpu->F &= ~FLAG_H;
  }
  (*u8)++;
}

void adc_a_n(struct CPU *cpu, uint8_t n)
{
  uint16_t result = (uint16_t) cpu->A + n + ((cpu->F & FLAG_C) >> 4);
  if (result == 0x00){
    cpu->F |= FLAG_Z;
  } else{
    cpu->F &= ~FLAG_Z
  }
  cpu->F &= ~FLAG_N;
  if ((cpu->A & 0x0F)+(n & 0x0F) & 0x10) {
    cpu->F |=FLAG_H;
  } else{
    cpu->F &= ~FLAG_H;
  }
  if (result > 0xFF){
    cpu->F |= FLAG_C;
  } else{
    cpu->F &= ~FLAG_C;
  }
  cpu->A = result;
}

void sub_a_n(struct CPU *cpu, uint8_t n)
{
  //example: sub_a_n(cpu, memory[hl])
  //or: sub_a_n(cpu, cpu->B);
  //or: sub_a_n(cpu, fetch(cpu,memory))
  uint16_t result = cpu->A - n;
  if (result == 0x00){
    cpu->F |= FLAG_Z;
  } else{
    cpu->F &= ~FLAG_Z;
  }
  cpu->F |= FLAG_N;
  if ((cpu->A & 0x0F) < (n & 0x0F)){
    cpu->F |= FLAG_H;
  } else{
    cpu->F &= ~FLAG_H;
  }
  if (cpu->A < n){
    cpu->F |= FLAG_C;
  } else{
    cpu->F &= ~FLAG_C;
  }
  cpu->A = result;
}

void sbc_a_n(struct CPU *cpu, uint8_t n)
{
  uint16_t result = (uint16_t) cpu->A - (n + ((cpu->F & FLAG_C) >> 4));
  if (result == 0x00){
    cpu->F |= FLAG_Z;
  } else{
    cpu->F &= ~FLAG_Z;
  }
  cpu->F |= FLAG_N;
  if ((cpu->A & 0x0F) < (n & 0x0F)){
    cpu->F |= FLAG_H;
  } else{
    cpu->F &= ~FLAG_H;
  }
  if (cpu->A < n){
    cpu->F |= FLAG_C;
  } else{
    cpu->F &= ~FLAG_C;
  }
  cpu->A = result;

}

void and_a_n(struct CPU *cpu, uint8_t n)
{
  uint8_t result = cpu->A & n;
  if (result == 0x00){
    cpu->F |= FLAG_Z;
  } else{
    cpu->F &= ~FLAG_Z;
  }
  cpu->F &= ~FLAG_N;
  cpu->F |= FLAG_H;
  cpu->F &= ~FLAG_C;
  cpu->A = result;
}

void or_a_n(struct CPU *cpu, uint8_t n)
{
  uint8_t result = cpu->A | n;
  if (result == 0x00){
    cpu->F |= FLAG_Z;
  } else{
    cpu->F &= ~FLAG_Z;
  }
  cpu->F &= ~FLAG_N;
  cpu->F &= ~FLAG_H;
  cpu->F &= ~FLAG_C;
  cpu->A = result;
}

void xor_a_n(struct CPU *cpu, uint8_t n)
{
  uint8_t result = cpu->A ^ n;
  if (result == 0x00){
    cpu->F |= FLAG_Z;
  } else{
    cpu->F &= ~FLAG_Z;
  }
  cpu->F &= ~FLAG_N;
  cpu->F &= ~FLAG_H;
  cpu->F &= ~FLAG_C;
  cpu->A = result;
}

void dec_n(uint8_t *n)
{
  if(((*n) - 1)==0x00){
    cpu->F |= FLAG_Z;
  } else{
    cpu->F &= ~FLAG_Z;
  }
  cpu->F |= FLAG_N;
  if ((*n & 0x0F)<0x01){
    cpu->F |= FLAG_H;
  } else{
    cpu->F &= ~FLAG_H;
  }
  (*n)--;
}

//16bit arithmetic
void add_hl_n(struct CPU *cpu, uint16_t n)
{
  uint16_t hl = get_hl(cpu->H,cpu->L);
  cpu->F &= ~FLAG_N;
  if ((hl & 0xFFF) + (n & 0xFFF) > 0xFFF) {
    cpu->F |= FLAG_H;
  } else {
    cpu->F &= ~FLAG_H;
  }
  if ((hl & 0xFFFF) + (n & 0xFFFF) > 0xFFFF) {
    cpu->F |= FLAG_C;
  } else {
    cpu->F &= ~FLAG_C;
  }
  cpu->L = hl+n;
  cpu->H = (hl+n)>>8;
}

void add_sp_n(struct CPU *cpu, uint8_t n)
{
  int8_t signed_n = (int8_t) n;
  cpu->F &= ~FLAG_Z;
  cpu->F &= ~FLAG_N;

}

void inc_nn(uint8_t *high, uint8_t *low)
{
  uint16_t v = ((*high)<<8)|(*low);
  v++;
  *high = v >> 8;
  *low = v;
}

void dec_nn(uint8_t *high, uint8_t *low)
{
  uint16_t v = ((*high)<<8)|(*low);
  v--;
  *high = v >> 8;
  *low = v;
}


