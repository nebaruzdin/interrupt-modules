# interrupt-modules

This is a set of Linux kernel modules that, with the help of pseudo-files in
procfs, allow to view hardware and internal kernel data, that can be helpful in
researching and debugging interrupts-related issues.

All modules work for both: x86 and x86_64 architectures. They were tested on
Linux kernel version 4.4.5.

## mp

Creates /proc/mp file that shows the contents of the Intel Multiprocessor
Specification data structures.

Particularly, it can be used to determine the number of IO-APICs in the system
and their base addresses. (Note that you can also use ACPI tables for the same
purpose.) For instance:

		$ cat /proc/mp
		...
		Base MP Configuration Table:
		...
		0x104: 02 00 20 01 00 00 C0 FE
		0x10C: 02 02 20 01 00 10 C0 FE
		...

Here the first "02" byte identifies the IO-APIC entry. Last 4 bytes contain the
IO-APIC base address in little-endian. So in this case the system has two
IO-APICs with base addresses 0xFEC00000 and 0xFEC01000.

For details see:
- [Intel Multiprocessor Specification. Version 1.4][1]
- http://wiki.osdev.org/SMP#Finding_information_using_MP_Table

## ioapic

By default creates /proc/ioapic0 file that shows the contents of the IO-APIC
interrupt routing table. Supports an optional second IO-APIC: if the second
IO-APIC's base address is provided, the /proc/ioapic1 file is also created.
IO-APICs' base addresses are set through module parameters:

		# insmod ioapic.ko ioapic0_base=0xFEC00000 ioapic1_base=0xFEC01000

Thus you can simultaneously view the contents of any two of the system IO-APICs.

For details see:
- [82093AA I/O Advanced Programmable Interrupt Controller (IOAPIC)][2]
- http://wiki.osdev.org/IOAPIC

## idt

Creates /proc/idt file that shows the contents of the Interrupt Descriptor
Table (IDT).

Each rule from the IO-APIC's routing table can contain a number, known as an
interrupt vector (IV), that refers to a specific entry in IDT. IDT entry, in
turn, contains the location of an interrupt service routine (ISR) that will be
executed for the given interrupt.

For details see:
- [Intel 64 and IA-32 Architectures Software Developer’s Manual.
Volume 3A: System Programming Guide, Part 1][3]
- http://wiki.osdev.org/Interrupt_Descriptor_Table

[1]: http://download.intel.com/design/archives/processors/pro/docs/24201606.pdf
[2]: http://download.intel.com/design/chipsets/datashts/29056601.pdf
[3]: http://download.intel.com/design/processor/manuals/253668.pdf
