# =========================================================== 
# 为C/C++项目设计的通用版Makefile
#  
# 介绍:  
# ---------------------------------------------------------- 
# 1. 将Makefile拷贝到工程目录下.  
# 2. Customize in the "Customizable Section" only if necessary:  
#    * 目前工程只支持二级目录 
#    * 找到SRCDIRS下所有SOURCE_SUFFIXES后缀源文件编译成.o
#    * 过滤掉目录EXCLUDE_DIRS 
#    * 如果要编出bin lib dll文件,分别填写BIN_TARG LIB_TARG DLL_TARG
#    * 编出目标文件额外依赖的库文件填写在USERLIBS  
# 3. make工程.  
#  
# Make目标:  
# ----------------------------------------------------------  
#   The Makefile provides the following targets to make:  
#   $ make              编译连接
#   $ make DEP=yes      生成依赖文件,使用依赖文件编译
#   $ make bin          生成可执行文件
#   $ make lib          生成.a静态库文件
#   $ make dll          生成.so动态库文件  
#   $ make clean        清理[.o] [.d] [bin] [lib] [dll]文件
#   $ make help         介绍MakeFile的使用
#   $ make show         展示Makefile中的变量    
# ===========================================================


# 函数
##===========================================================
# define可以看做是宏定义,使用$(get_sub_dir)相当于将这块内容放到对应处
# define get_sub_dir
# 	@find $1 -path $2 -a -prune -o -type d -print
# endef

# get_sub_dir = $(shell find $1 -path $2 -a -prune -o -type d -print)

#---------------------------------------------#
#  获取$1下的子目录,过滤掉$2                  #
#  $1 -- 上级目录                             #
#  $2 -- 被过滤掉的子目录                     #
#---------------------------------------------#
# 调用是$(call get_sub_dir, 参数1, 参数2)
get_sub_dir = $(foreach d, $(filter-out $(2), $(wildcard $1/*)), $(shell if [ -d $(d) ]; then echo $(d); fi))
##===========================================================

##===========================================================
# ?=不会覆盖存在的值,:=是直接覆盖                                             
GCC             := gcc  
GXX             := g++  
LD              := ld  
AR              := ar 
##===========================================================

# 指定后缀
##===========================================================
SOURCE_SUFFIXES  = c C cc cpp CPP c++ cxx cp
OBJECT_SUFFIX    = o
DEPEND_SUFFIX    = d
##===========================================================

# 目录
##===========================================================
BIN              = $(MKHOME)/bin
LIB              = $(MKHOME)/lib
SRC              = $(MKHOME)/src
INCLUDE          = $(MKHOME)/include
INSTALL          = /usr/local
##----------------------------------------------------------
SRCDIRS         := $(CURDIR)

# SUBSRCDIRS      := $(foreach d, $(wildcard $(SRCDIRS)/*), $(shell if [ -d "$d" -a $(EXCLUDE_DIRS) != $(notdir $(d)) ]; then echo -n "$d"; fi)) 
SUBSRCDIRS      := $(call get_sub_dir, $(CURDIR), $(EXCLUDE_DIRS))
SRCDIRS         += $(SUBSRCDIRS) $(ADD_DIRS)
##===========================================================

# 文件
##===========================================================
ALLFILES         = $(foreach d, $(SRCDIRS), $(wildcard $(addprefix $(d)/*., $(SOURCE_SUFFIXES))))
ALLOBJS          = $(addsuffix .o, $(basename $(ALLFILES)))  
ALLDEPS          = $(ALLOBJS:.o=.d)  
##===========================================================

# 编译连接参数,目标文件,依赖的.o文件
##===========================================================
CFLAGS      = -w -g -fPIC
CXXFLAGS    = -w -g -fPIC
##----------------------------------------------------------	
BIN_FLAGS  	= -g -Wall 	
BIN_OBJS    = $(ALLOBJS)

##----------------------------------------------------------
LIB_FLAGS   = -rc
LIB_OBJS    = $(ALLOBJS)

##----------------------------------------------------------
DLL_FLAGS   = -shared -g
DLL_OBJS    = $(ALLOBJS)

##===========================================================

# 定义头文件路径,连接库路径,需要连接的其他库文件
##===========================================================

USERINC     = -I$(MKHOME)/include
USERLD      = -L./ -L$(LIB) 
##----------------------------------------------------------


