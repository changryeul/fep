/*------------------------------------------------------------------------
#   vexch_catalog.c — 가상거래소 상품 카탈로그 파서 (공유)
------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vexch_catalog.h"

VX_PRODUCT vx_cat[VX_MAX_PRODUCT];
int        vx_cat_cnt = 0;

static void vx_trim(char *s)
{
    char *t;
    for (t = s; *t && *t != '\n' && *t != '\r' && *t != '#'; t++) ;
    *t = 0;
    for (t = s + strlen(s) - 1; t >= s && (*t == ' ' || *t == '\t'); t--) *t = 0;
    while (*s == ' ' || *s == '\t') memmove(s, s + 1, strlen(s));
}

static void vx_set(VX_PRODUCT *p, const char *k, const char *v)
{
    if      (!strcmp(k, "market"))      strncpy(p->market, v, 1);
    else if (!strcmp(k, "order_tr"))    strncpy(p->order_tr, v, 15);
    else if (!strcmp(k, "order_size"))  p->order_size = atoi(v);
    else if (!strcmp(k, "resp_tr"))     strncpy(p->resp_tr, v, 15);
    else if (!strcmp(k, "resp_size"))   p->resp_size = atoi(v);
    else if (!strcmp(k, "exec_tr"))     strncpy(p->exec_tr, v, 15);
    else if (!strcmp(k, "exec_size"))   p->exec_size = atoi(v);
    else if (!strcmp(k, "wrapper"))     strncpy(p->wrapper, v, 19);
    else if (!strcmp(k, "fill_rule"))   strncpy(p->fill_rule, v, 11);
    else if (!strcmp(k, "enabled"))     p->enabled = (v[0] == 'Y' || v[0] == 'y');
    else if (!strcmp(k, "sise_kind"))   strncpy(p->sise_kind, v, 7);
    else if (!strcmp(k, "sise_ip"))     strncpy(p->sise_ip, v, 19);
    else if (!strcmp(k, "sise_port"))   p->sise_port = atoi(v);
    else if (!strcmp(k, "sise_symbol")) strncpy(p->sise_symbol, v, 11);
    else if (!strcmp(k, "sise_excode")) strncpy(p->sise_excode, v, 1);
    else if (!strcmp(k, "sise_tr"))     strncpy(p->sise_tr, v, 15);
    else if (!strcmp(k, "sise_size"))   p->sise_size = atoi(v);
}

int vx_load_catalog(const char *path, void (*log_fn)(const char *))
{
    FILE *f = fopen(path, "r");
    char  line[256], msg[320], *e;
    VX_PRODUCT *cur = NULL;

    vx_cat_cnt = 0;
    if (!f) {
        snprintf(msg, sizeof(msg), "vexch_catalog: cannot open [%s]", path);
        if (log_fn) log_fn(msg); else printf("%s\n", msg);
        return (-1);
    }
    while (fgets(line, sizeof(line), f)) {
        char *s = line;
        while (*s == ' ' || *s == '\t') s++;
        if (*s == '#' || *s == '\n' || *s == '\r' || *s == 0) continue;
        if (*s == '[') {
            if (vx_cat_cnt < VX_MAX_PRODUCT) {
                cur = &vx_cat[vx_cat_cnt++];
                memset(cur, 0, sizeof(*cur));
                strcpy(cur->wrapper, "TCHTDP00000");
                strcpy(cur->sise_kind, "none");
                sscanf(s, "[%31[^]]", cur->name);
            }
            continue;
        }
        e = strchr(s, '=');
        if (!e || !cur) continue;
        *e = 0;
        { char key[64], val[128]; strncpy(key, s, 63); key[63]=0; strncpy(val, e+1, 127); val[127]=0;
          vx_trim(key); vx_trim(val); vx_set(cur, key, val); }
    }
    fclose(f);
    snprintf(msg, sizeof(msg), "vexch_catalog: loaded %d products from %s", vx_cat_cnt, path);
    if (log_fn) log_fn(msg); else printf("%s\n", msg);
    return (vx_cat_cnt);
}
