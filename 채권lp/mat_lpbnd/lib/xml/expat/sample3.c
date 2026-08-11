#include <expat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "log.h"

#ifdef XML_LARGE_SIZE
#if defined(XML_USE_MSC_EXTENSIONS) && _MSC_VER < 1400
#define XML_FMT_INT_MOD "I64"
#else
#define XML_FMT_INT_MOD "ll"
#endif
#else
#define XML_FMT_INT_MOD "l"
#endif

struct setting {
    const char *key;
    char *value;
} config[] = {
    {"board_height", NULL}, {"board_width", NULL}, {"maximum_highscores", NULL}
};

struct setting *current_setting;

int
key_cmp(void const *ld, void const *rd)
{
    struct setting const *const l = ld;
    struct setting const *const r = rd;
    return strcmp(l->key, r->key);
}

void XMLCALL
handler(void *userData, const XML_Char *s, int len)
{
    if(len == 0){
        return;
    }

    if(!current_setting){
        return;
    }

    char *value = malloc((len+1) * sizeof(XML_Char));
    strncpy(value, s, len);
    current_setting->value = value;
}

static void XMLCALL
startElement(void *userData, const char *name, const char **atts)
{
    struct setting key = { .key = name };
    current_setting = bsearch(&key, config, sizeof(config)/sizeof(config[0]), sizeof(config[0]), key_cmp);
}

static void XMLCALL
endElement(void *userData, const char *name)
{
    current_setting = NULL;
}

int
sample(int argc, char *argv[])
{
    char buf[65535];

    XML_Parser parser = XML_ParserCreate(NULL);

    int done;
    int depth = 0;

    XML_SetUserData(parser, &depth);
    XML_SetElementHandler(parser, startElement, endElement);
    XML_SetCharacterDataHandler(parser, handler);

    do {
        int len = (int)fgets(buf, 65535, stdin);
        done = len < strlen(buf);
        if (XML_Parse(parser, buf, len, done) == XML_STATUS_ERROR) {
            LogMsg(stderr,
                    "%s at line %" XML_FMT_INT_MOD "u\n",
                    XML_ErrorString(XML_GetErrorCode(parser)),
                    XML_GetCurrentLineNumber(parser));
            return 1;
        }
    } while (!done);

    XML_ParserFree(parser);

    int i;
    for (i = 0; i < (sizeof(config)/sizeof(config[0])); i++) {
        struct setting current = config[i];
        printf("%s: %s\n", current.key, current.value);
        free(current.value);
    }

    return 0;
}
