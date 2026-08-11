#include "log.h"
#include "slp.h"

int SLP_PARAM_Print( SLP_PARAM* ptr)
{
    LogRaw( "%s", "----[ SLP_PARAM ]-----------------------------------------------------------------------\n");
    LogRaw( "shared memory, semaphore key  key                    4    0 = [0x%08x]\n", 	ptr->key);
    LogRaw( "max data count                max_rec                4    4 = [%d]\n", 	ptr->max_rec);
    LogRaw( "data record size              rec_size               4    8 = [%d]\n", 	ptr->rec_size);
    LogRaw( "max index count               max_idx                4   12 = [%d]\n", 	ptr->max_idx);
    LogRaw( "index size                    idx_size               4   16 = [%d]\n", 	ptr->idx_size);
    LogRaw( "status size                   stat_size              4   20 = [%d]\n", 	ptr->stat_size);
    LogRaw( "shared memory total size      size                   8   24 = [%ld]\n", 	ptr->size);
    LogRaw( "%s", "-----------------------------------------------------------------------[ SLP_PARAM ]----\n");

    return sizeof( SLP_PARAM);
}

int SLP_STATUS_Print( SLP_STATUS* ptr)
{
    LogRaw( "%s", "----[ SLP_STATUS ]----------------------------------------------------------------------\n");
	SLP_PARAM_Print( &ptr->param);
    LogRaw( "index add position            idx_pos                4    0 = [%d]\n", 	ptr->idx_pos);
    LogRaw( "index count                   idx_cnt                4    4 = [%d]\n", 	ptr->idx_cnt);
    LogRaw( "data add position             dat_pos                4    8 = [%d]\n", 	ptr->dat_pos);
    LogRaw( "data count                    dat_cnt                4   12 = [%d]\n", 	ptr->dat_cnt);
    LogRaw( "%s", "----------------------------------------------------------------------[ SLP_STATUS ]----\n");

    return sizeof( SLP_STATUS);
}

int SLP_PAIR_Print( SLP_PAIR* ptr)
{
    LogRaw( "%s", "----[ SLP_PAIR ]------------------------------------------------------------------------\n");
    LogRaw( "                              num                    4    0 = [%d]\n", 	ptr->num);
    LogRaw( "                              base                   4    4 = [%d]\n", 	ptr->base);
    LogRaw( "                              cont                   4    8 = [%d]\n", 	ptr->cont);
    LogRaw( "                              point                  4   12 = [%d]\n", 	ptr->point);
    LogRaw( "                              unit                   4   16 = [%d]\n", 	ptr->unit);
    LogRaw( "                              symbol                 8   20 = [%.8s]\n", 	ptr->symbol);
    LogRaw( "                              comment               64   28 = [%.64s]\n", 	ptr->comment);
    LogRaw( "%s", "------------------------------------------------------------------------[ SLP_PAIR ]----\n");

    return sizeof( SLP_PAIR);
}

int SLP_INDEX_Print( SLP_INDEX* ptr)
{
    LogRaw( "%s", "----[ SLP_INDEX ]-----------------------------------------------------------------------\n");
    LogRaw( "current index position        pos                    4    0 = [%d]\n", 	ptr->pos);
    LogRaw( "index start position          start                  4    4 = [%d]\n", 	ptr->start);
    LogRaw( "index member count            cnt                    4    8 = [%d]\n", 	ptr->cnt);
    SLP_PAIR_Print( &ptr->pair);
    LogRaw( "%s", "-----------------------------------------------------------------------[ SLP_INDEX ]----\n");

    return sizeof( SLP_INDEX);
}

int SLP_HEAD_Print( SLP_HEAD* ptr)
{
    LogRaw( "%s", "----[ SLP_HEAD ]------------------------------------------------------------------------\n");
    LogRaw( "0-empty, 1-order, 2-execute,  stat                   4    0 = [%d]\n", 	ptr->stat);
    LogRaw( "prev data position start=0    prev                   4    4 = [%d]\n", 	ptr->prev);
    LogRaw( "next data position end=0      next                   4    8 = [%d]\n", 	ptr->next);
    LogRaw( "%s", "------------------------------------------------------------------------[ SLP_HEAD ]----\n");

    return sizeof( SLP_HEAD);
}

int SLP_DATA_Print( SLP_DATA* ptr)
{
    LogRaw( "%s", "----[ SLP_DATA ]------------------------------------------------------------------------\n");
    LogRaw( "                              data                 512    0 = [%.512s]\n", 	ptr->data);
    LogRaw( "%s", "------------------------------------------------------------------------[ SLP_DATA ]----\n");

    return sizeof( SLP_DATA);
}

int SLP_RECORD_Print( SLP_RECORD* ptr)
{
    LogRaw( "%s", "----[ SLP_RECORD ]----------------------------------------------------------------------\n");
    LogRaw( "%s", "----------------------------------------------------------------------[ SLP_RECORD ]----\n");

    return sizeof( SLP_RECORD);
}

int SLP_Print( SLP* ptr)
{
    LogRaw( "%s", "----[ SLP ]-----------------------------------------------------------------------------\n");
    LogRaw( "%s", "-----------------------------------------------------------------------------[ SLP ]----\n");

    return sizeof( SLP);
}

