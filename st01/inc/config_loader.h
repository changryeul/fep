#ifndef		__CONFIG_LOADER_H
#define		__CONFIG_LOADER_H
/*------------------------------------------------------------------------
#	Module	: config loader abstraction layer
#	File	: config_loader.h
#	Description	: Provides Config_Load_All() as unified entry point.
#				  Selects DB or INI based on _FEP_DB_MODE env var.
#				  AUTO mode: DB first, .ini fallback on failure.
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Load Mode (determined by _FEP_DB_MODE environment variable)
------------------------------------------------------------------------*/
typedef enum {
	CFG_MODE_AUTO,		/* DB first, .ini fallback on failure (default)	*/
	CFG_MODE_DB,		/* DB only, no fallback							*/
	CFG_MODE_INI		/* .ini only, existing behavior					*/
} CFG_LOAD_MODE;

/*------------------------------------------------------------------------
	Load Source (actual source used for loading)
------------------------------------------------------------------------*/
typedef enum {
	CFG_SRC_NONE,		/* not loaded yet								*/
	CFG_SRC_DB,			/* loaded from SQLite DB							*/
	CFG_SRC_INI			/* loaded from .ini files						*/
} CFG_LOAD_SOURCE;

/*------------------------------------------------------------------------
	Load Result
------------------------------------------------------------------------*/
typedef struct {
	CFG_LOAD_SOURCE	source;			/* actual source used				*/
	int				total_keys;		/* number of config keys loaded		*/
	int				db_ok;			/* DB load success (1/0)			*/
	int				ini_ok;			/* INI fallback used (1/0)			*/
	char			loaded_at[20];	/* load time (HH:MM:SS)				*/
	char			env_id[10];		/* environment ID					*/
} CFG_LOAD_RESULT;

/*------------------------------------------------------------------------
	Main API
------------------------------------------------------------------------*/

/*
 * Load all configuration (main entry point)
 * Called from pz_memory_proc.c Shm_Conf_Process()
 *
 * @param flag  same meaning as *_Config_Read(flag)
 *              0: full init (all daemons)
 *              1: current daemon only
 * @return CFG_LOAD_RESULT
 *
 * Behavior:
 *   1. Check _FEP_DB_MODE env var -> determine CFG_LOAD_MODE
 *   2. AUTO mode: try cfg_db_open()
 *      - success -> load all from DB -> sync to .ini
 *      - failure -> .ini fallback (existing *_Config_Read)
 *   3. Record result in CFG_LOAD_RESULT
 */
extern CFG_LOAD_RESULT	Config_Load_All (int flag);

/*
 * Reload specific config group
 * Called from px_cfgload
 *
 * @param group_id  "daemon", "proc", "tcp1", "tcp2", "udpip", etc.
 * @param flag      load flag
 * @return CFG_LOAD_RESULT
 */
extern CFG_LOAD_RESULT	Config_Reload_Group (const char *group_id, int flag);

/*
 * Get current load mode
 */
extern CFG_LOAD_MODE	Config_Get_Mode (void);

/*
 * Get last load result
 */
extern CFG_LOAD_RESULT	Config_Get_Last_Result (void);

/*************************************************************************
	End of Program (config_loader.h)
*************************************************************************/
#endif
