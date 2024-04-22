#ifndef VARIORUM_ANNOTATION_H_INCLUDE
#define VARIORUM_ANNOTATION_H_INCLUDE
#include <stdio.h>
///@brief store the caller function's file, line and name.
#define CALL_INFO __FILE__, __LINE__, __func__
///@brief macro wrapper for actual functions, needed to store the call info.
#define VARIORUM_ANNOTATE_GET_POWER_INFO                                       \
  variorum_annotate_function_call(VARIORUM_POWER_CALL, CALL_INFO)
///@brief macro wrapper for actual functions, needed to store the call info.
#define VARIORUM_ANNOTATE_GET_NODE_POWER_DOMAIN_INFO                           \
  variorum_annotate_function_call(VARIORUM_DOMAIN_CALL, CALL_INFO)
#define VARIORUM_ANNOTATE_GET_ENERGY_INFO                                      \
  variorum_annotate_function_call(VARIORUM_ENERGY_CALL, CALL_INFO)
typedef enum {
  VARIORUM_POWER_CALL,
  VARIORUM_ENERGY_CALL,
  VARIORUM_DOMAIN_CALL,
} variorum_call_type_t;

void variorum_annotate_function_call(variorum_call_type_t type, const char *calling_file,
                            int line, const char *function_name);

void variorum_annotate_init();

void variorum_annotate_finalize();
#endif
