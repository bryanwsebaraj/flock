# -*-Makefile-*-

OBJDIRS += $(KERN_OBJDIR)/dev/sse

KERN_SRCFILES += $(KERN_DIR)/dev/sse/aes.c

$(KERN_OBJDIR)/dev/sse/%.o: $(KERN_DIR)/dev/sse/%.c
	@echo + cc[KERN/dev/sse] $<
	@mkdir -p $(@D)
	$(V)$(CC) $(KERN_CFLAGS) -maes -msse -c -o $@ $<
	