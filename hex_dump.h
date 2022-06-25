#ifndef HEX_DUMP_H
#define HEX_DUMP_H

#ifdef __cplusplus
extern "C" {
#endif
void hex_dump(const char *function_name, const int line_number, const char *title, const void *mem, const int len);
#ifdef __cplusplus
}
#endif
#endif // HEX_DUMP_H
