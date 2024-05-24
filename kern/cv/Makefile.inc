# -*- Makefile -*-

OBJDIRS += $(KERN_OBJDIR)/cv

KERN_SRCFILES += $(KERN_DIR)/cv/cv.c

$(KERN_OBJDIR)/cv/%.o: $(KERN_DIR)/cv/%.c
	@echo + $(COMP_NAME)[KERN/cv] $<
	@mkdir -p $(@D)
	$(V)$(CCOMP) $(CCOMP_KERN_CFLAGS) -c -o $@ $<

$(KERN_OBJDIR)/cv/%.o: $(KERN_DIR)/cv/%.S
	@echo + as[KERN/cv] $<
	@mkdir -p $(@D)
	$(V)$(CC) $(KERN_CFLAGS) -c -o $@ $<