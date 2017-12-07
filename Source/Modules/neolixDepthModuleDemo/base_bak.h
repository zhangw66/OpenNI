#if 0
#ifndef __BASE_H__
#define __BASE_H__
//首先定义两个辅助宏
#define   PRINT_MACRO_HELPER(x)   #x 
#define   PRINT_MACRO(x)   #x"sb="PRINT_MACRO_HELPER(x) 

#define   USE_TY_SDK 0
#define   USE_PMD_MARS04_SDK 1
//#define CHOSEN_SDK USE_TY_SDK
#define CHOSEN_SDK USE_PMD_MARSO4_SDK
//#define CHOSEN_SDK 1
#pragma message(PRINT_MACRO(CHOSEN_SDK))
#pragma message(PRINT_MACRO(USE_TY_SDK))
#endif
#endif
#if 1
#pragma once
//首先定义两个辅助宏
#define   PRINT_MACRO_HELPER(x)   #x 
#define   PRINT_MACRO(x)   #x"sb="PRINT_MACRO_HELPER(x) 
#define   TEST_TY			6
#define   USE_TY_SDK 			0
#define   USE_PMD_MARS1_SDK 		2
//#define   CHOSEN_SDK TEST_TY
#define   CHOSEN_SDK USE_PMD_MARS1_SDK
//#define   CHOSEN_SDK USE_TY_SDK
#pragma message(PRINT_MACRO(CHOSEN_SDK))
#pragma message(PRINT_MACRO(USE_TY_SDK))
#else
#pragma once
#define   PRINT_MACRO_HELPER(x)   #x 
#define   PRINT_MACRO(x)   #x"ok="PRINT_MACRO_HELPER(x) 
#define TEST_SUNNYMARS	5
#define TEST_TY			6
#define TEST_CASE TEST_SUNNYMARS
#pragma message(PRINT_MACRO(TEST_SUNNYMARS))
#pragma message(PRINT_MACRO(TEST_CASE))
#endif
