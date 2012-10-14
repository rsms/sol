#include "value.h"

const SValue SValueNil   = {SValueTNil,   { .p = 0 }};
const SValue SValueTrue  = {SValueTTrue,  { .n = 1 }};
const SValue SValueFalse = {SValueTFalse, { .n = 0 }};

char* SValueRepr(char* buf, size_t bufsize, SValue* v) {
  switch (v->type) {
  case SValueTNil:    return memcpy(buf, "nil", bufsize);
  case SValueTTrue:   return memcpy(buf, "true", bufsize);
  case SValueTFalse:  return memcpy(buf, "false", bufsize);
  case SValueTNumber: {
    snprintf(buf, bufsize, SNumberFormat, v->value.n);
    return buf;
  }
  default: return memcpy(buf, "(?)", bufsize);
  }
}
