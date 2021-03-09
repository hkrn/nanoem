#include <stdio.h>
#include <string.h>

#include "lz4/lib/lz4hc.h"
#include "protobuf-c/protobuf-c.h"
#include "protoc/common.pb-c.h"
#include "protoc/translation.pb-c.h"
#include "yaml-cpp/yaml.h"

#include <string>

namespace {

static Nanoem__Translation__Phrase *
packPhrase(YAML::Node node, const char *locale)
{
    YAML::Node phraseNode = node["phrase"];
    const std::string &key = node["key"].as<std::string>();
    const std::string &text = phraseNode[locale].as<std::string>();
    Nanoem__Translation__Phrase *phrase = new Nanoem__Translation__Phrase;
    nanoem__translation__phrase__init(phrase);
    phrase->id = strdup(key.c_str());
    phrase->text = strdup(text.c_str());
    return phrase;
}

static Nanoem__Translation__Unit *
packUnit(YAML::Node rootNode, enum _Nanoem__Common__Language language, const char *locale)
{
    size_t numNodes = rootNode.size();
    Nanoem__Translation__Unit *unit = new Nanoem__Translation__Unit;
    nanoem__translation__unit__init(unit);
    unit->language = language;
    unit->n_phrases = numNodes;
    unit->phrases = new Nanoem__Translation__Phrase *[numNodes];
    for (size_t i = 0; i < numNodes; i++) {
        YAML::Node node = rootNode[i];
        unit->phrases[i] = packPhrase(node, locale);
    }
    return unit;
}

static void
deleteUnit(Nanoem__Translation__Unit *unit)
{
    for (size_t i = 0, numPhrases = unit->n_phrases; i < numPhrases; i++) {
        Nanoem__Translation__Phrase *phrase = unit->phrases[i];
        free(phrase->id);
        free(phrase->text);
        delete phrase;
    }
    delete[] unit->phrases;
    delete unit;
}

static void
deleteBundle(Nanoem__Translation__Bundle *bundle)
{
    for (size_t i = 0, numUnits = bundle->n_units; i < numUnits; i++) {
        deleteUnit(bundle->units[i]);
    }
    delete[] bundle->units;
    delete bundle;
}

} /* namespace anonymous */

int
main(int argc, char *argv[])
{
    if (argc == 3) {
        const char *input = argv[1];
        const char *output = argv[2];
        YAML::Node rootNode = YAML::LoadFile(input);
        Nanoem__Translation__Bundle *bundle = new Nanoem__Translation__Bundle;
        nanoem__translation__bundle__init(bundle);
        bundle->n_units = 2;
        bundle->units = new Nanoem__Translation__Unit *[2];
        bundle->units[0] = packUnit(rootNode, NANOEM__COMMON__LANGUAGE__LC_JAPANESE, "ja_JP");
        bundle->units[1] = packUnit(rootNode, NANOEM__COMMON__LANGUAGE__LC_ENGLISH, "en_US");
        size_t size = nanoem__translation__bundle__get_packed_size(bundle);
        uint8_t *buffer = new uint8_t[size];
        nanoem__translation__bundle__pack(bundle, buffer);
        if (FILE *fp = fopen(output, "wb")) {
            fwrite(buffer, size, 1, fp);
            fclose(fp);
        }
        deleteBundle(bundle);
        delete[] buffer;
    }
    return 0;
}
