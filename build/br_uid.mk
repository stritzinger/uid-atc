RTEMS_API = 4.12
RTEMS_CPU = powerpc
RTEMS_SHARE = $(RTEMS_ROOT)/share/rtems$(RTEMS_API)
PROJECT_ROOT = $(RTEMS_ROOT)/$(RTEMS_CPU)-rtems$(RTEMS_API)/$(RTEMS_BSP)
PROJECT_INCLUDE = $(PROJECT_ROOT)/lib/include
PROJECT_LIB = $(PROJECT_ROOT)/lib
BUILDDIR = b-$(RTEMS_BSP)

prefix = $(RTEMS_ROOT)
exec_prefix = $(RTEMS_ROOT)/$(RTEMS_CPU)-rtems$(RTEMS_API)

include $(RTEMS_ROOT)/make/custom/$(RTEMS_BSP).cfg

DEPFLAGS = -MT $@ -MD -MP -MF $(basename $@).d
SYSFLAGS = -B $(PROJECT_LIB) -specs bsp_specs -qrtems
WARNFLAGS = -Wall -Wextra -Wconversion -Wformat-security -Wformat=2 -Wshadow -Wcast-qual -Wcast-align -Wredundant-decls
CWARNFLAGS = $(WARNFLAGS) -Wstrict-prototypes -Wbad-function-cast
OPTFLAGS = $(CFLAGS_OPTIMIZE_V)

CFLAGS = $(DEPFLAGS) $(SYSFLAGS) $(CWARNFLAGS) $(CPU_CFLAGS) $(OPTFLAGS)
CXXFLAGS = $(DEPFLAGS) $(SYSFLAGS) $(WARNFLAGS) $(CPU_CFLAGS) $(OPTFLAGS)
LINKFLAGS = $(SYSFLAGS) $(CPU_CFLAGS) $(LDFLAGS) $(OPTFLAGS)
ASFLAGS = $(CPU_CFLAGS)

CCLINK = $(CC) $(LINKFLAGS) -Wl,-Map,$(basename $@).map
CXXLINK = $(CXX) $(LINKFLAGS) -Wl,-Map,$(basename $@).map

$(BUILDDIR)/%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: %.S
	$(CC) $(CPPFLAGS) -DASM $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

AR = $(RTEMS_ROOT)/bin/$(RTEMS_CPU)-rtems4.12-ar
AS = $(RTEMS_ROOT)/bin/$(RTEMS_CPU)-rtems4.12-as
CC = $(RTEMS_ROOT)/bin/$(RTEMS_CPU)-rtems4.12-gcc --pipe
CXX = $(RTEMS_ROOT)/bin/$(RTEMS_CPU)-rtems4.12-g++
LD = $(RTEMS_ROOT)/bin/$(RTEMS_CPU)-rtems4.12-ld
NM = $(RTEMS_ROOT)/bin/$(RTEMS_CPU)-rtems4.12-nm
OBJCOPY = $(RTEMS_ROOT)/bin/$(RTEMS_CPU)-rtems4.12-objcopy
RANLIB = $(RTEMS_ROOT)/bin/$(RTEMS_CPU)-rtems4.12-ranlib
SIZE = $(RTEMS_ROOT)/bin/$(RTEMS_CPU)-rtems4.12-size
STRIP = $(RTEMS_ROOT)/bin/$(RTEMS_CPU)-rtems4.12-strip
export AR
export AS
export CC
export CXX
export LD
export NM
export OBJCOPY
export RANLIB
export SIZE
export STRIP
