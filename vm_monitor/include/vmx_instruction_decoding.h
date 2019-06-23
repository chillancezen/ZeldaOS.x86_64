/*
 * Copyright (c) 2019 Jie Zheng
 */

#ifndef _VMX_INSTRUCTION_DECODING_H
#define _VMX_INSTRUCTION_DECODING_H
#include <lib64/include/type.h>

// https://wiki.osdev.org/X86-64_Instruction_Encoding
// The REX prefix must be encoded when:
// 1) using 64-bit operand size and the instruction does not default to 64-bit
//    operand size;
// 2) using one of the extended registers (R8 to R15, XMM8 to XMM15, YMM8 to 
//    YMM15, CR8 to CR15 and DR8 to DR15) 
// 3) using one of the uniform byte registers SPL, BPL, SIL or DIL.
struct instruction_context {
    int instruction_length;
    uint8_t is_lock_prefixed;
    uint8_t is_repnez_prefixed;
    uint8_t is_rep_prefixed;
    uint8_t is_cs_overrided;
    uint8_t is_ss_overrided;
    uint8_t is_ds_overrided;
    uint8_t is_es_overrided;
    uint8_t is_fs_overrided;
    uint8_t is_gs_overrided;
    uint8_t branch_not_taken;
    uint8_t branch_taken;
    uint8_t operand_size_overrided;
    uint8_t address_size_overrided;
    
    uint8_t opcode_escape0;
    uint8_t opcode_escape1;

    uint8_t rex_prefix;
    uint32_t opcode;
    uint32_t opcode_length;
    uint8_t moderm;

    uint64_t operand_hint;
    uint8_t reg_index;
    uint8_t rm_index;
    uint8_t access_size;
    uint64_t immediate;
};

#define ACCESS_IN_BYTE 0x1
#define ACCESS_IN_WORD 0x2
#define ACCESS_IN_DWORD 0x4
#define ACCESS_IN_QWORD 0x8

#define OPERAND_HINT_MR 0x1
#define OPERAND_HINT_RM 0x2
#define OPERAND_HINT_IM 0x3

#define REX_W_MASK 0x08
#define REX_R_MASK 0x04
#define REX_X_MASK 0x02
#define REX_B_MASK 0x01

#define MODERM_MODE(moderm) (((moderm) >> 6) & 0x3)
#define MODERM_REG(moderm) (((moderm) >> 3) & 0x7)
#define MODERM_RM(moderm) ((moderm) & 0x7)

#define REX_W_BIT(rex) (!!((rex) & REX_W_MASK))
#define REX_R_BIT(rex) (!!((rex) & REX_R_MASK))
#define REX_X_BIT(rex) (!!((rex) & REX_X_MASK))
#define REX_B_BIT(rex) (!!((rex) & REX_B_MASK))

#define REG_INDEX(context)                                                     \
    (MODERM_REG(context->moderm) | (REX_R_BIT(context->rex_prefix) ? 1 << 3 : 0))
#define REG_MEM_INDEX(context)                                                 \
    (MODERM_RM(context->moderm) | (REX_B_BIT(context->rex_prefix) ? 1 << 3 : 0))


#define OPCODE_MASK_BYTE 0xff
#define OPCODE_MASK_WORD 0xffff
#define OPCODE_MASK_TRIBYTES 0xffffff

int
decode_x86_64_instruction(const uint8_t * instruction_stream,
                          struct instruction_context * context);
#endif
