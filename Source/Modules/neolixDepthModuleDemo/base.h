#ifndef __BASE_H__
#define __BASE_H__
//首先定义两个辅助宏
#define   PRINT_MACRO_HELPER(x)   #x 
#define   PRINT_MACRO(x)   #x"sb="PRINT_MACRO_HELPER(x) 

#define   USE_TY_SDK 0
#define   USE_PMD_MARS04_SDK 1
//#define CHOSEN_SDK USE_TY_SDK
#define CHOSEN_SDK USE_PMD_MARS04_SDK
//#define CHOSEN_SDK 1
#if CHOSEN_SDK == USE_PMD_MARS04_SDK
#define SUPPORTED_X_RES 224
#define SUPPORTED_Y_RES 172
#define SUPPORTED_FPS 5
#define MAX_DEPTH_VALUE	15000
#define NEOLIX_CAPABILITY_SET_USE_CASE					"set use case"
#define NEOLIX_CAPABILITY_SET_SCENE					    "set scene"

#endif
#pragma message(PRINT_MACRO(CHOSEN_SDK))
#pragma message(PRINT_MACRO(USE_PMD_MARS04_SDK))
//#endif
#endif
