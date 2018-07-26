# 目录
##===========================================================
EXCLUDE_DIRS    := $(CURDIR)/stub_entity
##===========================================================

include $(MKHOME)/mak/mk_stand_prefix.mak

# 编译连接参数,目标文件,依赖的.o文件
##===========================================================
BIN_TARG    = $(BIN)/tool_gson
##----------------------------------------------------------
LIB_TARG    = 
##----------------------------------------------------------
DLL_TARG    = 
##===========================================================

# 定义头文件路径,连接库路径,需要连接的其他库文件
##===========================================================
USERLIBS    = -lacl_all $(SYSLIB)
##----------------------------------------------------------
INCPATH     = -I/usr/include $(ACLINC) $(USERINC)
LDPATH      = -L/usr/local/lib $(ACLLD) $(USERLD)
##===========================================================

# 编译连接参数
##===========================================================
CFLAGS     := -c -g -W -Wall -Wcast-qual -Wcast-align -Wno-long-long -Wpointer-arith -Werror -Wshadow -O3 -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -D_USE_FAST_MACRO
CXXFLAGS   := -c -g -W -Wall -Wcast-qual -Wcast-align -Wno-long-long -Wpointer-arith -Werror -Wshadow -O3 -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -D_USE_FAST_MACRO
##----------------------------------------------------------	

# 检查系统: Linux, SunOS, Solaris, BSD variants, AIX, HP-UX
SYSLIB      = -lpthread -lz
UNIXNAME    = $(shell uname -sm)

# For FreeBSD
ifeq ($(findstring FreeBSD, $(UNIXNAME)), FreeBSD)
	CFLAGS += -Wstrict-prototypes -DFREEBSD -D_REENTRANT
	CXXFLAGS += -DFREEBSD -D_REENTRANT
	
	SYSLIB = -lcrypt -lpthread
endif

# For Darwin
ifeq ($(findstring Darwin, $(UNIXNAME)), Darwin)
	CFLAGS += -DMACOSX -Wno-invalid-source-encoding -Wno-extended-offsetof
	CXXFLAGS += -DMACOSX -Wno-invalid-source-encoding -Wno-extended-offsetof
	
	SYSLIB +=  -liconv -rdynamic
endif

# For Linux
ifeq ($(findstring Linux, $(UNIXNAME)), Linux)
	CFLAGS += -Wstrict-prototypes -DLINUX2 -D_REENTRANT
	CXXFLAGS += -DLINUX2 -D_REENTRANT
	
	SYSLIB += -lcrypt
endif

# For MINGW
ifeq ($(findstring MINGW, $(UNIXNAME)), MINGW)
	CFLAGS += -DLINUX2 -DMINGW
	CXXFLAGS += -DLINUX2 -DMINGW
	
	SYSLIB = -lpthread-2 -liconv
endif

# For MSYS
ifeq ($(findstring MSYS, $(UNIXNAME)), MSYS)
	CFLAGS += -DLINUX2 -DMINGW
	CXXFLAGS += -DLINUX2 -DMINGW
	
	SYSLIB = -lpthread-2 -liconv
endif

# For SunOS
ifeq ($(findstring SunOS, $(UNIXNAME)), SunOS)
	CFLAGS += -Wstrict-prototypes -DSUNOS5 -D_REENTRANT
	CXXFLAGS += -DSUNOS5 -D_REENTRANT	
	
	ifeq ($(findstring 86, $(UNIXNAME)), 86)
		SYSLIB += -lsocket -lnsl -lrt
	endif
	ifeq ($(findstring sun4u, $(UNIXNAME)), sun4u)
		SYSLIB += -lsocket -lnsl -lrt
	endif	
endif

# For HP-UX
ifeq ($(findstring HP-UX, $(UNIXNAME)), HP-UX)
	CFLAGS += -Wstrict-prototypes -DHP_UX -DHPUX11
	CXXFLAGS += -DHP_UX -DHPUX11	
endif

##===========================================================

include $(MKHOME)/mak/mk_stand_suffix.mak