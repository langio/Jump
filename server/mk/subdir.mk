SUBDIRS += 
RECURSION_SUBDIRS += 
SUBMAKEFILES += 
GOAL ?=
#-------------------------------------------------------------------------

SUBDIRS += $(foreach D,$(RECURSION_SUBDIRS),$(shell find $(D) -mindepth 1 -maxdepth 1 -type d 2> /dev/null))

define make_subdir
	@for subdir in $(SUBDIRS); do \
	(cd $$subdir && $(MAKE) $1) \
	done;
endef

define make_submakefile
	@for file in $(SUBMAKEFILES); do \
	($(MAKE) $1 -f $$file) \
	done;
endef

.PHONY : all clean

all:
	+$(call make_subdir,$(GOAL))
	+$(call make_submakefile,$(GOAL))

$(MAKECMDGOALS):
	+$(call make_subdir,$(MAKECMDGOALS))
	+$(call make_submakefile,$(MAKECMDGOALS))

