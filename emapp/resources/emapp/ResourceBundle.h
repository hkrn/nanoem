#ifndef NANOEM_EMAPP_RESOURCE_H_
#define NANOEM_EMAPP_RESOURCE_H_

#include "emapp/Forward.h"

struct ImFont;
struct ImFontAtlas;

namespace nanoem {

class DefaultTranslator;
class Image;
class Project;

namespace resources {

void initializeSharedToonTextures(Image **textures, ByteArray *bytes);
void initializeSharedToonColors(Vector4 *colors);
ImFont *initializeTextFont(ImFontAtlas *fontAtlas, nanoem_f32_t pointSize, void *ranges);
ImFont *initializeIconFont(ImFontAtlas *fontAtlas, nanoem_f32_t pointSize);
void getCredits(const nanoem_u8_t *&text, size_t &size);

bool loadBuiltInTranslation(DefaultTranslator *translator);

} /* namespace resources */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_RESOURCE_H_ */
