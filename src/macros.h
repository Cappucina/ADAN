#ifndef MACROS_H
#define MACROS_H

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof((array)[0]))

#define IS_DEFINED(val)                                                                  \
	(val &&                                                                          \
	 (val->kind == IRV_CONST || val->kind == IRV_GLOBAL || val->kind == IRV_PARAM || \
	  val->kind == IRV_TEMP || val->kind == IRV_TEMP) &&                             \
	 (val->kind == IRV_CONST || val->kind == IRV_GLOBAL || val->kind == IRV_PARAM || \
	          val->kind == IRV_TEMP                                                  \
	      ? 1                                                                        \
	      : 0))

#endif