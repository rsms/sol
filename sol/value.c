#include "value.h"

const SValue SValueNil   = {{ .p = 0 }, SValueTNil};
const SValue SValueTrue  = {{ .n = 1 }, SValueTTrue};
const SValue SValueFalse = {{ .n = 0 }, SValueTFalse};

char* SValueRepr(char* buf, size_t bufsize, SValue* v) {
  switch (v->type) {
  
  case SValueTNil:    return memcpy(buf, "nil", bufsize);
  case SValueTTrue:   return memcpy(buf, "true", bufsize);
  case SValueTFalse:  return memcpy(buf, "false", bufsize);

  case SValueTNumber: {
    snprintf(buf, bufsize, SNumberFormat, v->value.n);
    return buf;
  }
  
  case SValueTFunc: {
    snprintf(buf, bufsize, "<func %p>", v->value.p);
    return buf;
  }
  
  default: return memcpy(buf, "(?)", bufsize);
  }
}
