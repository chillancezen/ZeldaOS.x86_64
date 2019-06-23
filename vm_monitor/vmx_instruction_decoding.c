/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <vm_monitor/include/vmx_instruction_decoding.h>
#include <lib64/include/string.h>
#include <lib64/include/logging.h>

typedef void opcode_resolver(const uint8_t * instruction_stream, struct instruction_context * context);


static void
opcode_8a_resolver(const uint8_t * instruction_stream,
                   struct instruction_context * context)
{
    // This is read one byte from memory to a register
    context->moderm = *instruction_stream;
    context->instruction_length += 1;
    context->rm_index = REG_MEM_INDEX(context);
    context->reg_index = REG_INDEX(context);
    context->access_size = ACCESS_IN_BYTE;
}

static void
opcode_8b_resolver(const uint8_t * instruction_stream,
                   struct instruction_context * context)
{
    context->moderm = *instruction_stream;
    context->instruction_length += 1;
    context->rm_index = REG_MEM_INDEX(context);
    context->reg_index = REG_INDEX(context);
    if (context->operand_size_overrided) {
        context->access_size = ACCESS_IN_WORD;
    } else if (REX_W_BIT(context->rex_prefix)) {
        context->access_size = ACCESS_IN_QWORD;
    } else {
        context->access_size = ACCESS_IN_DWORD;
    }
}

static void
opcode_c6_resolver(const uint8_t * instruction_stream,
                   struct instruction_context * context)
{
    context->moderm = *instruction_stream;
    context->immediate = *(instruction_stream + 1);
    context->instruction_length += 2;
    context->rm_index = REG_MEM_INDEX(context);
    context->reg_index = REG_INDEX(context);
    context->access_size = ACCESS_IN_BYTE;
}

static void
opcode_c7_resolver(const uint8_t * instruction_stream,
                   struct instruction_context * context)
{
    context->moderm = *instruction_stream;
    context->rm_index = REG_MEM_INDEX(context);
    context->reg_index = REG_INDEX(context);
    if (context->operand_size_overrided) {
        context->access_size = ACCESS_IN_WORD;
        context->immediate = *(uint16_t *)(instruction_stream + 1);
        context->instruction_length += 3;
    } else if (REX_W_BIT(context->rex_prefix)) {
        context->access_size = ACCESS_IN_QWORD;
        context->immediate = *(uint32_t *)(instruction_stream + 1);
        context->instruction_length += 5;
    } else {
        context->access_size = ACCESS_IN_DWORD;
        context->immediate = *(uint32_t *)(instruction_stream + 1);
        context->instruction_length += 5;
    }
}

static void
opcode_b6_resolver(const uint8_t * instruction_stream,
                   struct instruction_context * context)
{
    context->moderm = *instruction_stream;
    context->instruction_length += 1;
    context->rm_index = REG_MEM_INDEX(context);
    context->reg_index = REG_INDEX(context);
    context->access_size = ACCESS_IN_BYTE;
}

static void
opcode_b7_resolver(const uint8_t * instruction_stream,
                   struct instruction_context * context)
{
    context->moderm = *instruction_stream;
    context->instruction_length += 1;
    context->rm_index = REG_MEM_INDEX(context);
    context->reg_index = REG_INDEX(context);
    context->access_size = ACCESS_IN_WORD;
}

struct opcode_def {
    uint32_t code;
    uint32_t mask;
    uint8_t operand_hint;
    opcode_resolver * resolve;
} predefined_opcodes[]= {
    //https://www.felixcloutier.com/x86/mov
    //https://www.felixcloutier.com/x86/movzx
    {0xb6, OPCODE_MASK_BYTE, OPERAND_HINT_MR, opcode_b6_resolver},
    {0xb7, OPCODE_MASK_BYTE, OPERAND_HINT_MR, opcode_b7_resolver},
    {0xc6, OPCODE_MASK_BYTE, OPERAND_HINT_IM, opcode_c6_resolver},
    {0xc7, OPCODE_MASK_BYTE, OPERAND_HINT_IM, opcode_c7_resolver},
    {0x8a, OPCODE_MASK_BYTE, OPERAND_HINT_MR, opcode_8a_resolver},
    {0x8b, OPCODE_MASK_BYTE, OPERAND_HINT_MR, opcode_8b_resolver},
    {0x88, OPCODE_MASK_BYTE, OPERAND_HINT_RM, opcode_8a_resolver},
    {0x89, OPCODE_MASK_BYTE, OPERAND_HINT_RM, opcode_8b_resolver},
};

int
decode_x86_64_instruction(const uint8_t * instruction_stream,
                          struct instruction_context * context)
{
    int idx = 0;
    const uint8_t * lptr = instruction_stream;
    memset(context, 0x0, sizeof(struct instruction_context));
    // resolve the prefix groups 1,2,3,4
    for (; *lptr; lptr++) {
        uint8_t is_to_stop = 0;
        switch(*lptr)
        {
#define _(code, field)                                                         \
            case code:                                                         \
                context->field = 1;                                            \
                break;
            _(0x0f, opcode_escape0);
            _(0x38, opcode_escape1);
            _(0x3a, opcode_escape1);
            _(0xf0, is_lock_prefixed);
            _(0xf2, is_repnez_prefixed);
            _(0xf3, is_rep_prefixed);
            _(0x2e, is_cs_overrided);
            _(0x36, is_ss_overrided);
            _(0x3e, is_ds_overrided);
            _(0x26, is_es_overrided);
            _(0x64, is_fs_overrided);
            _(0x65, is_gs_overrided);
            _(0x66, operand_size_overrided);
            _(0x67, address_size_overrided);
            default:
                is_to_stop = 1;
                break;
#undef _
        }
        if (is_to_stop) {
            break;
        }
        context->instruction_length++;
    }
    // examine REX prefix
    if (*lptr >= 0x40 && *lptr <= 0x4f) {
        context->rex_prefix = *lptr++;
        context->instruction_length++;
    }
    // examine the opcode
    for (idx = 0; idx < sizeof(predefined_opcodes) / sizeof(struct opcode_def);
         idx++) {
        if ((predefined_opcodes[idx].mask & predefined_opcodes[idx].code) ==
            (predefined_opcodes[idx].mask & *(uint32_t *)lptr)) {
            switch (predefined_opcodes[idx].mask)
            {
                case OPCODE_MASK_BYTE:
                    context->opcode = *(uint8_t*)lptr;
                    context->opcode_length = 1;
                    lptr += 1;
                    context->instruction_length += 1;
                    break;
                case OPCODE_MASK_WORD:
                    context->opcode = *(uint16_t*)lptr;
                    context->opcode_length = 2;
                    lptr += 2;
                    context->instruction_length += 2;
                    break;
                case OPCODE_MASK_TRIBYTES:
                    context->opcode = *(uint32_t*)lptr &
                                      predefined_opcodes[idx].mask;
                    context->opcode_length = 3;
                    lptr += 3;
                    context->instruction_length += 3;
                    break;
                default:
                    __not_reach();
                    break;
            }
            context->operand_hint = predefined_opcodes[idx].operand_hint;
            ASSERT(predefined_opcodes[idx].resolve);
            predefined_opcodes[idx].resolve(lptr, context);
            break;
        }
    }
    ASSERT(context->opcode);
    return ERROR_OK;
}
