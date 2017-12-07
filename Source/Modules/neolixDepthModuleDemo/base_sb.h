#ifndef __BASE_H__
#define __BASE_H__
//首先定义两个辅助宏
#define   PRINT_MACRO_HELPER(x)   #x 
#define   PRINT_MACRO(x)   #x"sb="PRINT_MACRO_HELPER(x) 

#define   USE_TY_SDK 0
#define   USE_PMD_MARS04_SDK 1
//#define CHOSEN_SDK USE_TY_SDK
//#define CHOSEN_SDK USE_PMD_MARSO4_SDK
#define CHOSEN_SDK 1
#if CHOSEN_SDK == USE_PMD_MARS04_SDK 
#pragma message(PRINT_MACRO(CHOSEN_SDK))
#endif
#endif
