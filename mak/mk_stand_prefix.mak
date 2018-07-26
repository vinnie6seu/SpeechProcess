# =========================================================== 
# ΪC/C++��Ŀ��Ƶ�ͨ�ð�Makefile
#  
# ����:  
# ---------------------------------------------------------- 
# 1. ��Makefile����������Ŀ¼��.  
# 2. Customize in the "Customizable Section" only if necessary:  
#    * Ŀǰ����ֻ֧�ֶ���Ŀ¼ 
#    * �ҵ�SRCDIRS������SOURCE_SUFFIXES��׺Դ�ļ������.o
#    * ���˵�Ŀ¼EXCLUDE_DIRS 
#    * ���Ҫ���bin lib dll�ļ�,�ֱ���дBIN_TARG LIB_TARG DLL_TARG
#    * ���Ŀ���ļ����������Ŀ��ļ���д��USERLIBS  
# 3. make����.  
#  
# MakeĿ��:  
# ----------------------------------------------------------  
#   The Makefile provides the following targets to make:  
#   $ make              ��������
#   $ make DEP=yes      ���������ļ�,ʹ�������ļ�����
#   $ make bin          ���ɿ�ִ���ļ�
#   $ make lib          ����.a��̬���ļ�
#   $ make dll          ����.so��̬���ļ�  
#   $ make clean        ����[.o] [.d] [bin] [lib] [dll]�ļ�
#   $ make help         ����MakeFile��ʹ��
#   $ make show         չʾMakefile�еı���    
# ===========================================================


# ����
##===========================================================
# define���Կ����Ǻ궨��,ʹ��$(get_sub_dir)�൱�ڽ�������ݷŵ���Ӧ��
# define get_sub_dir
# 	@find $1 -path $2 -a -prune -o -type d -print
# endef

# get_sub_dir = $(shell find $1 -path $2 -a -prune -o -type d -print)

#---------------------------------------------#
#  ��ȡ$1�µ���Ŀ¼,���˵�$2                  #
#  $1 -- �ϼ�Ŀ¼                             #
#  $2 -- �����˵�����Ŀ¼                     #
#---------------------------------------------#
# ������$(call get_sub_dir, ����1, ����2)
get_sub_dir = $(foreach d, $(filter-out $(2), $(wildcard $1/*)), $(shell if [ -d $(d) ]; then echo $(d); fi))
##===========================================================

##===========================================================
# ?=���Ḳ�Ǵ��ڵ�ֵ,:=��ֱ�Ӹ���                                             
GCC             := gcc  
GXX             := g++  
LD              := ld  
AR              := ar 
##===========================================================

# ָ����׺
##===========================================================
SOURCE_SUFFIXES  = c C cc cpp CPP c++ cxx cp
OBJECT_SUFFIX    = o
DEPEND_SUFFIX    = d
##===========================================================

# Ŀ¼
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

# �ļ�
##===========================================================
ALLFILES         = $(foreach d, $(SRCDIRS), $(wildcard $(addprefix $(d)/*., $(SOURCE_SUFFIXES))))
ALLOBJS          = $(addsuffix .o, $(basename $(ALLFILES)))  
ALLDEPS          = $(ALLOBJS:.o=.d)  
##===========================================================

# �������Ӳ���,Ŀ���ļ�,������.o�ļ�
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

# ����ͷ�ļ�·��,���ӿ�·��,��Ҫ���ӵ��������ļ�
##===========================================================

USERINC     = -I$(MKHOME)/include
USERLD      = -L./ -L$(LIB) 
##----------------------------------------------------------


