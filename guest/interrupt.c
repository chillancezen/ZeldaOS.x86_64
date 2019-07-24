/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <interrupt.h>
#include <lib.h>
#include <portio.h>

#define PIC_MASTER_COMMAND_PORT 0x20
#define PIC_MASTER_DATA_PORT 0x21
#define PIC_SLAVE_COMMAND_PORT 0xa0
#define PIC_SLAVE_DATA_PORT 0xa1


__attribute__((aligned(16)))
static struct interrupt_gate_descriptor interrupt_desc_table[256];

__attribute__((aligned(64)))
static struct idt_info idt;

static int_handler * handlers[256];

void
register_interrupt_handler(int vector, int_handler * handler)
{
    handlers[vector] = handler;
}

uint64_t
interrupt_handler(struct cpu_state64 * cpu)
{
    uint64_t rsp = (uint64_t)cpu;
    int_handler * handler = handlers[cpu->vector];
    if (handler) {
        rsp = handler(cpu);
    } else {
        print_string("guest interrupt delivered\n");
    }
    //acknowledge interrupt delivery
    if (cpu->vector >= 40) {
        outb(PIC_SLAVE_COMMAND_PORT, 0x20);
    }
    outb(PIC_MASTER_COMMAND_PORT, 0x20);
    return rsp;
}


static void
set_pl0_interrupt_gate(int vector, void (*isr)(void))
{
    uint64_t isr_qword = (uint64_t)isr;
    struct interrupt_gate_descriptor * pdesc =
        (struct interrupt_gate_descriptor *)&interrupt_desc_table[vector];
    memset(pdesc, 0x0, sizeof(struct interrupt_gate_descriptor));
    pdesc->selector = 0x8;
    pdesc->ist = 0x0;
    pdesc->type = 0xe; // 0xe: interrupt gate, 0xf: trap gate
    pdesc->dpl = 0x0;
    pdesc->present = 1;
    pdesc->offset_0_15 = isr_qword;
    pdesc->offset_16_31 = isr_qword >> 16;
    pdesc->offset_32_63 = isr_qword >> 32;
}

static void
load_idt(void)
{
    idt.idt_limit = sizeof(interrupt_desc_table) - 1;
    idt.idt_base_address = (uint64_t)interrupt_desc_table;
    __asm__ volatile("lidt %0;"
                     :
                     :"m"(idt)
                     :"cc");
    {
        struct idt_info __idt;
        __asm__ volatile("sidt %0;"
                         :"=m"(__idt)
                         :
                         :"memory");
    }
}

void
pic_remap(void)
{
    outb(PIC_MASTER_DATA_PORT, 0xff);
    outb(PIC_SLAVE_DATA_PORT, 0xff);

    outb(PIC_MASTER_COMMAND_PORT, 0x11);
    outb(PIC_SLAVE_COMMAND_PORT, 0x11);

    outb(PIC_MASTER_DATA_PORT, 0x20);
    outb(PIC_SLAVE_DATA_PORT, 0x28);

    outb(PIC_MASTER_DATA_PORT, 0x04);
    outb(PIC_SLAVE_DATA_PORT, 0x02);

    outb(PIC_MASTER_DATA_PORT, 0x01);
    outb(PIC_SLAVE_DATA_PORT, 0x01);

    outb(PIC_MASTER_DATA_PORT, 0x00);
    outb(PIC_SLAVE_DATA_PORT, 0x00);  
}

void
interrupt_init(void)
{
    memset(handlers, 0x0, sizeof(handlers));
#define _(_v) set_pl0_interrupt_gate(_v, int##_v)
    _(0);
    _(1);
    _(2);
    _(3);
    _(4);
    _(5);
    _(6);
    _(7);
    _(8);
    _(9);
    _(10);
    _(11);
    _(12);
    _(13);
    _(14);
    _(15);
    _(16);
    _(17);
    _(18);
    _(19);
    _(20);
    _(21);
    _(22);
    _(23);
    _(24);
    _(25);
    _(26);
    _(27);
    _(28);
    _(29);
    _(30);
    _(31);
    _(32);
    _(33);
    _(34);
    _(35);
    _(36);
    _(37);
    _(38);
    _(39);
    _(40);
    _(41);
    _(42);
    _(43);
    _(44);
    _(45);
    _(46);
    _(47);
    _(48);
    _(49);
    _(50);
    _(51);
    _(52);
    _(53);
    _(54);
    _(55);
    _(56);
    _(57);
    _(58);
    _(59);
    _(60);
    _(61);
    _(62);
    _(63);
    _(64);
    _(65);
    _(66);
    _(67);
    _(68);
    _(69);
    _(70);
    _(71);
    _(72);
    _(73);
    _(74);
    _(75);
    _(76);
    _(77);
    _(78);
    _(79);
    _(80);
    _(81);
    _(82);
    _(83);
    _(84);
    _(85);
    _(86);
    _(87);
    _(88);
    _(89);
    _(90);
    _(91);
    _(92);
    _(93);
    _(94);
    _(95);
    _(96);
    _(97);
    _(98);
    _(99);
    _(100);
    _(101);
    _(102);
    _(103);
    _(104);
    _(105);
    _(106);
    _(107);
    _(108);
    _(109);
    _(110);
    _(111);
    _(112);
    _(113);
    _(114);
    _(115);
    _(116);
    _(117);
    _(118);
    _(119);
    _(120);
    _(121);
    _(122);
    _(123);
    _(124);
    _(125);
    _(126);
    _(127);
    _(128);
    _(129);
    _(130);
    _(131);
    _(132);
    _(133);
    _(134);
    _(135);
    _(136);
    _(137);
    _(138);
    _(139);
    _(140);
    _(141);
    _(142);
    _(143);
    _(144);
    _(145);
    _(146);
    _(147);
    _(148);
    _(149);
    _(150);
    _(151);
    _(152);
    _(153);
    _(154);
    _(155);
    _(156);
    _(157);
    _(158);
    _(159);
    _(160);
    _(161);
    _(162);
    _(163);
    _(164);
    _(165);
    _(166);
    _(167);
    _(168);
    _(169);
    _(170);
    _(171);
    _(172);
    _(173);
    _(174);
    _(175);
    _(176);
    _(177);
    _(178);
    _(179);
    _(180);
    _(181);
    _(182);
    _(183);
    _(184);
    _(185);
    _(186);
    _(187);
    _(188);
    _(189);
    _(190);
    _(191);
    _(192);
    _(193);
    _(194);
    _(195);
    _(196);
    _(197);
    _(198);
    _(199);
    _(200);
    _(201);
    _(202);
    _(203);
    _(204);
    _(205);
    _(206);
    _(207);
    _(208);
    _(209);
    _(210);
    _(211);
    _(212);
    _(213);
    _(214);
    _(215);
    _(216);
    _(217);
    _(218);
    _(219);
    _(220);
    _(221);
    _(222);
    _(223);
    _(224);
    _(225);
    _(226);
    _(227);
    _(228);
    _(229);
    _(230);
    _(231);
    _(232);
    _(233);
    _(234);
    _(235);
    _(236);
    _(237);
    _(238);
    _(239);
    _(240);
    _(241);
    _(242);
    _(243);
    _(244);
    _(245);
    _(246);
    _(247);
    _(248);
    _(249);
    _(250);
    _(251);
    _(252);
    _(253);
    _(254);
    _(255);
#undef _
    load_idt();
    pic_remap();
    print_string("Finishing guest interrupt initialization\n");
}
