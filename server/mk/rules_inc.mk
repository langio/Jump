#包含文件需要指定下面几个参数
TARGET ?= 
SRC_DIR_RECURSION ?= Y
SRC_FILE_DIRS += 
OTHER_SRC_FILES += 
INCLUDE += 
RECURSION_INCLUDE +=
LIBS += 


#make目标之前需要提前make的目录或makefile
NEED_MAKE_DEP ?= N
DEP_MAKE_SUBDIRS +=
RECURSION_DEP_MAKE_SUBDIRS +=
DEP_MAKE_MAKEFILES += 
#目标需要额外检测依赖的其它对象
TARGET_DEP_OBJS +=

#中间文件保存目录
BUILD_DIR ?=

#------------------------------------------------------------------------------------
COMPILER ?= g++
AR = ar
RM = rm -vrf

CFLAGS += -g -Wall -Wno-deprecated -pipe -fPIC -D_GNU_SOURCE -D_REENTRANT
#CFLAGS += -pg -Wextra -D_NEW_LIC

#SHARED_FLAGS = -shared -z defs -fPIC 
#SHARED_FLAGS += -shared -z defs 
SHARED_FLAGS += -shared

SHELL = /bin/bash

#------------------------------------------------------------------------------------

ifneq ($(strip $(SRC_DIR_RECURSION)),N)
	ALL_SOURCE_FILES += $(foreach D,$(SRC_FILE_DIRS),$(shell find $(D) -name "*.cpp" -o -name "*.cc" -o -name "*.c" 2> /dev/null))
else
	ALL_SOURCE_FILES += $(foreach D,$(SRC_FILE_DIRS),$(shell ls $(D)/*.cpp $(D)/*.cc $(D)/*.c 2> /dev/null))
endif

ALL_SOURCE_FILES += $(OTHER_SRC_FILES)

ifneq ($(strip $(BUILD_DIR)),)
	TMP_OBJS = $(patsubst %.cpp,%.o,$(patsubst %.cc,%.o, $(patsubst %.c,%.o, $(ALL_SOURCE_FILES))))
	ALL_OBJS += $(addprefix $(BUILD_DIR)/, $(notdir $(TMP_OBJS)))
else
	ALL_OBJS += $(patsubst %.cpp,%.o,$(patsubst %.cc,%.o, $(patsubst %.c,%.o, $(ALL_SOURCE_FILES))))
endif


INCLUDE += $(foreach D,$(RECURSION_INCLUDE),$(shell find $(D) -path '*/.svn' -prune -o -type d -exec echo "-I"{} \; 2> /dev/null))

CLEANOBJS += $(TARGET) $(ALL_OBJS) $(SRC_DEP_FILE)


ifneq ($(strip $(BUILD_DIR)),)
	CLEANOBJS += $(BUILDDIR_RULES_FILE) $(BUILD_DIR)
endif

DEP_MAKE_SUBDIRS += $(foreach D,$(RECURSION_DEP_MAKE_SUBDIRS),$(shell find $(D) -mindepth 1 -maxdepth 1 -type d 2> /dev/null))


define compile_src_file
    $(COMPILER) $(CFLAGS) $(INCLUDE) -c $1 -o $2
endef	

define create_src_dep_file
    echo "update $2 ..."; \
    set -e; rm -f $2; \
    DCONTENT=`$(COMPILER) -MM $(INCLUDE) $1 | sed "s,.*\.o[ ]*:,$(2:.d=.o) $2 :,g"`; \
    echo "$$DCONTENT" > $2; \
    echo "$$DCONTENT" | sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
                -e '/^$$/d' -e 's/$$/ :/' >> $2; \
    if [[ -z `cat $2` ]]; then rm -f $2; fi;
endef


define create_builddir_rules_file 
    echo "update builddir_rules_file.ruled ..."; \
    mkdir -p $(BUILD_DIR); rm -f $(BUILDDIR_RULES_FILE); \
    for srcfile in $(ALL_SOURCE_FILES); do \
        obj="$(BUILD_DIR)/"`basename $${srcfile%.*}`".o"; \
        echo "$$obj : $$srcfile" >> $(BUILDDIR_RULES_FILE); \
        echo -e '\t$$(call compile_src_file, $$<, $$@)' >> $(BUILDDIR_RULES_FILE); \
        objd="$(BUILD_DIR)/"`basename $${srcfile%.*}`".d"; \
        echo "$$objd : $$srcfile" >> $(BUILDDIR_RULES_FILE); \
        echo -e '\t@$$(call create_src_dep_file, $$<, $$@)' >> $(BUILDDIR_RULES_FILE); \
    done;
endef

define make_subdir
	@for subdir in $1; do \
        (cd $$subdir && $(MAKE) $2) \
	done;
endef

define make_submakefile
	@for file in $1; do \
        ($(MAKE) $2 -f $$file) \
	done;
endef

#------------------------------------------------------------------------------------

$(filter %.a,$(TARGET)) : $(ALL_OBJS) $(TARGET_DEP_OBJS)
	$(AR) rcs $@ $(ALL_OBJS) 
	ranlib $(TARGET)

$(filter %.so,$(TARGET)) : $(ALL_OBJS) $(TARGET_DEP_OBJS)
	$(COMPILER) $(CFLAGS) $(SHARED_FLAGS) $(ALL_OBJS) $(LIBS) -o $@
	
$(filter-out %.so %.a,$(TARGET)) : $(ALL_OBJS) $(TARGET_DEP_OBJS)
	$(COMPILER) $(CFLAGS) $(INCLUDE) $(ALL_OBJS) $(LIBS) -o $@

.PHONY: clean cleanall run unittest rununittest
clean:
	@$(RM) $(CLEANOBJS)

cleanall: clean
    ifneq ($(strip $(NEED_MAKE_DEP)),N)
	$(call make_subdir, $(DEP_MAKE_SUBDIRS), clean)
	$(call make_submakefile, $(DEP_MAKE_MAKEFILES), clean)
    endif

run: $(TARGET)
	$(call RUN_CMD)

unittest:
	+$(call make_subdir, $(UNITTEST_DIRS))
	+$(call make_submakefile, $(UNITTEST_MAKEFILES))

rununittest:
	+$(call make_subdir, $(UNITTEST_DIRS), run)
	+$(call make_submakefile, $(UNITTEST_MAKEFILES), run)

#------------------------------------------------------------------------------------
SRC_DEP_FILE = $(ALL_OBJS:.o=.d)

ifneq ($(strip $(MAKECMDGOALS)),clean)
ifneq ($(strip $(MAKECMDGOALS)),cleanall)
ifneq ($(strip $(MAKECMDGOALS)),unittest)
ifneq ($(strip $(MAKECMDGOALS)),rununittest)

#生成源文件依赖文件
ifneq ($(strip $(SRC_DEP_FILE)),)
-include $(SRC_DEP_FILE)
endif

#如果有指定编译中间目录，则需要提前生成相关编译命令文件
ifneq ($(strip $(BUILD_DIR)),)
BUILDDIR_RULES_FILE = $(BUILD_DIR)/builddir_rules_file.ruled
ifneq ($(strip $(BUILDDIR_RULES_FILE)),)
-include $(BUILDDIR_RULES_FILE)
endif
endif


#执行makefile依赖关系
MAKEFILE_DEP_FILE = .make.makefiled
ifneq ($(strip $(NEED_MAKE_DEP)),N)
ifneq ($(strip $(MAKEFILE_DEP_FILE)),)
-include $(MAKEFILE_DEP_FILE)
endif
endif

endif
endif
endif
endif

#------------------------------------------------------------------------------------
#源文件编译规则
%.o:%.cpp
	$(call compile_src_file, $<, $@)

%.o:%.cc
	$(call compile_src_file, $<, $@)

%.o:%.c
	$(call compile_src_file, $<, $@)

#源文件依赖文件生成规则
%.d: %.cpp
	@$(call create_src_dep_file, $<, $@)
	
%.d: %.cc
	@$(call create_src_dep_file, $<, $@)

%.d: %.c
	@$(call create_src_dep_file, $<, $@)

#如果有指定编译中间目录，生成相关编译命令文件规则
ifneq ($(strip $(BUILD_DIR)),)
$(BUILDDIR_RULES_FILE) : $(ALL_SOURCE_FILES)
	@$(call create_builddir_rules_file)
endif


#makefile依赖规则
$(MAKEFILE_DEP_FILE) :
	$(call make_subdir, $(DEP_MAKE_SUBDIRS), $(MAKECMDGOALS))
	$(call make_submakefile, $(DEP_MAKE_MAKEFILES), $(MAKECMDGOALS))


%.cpp :
	$(error ERROR: file "$@" not exist)

%.c :
	$(error ERROR: file "$@" not exist)

%.cc :
	$(error ERROR: file "$@" not exist)
