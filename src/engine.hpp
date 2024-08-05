#pragma once

using PrintFn_t = void(const char *format, ...);

extern PrintFn_t *Print;
extern PrintFn_t *Warn;
