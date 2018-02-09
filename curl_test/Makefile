ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif
include $(DEVKITARM)/3ds_rules

.SUFFIXES:
# don't pollute my VPATH
VPATH		:=

.PHONY: clean outdir

.DEFAULT_GOAL	:=	out/fgh.3dsx

APP_TITLE	:=	fgh

ARCH		:=	-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft

COMNFLAGS	:=	$(ARCH) \
				-g -O2 -fdiagnostics-color=always -D_GNU_SOURCE -Wall -Wextra -pedantic \
				-fomit-frame-pointer -ffunction-sections -fdata-sections \
				-isystem$(CTRULIB)/include $(foreach dir,$(PORTLIBS),-isystem$(dir)/include)
CXXFLAGS	:=	$(COMNFLAGS) -std=c++14 -fno-rtti -fno-exceptions -fno-use-cxa-atexit
CFLAGS		:=	$(COMNFLAGS) -std=c11

CFILES		:=	main.c
CXXFILES	:=

OBJFILES	:=	$(patsubst %,obj/%.o,$(CFILES) $(CXXFILES))
LIBPATHS	:=	-L$(CTRULIB)/lib $(foreach dir,$(PORTLIBS),-L$(dir)/lib)

LD			:=	$(CC)
ASFLAGS		:=	-g $(ARCH)
LDFLAGS		=	-specs=3dsx.specs -g $(ARCH) -Wl,-Map,$@.map

obj/%.c.o: src/%.c
	@echo $^ =\> $@
	@mkdir -p $(dir $@)
	@$(CC) -MMD -MP -MF obj/$*.c.d $(CFLAGS) -c $< -o $@ $(ERROR_FILTER)

outdir:
	@mkdir -p out

out/fgh.3dsx: _3DSXFLAGS += --smdh=out/fgh.smdh
out/fgh.3dsx: out/fgh.smdh

out/fgh.elf: LIBS := -lcurl -lmbedtls -lmbedx509 -lmbedcrypto -lz -lctru
out/fgh.elf: OFILES := $(OBJFILES)
out/fgh.elf: $(OBJFILES) outdir

clean:
	@rm -vrf obj out

-include $(patsubst %,obj/%.d,$(CFILES) $(CXXFILES))
