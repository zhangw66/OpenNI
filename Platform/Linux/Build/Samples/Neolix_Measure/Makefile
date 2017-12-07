include ../../Common/CommonDefs.mak

BIN_DIR = ../../../Bin

INC_DIRS = ../../../../../Include \
			../../../../../Samples/Neolix_Measure/neolixAlgorithm_latest \
			../../../../../Samples/Neolix_Measure/neolixAlgorithm_latest/common 

SRC_FILES = ../../../../../Samples/Neolix_Measure/*.cpp \
			#../../../../../Samples/Neolix_Measure/neolixai/config/*.cpp \
			#../../../../../Samples/Neolix_Measure/neolixai/common/*.cpp \
			#../../../../../Samples/Neolix_Measure/neolixai/driverdecorate/*.cpp \
			../../../../../Samples/Neolix_Measure/neolixai/imagepro/*.cpp\
			../../../../../Samples/Neolix_Measure/neolixai/measure/*.cpp\
			../../../../../Samples/Neolix_Measure/neolixai/pointcloud/*.cpp
	

EXE_NAME = Neolix_Measure

ifeq ("$(OSTYPE)","Darwin")
	LDFLAGS += -framework OpenGL -framework GLUT
else
	USED_LIBS += glut GL
endif

CFLAGS += -DUSE_CUBING_ALGORITHM
USED_LIBS += OpenNI \
			 neolixCubingV2 \
			 opencv_core\
			 opencv_highgui\
			 opencv_imgproc
LIB_DIRS = ../../../../../Samples/Neolix_Measure/neolixAlgorithm_latest/lib\
		   /usr/local/lib        
include ../../Common/CommonCppMakefile

