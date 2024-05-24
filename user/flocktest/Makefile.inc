# -*-Makefile-*-

OBJDIRS		+= $(USER_OBJDIR)/flocktest

USER_FLOCKTEST_SRC	+= $(USER_DIR)/flocktest/flocktest.c
USER_FLOCKTEST_OBJ	:= $(patsubst %.c, $(OBJDIR)/%.o, $(USER_FLOCKTEST_SRC))
USER_FLOCKTEST_OBJ	:= $(patsubst %.S, $(OBJDIR)/%.o, $(USER_FLOCKTEST_OBJ))
KERN_BINFILES	+= $(USER_OBJDIR)/flocktest/flocktest

USER_FLOCKDEMO_SRC	+= $(USER_DIR)/flocktest/flockdemo.c
USER_FLOCKDEMO_OBJ	:= $(patsubst %.c, $(OBJDIR)/%.o, $(USER_FLOCKDEMO_SRC))
USER_FLOCKDEMO_OBJ	:= $(patsubst %.S, $(OBJDIR)/%.o, $(USER_FLOCKDEMO_OBJ))
KERN_BINFILES	+= $(USER_OBJDIR)/flocktest/flockdemo

USER_FLOCKREADER_SRC	+= $(USER_DIR)/flocktest/flockreader.c
USER_FLOCKREADER_OBJ	:= $(patsubst %.c, $(OBJDIR)/%.o, $(USER_FLOCKREADER_SRC))
USER_FLOCKREADER_OBJ	:= $(patsubst %.S, $(OBJDIR)/%.o, $(USER_FLOCKREADER_OBJ))
KERN_BINFILES	+= $(USER_OBJDIR)/flocktest/flockreader

USER_FLOCKWRITER_SRC	+= $(USER_DIR)/flocktest/flockwriter.c
USER_FLOCKWRITER_OBJ	:= $(patsubst %.c, $(OBJDIR)/%.o, $(USER_FLOCKWRITER_SRC))
USER_FLOCKWRITER_OBJ	:= $(patsubst %.S, $(OBJDIR)/%.o, $(USER_FLOCKWRITER_OBJ))
KERN_BINFILES	+= $(USER_OBJDIR)/flocktest/flockwriter

USER_FLOCKSTALL_SRC	+= $(USER_DIR)/flocktest/flockstall.c
USER_FLOCKSTALL_OBJ	:= $(patsubst %.c, $(OBJDIR)/%.o, $(USER_FLOCKSTALL_SRC))
USER_FLOCKSTALL_OBJ	:= $(patsubst %.S, $(OBJDIR)/%.o, $(USER_FLOCKSTALL_OBJ))
KERN_BINFILES	+= $(USER_OBJDIR)/flocktest/flockstall

flocktest: $(USER_OBJDIR)/flocktest/flocktest $(USER_OBJDIR)/flocktest/flockdemo $(USER_OBJDIR)/flocktest/flockreader $(USER_OBJDIR)/flocktest/flockwriter $(USER_OBJDIR)/flocktest/flockstall

$(USER_OBJDIR)/flocktest/flocktest: $(USER_LIB_OBJ) $(USER_FLOCKTEST_OBJ)
	@echo + ld[USER/flocktest] $@
	$(V)$(LD) -o $@ $(USER_LDFLAGS) $(USER_LIB_OBJ) $(USER_FLOCKTEST_OBJ) $(GCC_LIBS)
	mv $@ $@.bak
	$(V)$(OBJCOPY) --remove-section .note.gnu.property $@.bak $@
	$(V)$(OBJDUMP) -S $@ > $@.asm
	$(V)$(NM) -n $@ > $@.sym

$(USER_OBJDIR)/flocktest/flockdemo: $(USER_LIB_OBJ) $(USER_FLOCKDEMO_OBJ)
	@echo + ld[USER/flocktest] $@
	$(V)$(LD) -o $@ $(USER_LDFLAGS) $(USER_LIB_OBJ) $(USER_FLOCKDEMO_OBJ) $(GCC_LIBS)
	mv $@ $@.bak
	$(V)$(OBJCOPY) --remove-section .note.gnu.property $@.bak $@
	$(V)$(OBJDUMP) -S $@ > $@.asm
	$(V)$(NM) -n $@ > $@.sym

$(USER_OBJDIR)/flocktest/flockreader: $(USER_LIB_OBJ) $(USER_FLOCKREADER_OBJ)
	@echo + ld[USER/flocktest] $@
	$(V)$(LD) -o $@ $(USER_LDFLAGS) $(USER_LIB_OBJ) $(USER_FLOCKREADER_OBJ) $(GCC_LIBS)
	mv $@ $@.bak
	$(V)$(OBJCOPY) --remove-section .note.gnu.property $@.bak $@
	$(V)$(OBJDUMP) -S $@ > $@.asm
	$(V)$(NM) -n $@ > $@.sym

$(USER_OBJDIR)/flocktest/flockwriter: $(USER_LIB_OBJ) $(USER_FLOCKWRITER_OBJ)
	@echo + ld[USER/flocktest] $@
	$(V)$(LD) -o $@ $(USER_LDFLAGS) $(USER_LIB_OBJ) $(USER_FLOCKWRITER_OBJ) $(GCC_LIBS)
	mv $@ $@.bak
	$(V)$(OBJCOPY) --remove-section .note.gnu.property $@.bak $@
	$(V)$(OBJDUMP) -S $@ > $@.asm
	$(V)$(NM) -n $@ > $@.sym

$(USER_OBJDIR)/flocktest/flockstall: $(USER_LIB_OBJ) $(USER_FLOCKSTALL_OBJ)
	@echo + ld[USER/flocktest] $@
	$(V)$(LD) -o $@ $(USER_LDFLAGS) $(USER_LIB_OBJ) $(USER_FLOCKSTALL_OBJ) $(GCC_LIBS)
	mv $@ $@.bak
	$(V)$(OBJCOPY) --remove-section .note.gnu.property $@.bak $@
	$(V)$(OBJDUMP) -S $@ > $@.asm
	$(V)$(NM) -n $@ > $@.sym

$(USER_OBJDIR)/flocktest/%.o: $(USER_DIR)/flocktest/%.c
	@echo + cc[USER/flocktest] $<
	@mkdir -p $(@D)
	$(V)$(CC) $(USER_CFLAGS) -c -o $@ $<

$(USER_OBJDIR)/flocktest/%.o: $(USER_DIR)/flocktest/%.S
	@echo + as[USER/flocktest] $<
	@mkdir -p $(@D)
	$(V)$(CC) $(USER_CFLAGS) -c -o $@ $<