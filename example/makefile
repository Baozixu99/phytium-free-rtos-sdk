# Compiling all examples
TOPTARGETS := all clean

#SUBDIRS := $(wildcard */.)
SUBDIRS := function_test \
		   hello_world \
		   start_up \
		   template \
		   lwip_test

$(TOPTARGETS): $(SUBDIRS)
	@for i in $(SUBDIRS); do \
	echo -e "\033[44;37;1m +++ Compiling $$i...\033[0m"; \
	$(MAKE) -C $$i $(MAKECMDGOALS) > /dev/null || exit $$?; \
	echo -e "\033[44;37;1m +++ Compiled OK $$i...\033[0m"; \
	done

.PHONY: $(TOPTARGETS) $(SUBDIRS)