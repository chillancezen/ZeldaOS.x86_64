#  ZeldaOS.x86_64

`The 64bit version of ZeldaOS which is still written in C&GAS from scratch for study use. Its main goal this time is to support SMP and virtualization. it's able to run a game: tetris inside guest OS`
[![asciicast](https://asciinema.org/a/5qJG94cGhMy1M0PTzHGqruEGS.svg)](https://asciinema.org/a/5qJG94cGhMy1M0PTzHGqruEGS)

### how to build and run it?

It's handy to run the demo within the root of the repository on a nested-virtualization enabled Linux host([how to enable it?](https://github.com/chillancezen/ZeldaOS.x86_64/issues/2#issuecomment-495082106)):

`#KVM='--enable-kvm --cpu host' ZELDA64=/path/to/repo CC=gcc-4.8 make run`

to clean the build, run:


`#KVM='--enable-kvm --cpu host' ZELDA64=/path/to/repo CC=gcc-4.8 make clean`
## Features Inventory

### Arch:x86_64 features
- [X] running in 64-bit long mode 
- [X] SMP boot orchestration.
- [X] 64-bit IDT management
- [X] 64-bit segmentation/GDT management
- [X] Inter-processors Interrupt(IPI) management

### Memory features
- [X] Customed physical memory detection
- [X] Level 4 paging(2MB pages and 4K pages)

### Devices features
- [X] Basic 16-color video buffer managment
- [X] APIC(Advanced PIC) management(LAPIC and IOAPIC)
- [X] Basic serial port output management


### Kernel features 
- [X] cusomized bootloader(only support legacy BIOS boot)
- [X] PXE boot supported with https://github.com/goupaz/pxeboot
- [X] Synchronization primitives: spinlock...
- [X] Processor-local storage and per-cpu framework.
- [X] Kernel PANIC.

### VM Monitor
- [X] Basic vmx mode switch(realmode, protected mode and 64-bit long mode)
- [X] Memory virtualization(with Intel VT-x EPT mechanism)
- [ ] APIC virtualization(in progress)
- [X] IO emulation(PIO and MMIO)
- [X] IO devices(serial port, video and keyboard), details can be found from [wiki](https://wiki.osdev.org/VMX#Peripheral_Emulation)
- [ ] SMP virtualization(in progress)



### screenshot of the demononstration.
The Game: Tetris running as guest OS:
![image of green window](https://raw.githubusercontent.com/chillancezen/DEPRECATED-misc/master/image/teris.png)

ZeldaOS64 provides a normal video window to display some runtime information(TBC):
![image of green window](https://raw.githubusercontent.com/chillancezen/DEPRECATED-misc/master/image/main.png)

also we have pretty beautiful screen of death(like BSOD from Microsoft Windows, It's derived from VMware ESXi vmkernel: Purple Screen of Death:PSOD)

![sreen of death](https://raw.githubusercontent.com/chillancezen/DEPRECATED-misc/master/image/purple-window.png)

the output of serial port is streamed like:
![output of serial port](https://raw.githubusercontent.com/chillancezen/DEPRECATED-misc/master/image/splash-serial-output.png)
