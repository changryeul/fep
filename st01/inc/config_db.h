#ifndef		__CONFIG_DB_H
#define		__CONFIG_DB_H
/*------------------------------------------------------------------------
#	Module	: SQLite DB access layer for FEP configuration
#	File	: config_db.h
#	Description	: DB-based config loading as alternative to .ini files.
#				  Each cfg_db_load_*() writes directly to SHM,
#				  producing the same result as *_Config_Read().
------------------------------------------------------------------------*/

#include	<sqlite3.h>

/*------------------------------------------------------------------------
	Return Codes
------------------------------------------------------------------------*/
#define		CFG_DB_OK			 0		/* success						*/
#define		CFG_DB_ERR_OPEN		-1		/* DB file open failure			*/
#define		CFG_DB_ERR_QUERY	-2		/* SQL query execution failure	*/
#define		CFG_DB_ERR_NODATA	-3		/* required data not found		*/
#define		CFG_DB_ERR_TYPE		-4		/* data type validation failure	*/
#define		CFG_DB_ERR_ENCRYPT	-5		/* encrypt/decrypt failure		*/

/*------------------------------------------------------------------------
	DB Connection Management
------------------------------------------------------------------------*/

/*
 * Open DB file
 * @param db_path  DB file path (NULL = $_FEP_CFG/fep_config.db)
 * @return CFG_DB_OK or CFG_DB_ERR_OPEN
 */
extern int		cfg_db_open (const char *db_path);

/*
 * Close DB file
 */
extern void		cfg_db_close (void);

/*
 * Check DB connection status
 * @return 1(connected) or 0(not connected)
 */
extern int		cfg_db_is_open (void);

/*------------------------------------------------------------------------
	Environment Detection
------------------------------------------------------------------------*/

/*
 * Detect current environment ID based on hostname
 * @return env_id string ("TEST", "REAL1", "REAL2")
 *
 * Rules:
 *   hostname == "podm11" -> "REAL1"
 *   hostname == "podm12" -> "REAL2"
 *   otherwise            -> "TEST"
 */
extern const char*	cfg_db_detect_env (void);

/*------------------------------------------------------------------------
	Config Load Functions (DB -> SHM)
	Each function produces the same SHM result as *_Config_Read(flag)
------------------------------------------------------------------------*/

/*
 * Load daemon.ini config from DB -> INFO()/DAEMON() structs
 * @param flag  0:INFO only, 1:INFO+DAEMON, 2:temp buffer, 3:reload
 * @return CFG_DB_OK or error code
 */
extern int		cfg_db_load_daemon (int flag);

/*
 * Load file.ini config from DB -> FILEM() structs
 * @param flag  0:all, 1:current daemon only
 * @return CFG_DB_OK or error code
 */
extern int		cfg_db_load_file (int flag);

/*
 * Load dshm.ini config from DB -> DSHM() structs
 */
extern int		cfg_db_load_dshm (int flag);

/*
 * Load tcp1.ini config from DB -> TCP1() structs
 */
extern int		cfg_db_load_tcp1 (int flag);

/*
 * Load tcp2.ini config from DB -> TCP2() structs
 */
extern int		cfg_db_load_tcp2 (int flag);

/*
 * Load udpip.ini config from DB -> UDPIP() structs
 */
extern int		cfg_db_load_udpip (int flag);

/*
 * Load sisetr.ini config from DB -> SISETR() structs
 */
extern int		cfg_db_load_sisetr (int flag);

/*
 * Load proc.ini config from DB -> PROC() structs
 * LogonPW is auto-decrypted
 */
extern int		cfg_db_load_proc (int flag);

/*
 * Load client.ini + pc.ini IP whitelist from DB
 * @param ip_list  result array (caller allocates)
 * @param max_cnt  max array size
 * @param ip_type  "CLIENT" or "ADMIN"
 * @return loaded IP count or error code (negative)
 */
extern int		cfg_db_load_ip_whitelist (char ip_list[][20], int max_cnt,
					const char *ip_type);

/*------------------------------------------------------------------------
	Config Save Functions (value save + auto history via trigger)
------------------------------------------------------------------------*/

/*
 * Save single config value (UPDATE)
 * fep_config_history auto-records via DB trigger
 * @return CFG_DB_OK or error code
 */
extern int		cfg_db_set_value (const char *env_id, const char *group_id,
					const char *section, const char *key_name,
					const char *new_value, const char *changed_by);

/*------------------------------------------------------------------------
	DB -> .ini Sync (Fallback synchronization)
------------------------------------------------------------------------*/

/*
 * Export specific group's DB config to .ini file
 * @param group_id  target group ("daemon", "proc", ...)
 * @param ini_path  output .ini file path
 * @return CFG_DB_OK or error code
 */
extern int		cfg_db_export_ini (const char *group_id,
					const char *ini_path);

/*
 * Export all groups to .ini (fallback sync)
 */
extern int		cfg_db_sync_all_ini (void);

/*------------------------------------------------------------------------
	Password Encryption
------------------------------------------------------------------------*/

/*
 * Encrypt LogonPW (plaintext -> AES-256-CBC)
 * @param plain    plaintext password
 * @param cipher   cipher output buffer (64 bytes min)
 * @return CFG_DB_OK or CFG_DB_ERR_ENCRYPT
 */
extern int		cfg_db_encrypt_pw (const char *plain, char *cipher);

/*
 * Decrypt LogonPW (AES-256-CBC -> plaintext)
 * @param cipher   ciphertext
 * @param plain    plaintext output buffer (20 bytes min)
 * @return CFG_DB_OK or CFG_DB_ERR_ENCRYPT
 */
extern int		cfg_db_decrypt_pw (const char *cipher, char *plain);

/*------------------------------------------------------------------------
	Utility Functions
------------------------------------------------------------------------*/

/*
 * Get DB config value as string
 * @param env_id    environment ID
 * @param group_id  config group
 * @param section   section name
 * @param key_name  key name
 * @param out_buf   output buffer
 * @param buf_len   buffer length
 * @return CFG_DB_OK or error code
 */
extern int		cfg_db_get_value (const char *env_id, const char *group_id,
					const char *section, const char *key_name,
					char *out_buf, int buf_len);

/*
 * Get DB config value as integer
 * @return integer value, or default_val on error
 */
extern int		cfg_db_get_int (const char *env_id, const char *group_id,
					const char *section, const char *key_name,
					int default_val);

/*
 * Get loaded key count for a specific group
 * @return key count or 0
 */
extern int		cfg_db_get_key_count (const char *env_id,
					const char *group_id);

/*************************************************************************
	End of Program (config_db.h)
*************************************************************************/
#endif
