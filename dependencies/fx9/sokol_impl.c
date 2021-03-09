#include <stdio.h>

#define SOKOL_LOG(s) do { fprintf(stderr, "%s\n", s); } while (0)

#include "GL/gl3w.h"
#include "sokol_app.h"
#include "sokol_gfx.h"

