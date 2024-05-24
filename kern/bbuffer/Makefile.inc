# -*- Makefile -*- 

OBJDIRS += $(KERN_OBJDIR)/bbuffer

KERN_SRCFILES += $(KERN_DIR)/bbuffer/bbuffer.c

$(KERN_OBJDIR)/bbuffer/%.o: $(KERN_DIR)/bbuffer/%.c
	@echo + $(COMP_NAME)[KERN/bbuffer] $<
	@mkdir -p $(@D)
	$(V)$(CCOMP) $(CCOMP_KERN_CFLAGS) -c -o $@ $<

$(KERN_OBJDIR)/bbuffer/%.o: $(KERN_DIR)/bbuffer/%.S
	@echo + as[KERN/bbuffer] $<
	@mkdir -p $(@D)
	$(V)$(CC) $(KERN_CFLAGS) -c -o $@ $<