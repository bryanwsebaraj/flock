# -*-Makefile-*-

KERN_DIR	:= kern
KERN_OBJDIR	:= $(OBJDIR)/kern

OBJDIRS		+= $(KERN_OBJDIR)

DEBUG_MSG	:= 1

# Arch-independent compiling and linking options
KERN_CFLAGS	:= $(CFLAGS) -D_KERN_ -I$(KERN_DIR) -I$(KERN_DIR)/kern -I. -m32
ifdef ENABLE_CCOMP
CCOMP_KERN_CFLAGS := $(CCOMP_CFLAGS) -D_KERN_ -I$(KERN_DIR) -I$(KERN_DIR)/kern -I.
CLIGHTGEN_FLAGS   += -D_KERN_ -I$(KERN_DIR) -I$(KERN_DIR)/kern -I.
endif

ifdef DEBUG_MSG
KERN_CFLAGS	+= -O2 -g
else
KERN_CFLAGS	+= -O2
endif

KERN_LDFLAGS	:= $(LDFLAGS) -e start -m elf_i386 -Ttext=0x00100000
GCC_LIBS	:= $(GCC_LIB32)

# Sub-makefiles
include		$(KERN_DIR)/config.mk
include		$(KERN_DIR)/dev/Makefile.inc
include		$(KERN_DIR)/lib/Makefile.inc
include		$(KERN_DIR)/pcpu/Makefile.inc
include		$(KERN_DIR)/init/Makefile.inc
include		$(KERN_DIR)/pmm/Makefile.inc
include		$(KERN_DIR)/vmm/Makefile.inc
include		$(KERN_DIR)/thread/Makefile.inc
include		$(KERN_DIR)/proc/Makefile.inc
include		$(KERN_DIR)/trap/Makefile.inc
include		$(KERN_DIR)/fs/Makefile.inc
include		$(KERN_DIR)/cv/Makefile.inc
include     $(KERN_DIR)/flock/Makefile.inc
include     $(KERN_DIR)/bbuffer/Makefile.inc

KERN_CFLAGS	+= $(KERN_DEBUG_FLAGS)
KERN_CFLAGS	+= -DSERIAL_DEBUG -DDEBUG_MSG
ifdef ENABLE_CCOMP
CCOMP_KERN_CFLAGS += $(KERN_DEBUG_FLAGS)
CLIGHTGEN_FLAGS   += $(KERN_DEBUG_FLAGS)
endif

# Object files of kernel
KERN_SRCFILES	:= $(wildcard $(KERN_SRCFILES))
KERN_OBJFILES	:= $(patsubst %.c, $(OBJDIR)/%.o, $(KERN_SRCFILES))
KERN_OBJFILES	:= $(patsubst %.S, $(OBJDIR)/%.o, $(KERN_OBJFILES))

# Targets

kern: $(KERN_OBJFILES) $(KERN_BINFILES)
	@echo All targets of kernel are done.

link: $(KERN_OBJDIR)/kernel

# Link kernel
$(KERN_OBJDIR)/kernel: kern
	@echo + ld[KERN] $@
	$(V)$(LD) -o $@ $(KERN_LDFLAGS) $(KERN_OBJFILES) $(GCC_LIBS) -b binary $(KERN_BINFILES)
	mv $@ $@.bak
	$(V)$(OBJCOPY) --remove-section .note.gnu.property $@.bak $@
	$(V)$(OBJDUMP) -S $@ > $@.asm
	$(V)$(NM) -n $@ > $@.sym
