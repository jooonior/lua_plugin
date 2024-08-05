#pragma once

using PrintFn_t = void(const char *format, ...);

inline PrintFn_t *Print;
inline PrintFn_t *Warn;
