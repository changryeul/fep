typedef struct _zbrs_bos_header_
{
	char			data_size[ 6];
	char			trcode[ 4];
	char			seq[ 9];
	char			block_cnt[ 2];
	char			err_code[ 2];
	char			frim_no[ 3];
	char			if_gu[ 6];
	char			time[ 12];
	char			compress[ 1];
	char			encrypt[ 1];
}	ZBRS_BOS_HEADER;

