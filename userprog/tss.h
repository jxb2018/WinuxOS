#ifndef _USERPROG_TSS_H
#define _USERPROG_TSS_H

void tss_init(void);
void update_tss_esp(struct task_struct* pthread);
struct gdt_desc make_gdt_desc(uint32_t* base_addr_, uint32_t limit, uint8_t attr_low, uint8_t attr_high);

#endif