# -*- Makefile -*-

OBJDIRS += $(KERN_OBJDIR)/flock

KERN_SRCFILES += $(KERN_DIR)/flock/flock.c

$(KERN_OBJDIR)/flock/%.o: $(KERN_DIR)/flock/%.c
	@echo + $(COMP_NAME)[KERN/flock] $<
	@mkdir -p $(@D)
	$(V)$(CCOMP) $(CCOMP_KERN_CFLAGS) -c -o $@ $<

$(KERN_OBJDIR)/flock/%.o: $(KERN_DIR)/flock/%.S
	@echo + as[KERN/flock] $<
	@mkdir -p $(@D)
	$(V)$(CC) $(KERN_CFLAGS) -c -o $@ $<