/*
 * Copyright 2016 Nikita Edward Baruzdin <nebaruzdin@gmail.com>
 *
 * The module creates a pseudo-file /proc/idt that shows the contents of the
 * Interrupt Descriptor Table (IDT).
 *
 * IDT format is described in the "Intel 64 and IA-32 Architectures Software
 * Developer's Manual. Volume 3A: System Programming Guide, Part 1".
 */

#include <linux/module.h>     // init_module(), printk()
#include <linux/proc_fs.h>    // proc_create(), remove_proc_entry()
#include <linux/seq_file.h>   // seq_printf(), seq_read(), seq_lseek()
#include <asm/io.h>           // ioremap(), iounmap()
#include <asm/desc.h>         // struct desc_ptr, struct gate_desc, store_idt()


MODULE_DESCRIPTION("Interrupt Descriptor Table viewer.");
MODULE_AUTHOR("Nikita Edward Baruzdin <nebaruzdin@gmail.com>");
MODULE_LICENSE("GPL");

#define mod_info(fmt, ...) \
	printk(KERN_INFO "%s: " fmt, MODNAME, ##__VA_ARGS__)

#define MODNAME             "idt"
#define IDT_PROC_FILENAME   "idt"

struct desc_ptr dtr;
int num_idt_entries;
static gate_desc *idt;


static void idt_setup(struct seq_file *sf)
{
	store_idt(&dtr); // This internally uses assembly instruction 'sidt'
	                 // to store the contents of the IDTR register in dtr.
	#ifdef CONFIG_X86_64
		#define IDT_ENTRY_BYTES 16
	#else
		#define IDT_ENTRY_BYTES 8
	#endif
	num_idt_entries = (dtr.size + 1) / IDT_ENTRY_BYTES;
	idt = (gate_desc *)dtr.address;

	seq_printf(sf, "\nIDT    Size: %d bytes / %d entries    "
	               "Virt address: 0x%lX\n",
	               dtr.size + 1, num_idt_entries, dtr.address);
}

static const char *idt_print_type(unsigned int type)
{
	char *type_str;
	switch (type) {
		case GATE_INTERRUPT:
			type_str = "interrupt";
			break;
		case GATE_TRAP:
			type_str = "trap     ";
			break;
		case GATE_TASK:
			type_str = "task     ";
			break;

		default:
			type_str = "other    ";
	}
	return type_str;
}

static int idt_show(struct seq_file *sf, void *v)
{
	int entry;

	idt_setup(sf);

	// Display detailed IDT entries.
	#ifdef CONFIG_X86_64
	seq_printf(sf, "\n      HEX                              "
	               "TYPE      DPL P IST SEGM OFFSET");
	#else
	seq_printf(sf, "\n      HEX              "
	               "TYPE      DPL P SEGM OFFSET");
	#endif
	for (entry = 0; entry < num_idt_entries; entry++) {
		seq_printf(sf, "\n0x%02X:", entry);
		#ifdef CONFIG_X86_64
		seq_printf(sf, " %016llX%016llX %s %X   %c %X   %04X "
		               "%08X%04X%04X",
		               *((uint64_t *)&idt[entry] + 1),
		               *(uint64_t *)&idt[entry],
		               idt_print_type(idt[entry].type),
		               idt[entry].dpl,
		               idt[entry].p ? '+' : '-',
		               idt[entry].ist,
		               idt[entry].segment,
		               idt[entry].offset_high,
		               idt[entry].offset_middle,
		               idt[entry].offset_low);
		#else
		seq_printf(sf, " %08X%08X %s %X   %c %04X %08X",
		               idt[entry].b, idt[entry].a,
		               idt_print_type(idt[entry].type),
		               idt[entry].dpl,
		               idt[entry].p ? '+' : '-',
		               idt[entry].base0,
		               (idt[entry].b & 0xFFFF0000) + idt[entry].limit0);
		#endif
	}
	seq_printf(sf, "\n\n");

	return 0;
}

static int idt_open(struct inode *inode, struct file *file)
{
	return single_open(file, idt_show, NULL);
}

static const struct file_operations idt_fops = {
	.owner   = THIS_MODULE,
	.open    = idt_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = single_release,
};

static int __init idt_init(void)
{
	mod_info("Loading.");

	proc_create(IDT_PROC_FILENAME, 0, NULL, &idt_fops);

	return 0;
}

static void __exit idt_exit(void)
{
	mod_info("Unloading.");

	remove_proc_entry(IDT_PROC_FILENAME, NULL);
}

module_init(idt_init);
module_exit(idt_exit);
