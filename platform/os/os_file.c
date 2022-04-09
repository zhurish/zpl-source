/*
 * os_util.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"


int os_file_access(zpl_char *filename)
{
	if( access(filename, F_OK) != 0)
		return ERROR;
	return OK;	
}

int os_write_file(const zpl_char *name, const zpl_char *string, zpl_uint32 len)
{
	FILE *fp = fopen(name, "w+");
	if(fp)
	{
		//fprintf(fp, "%s\n", string);
		fwrite(string, len, 1, fp);
		fflush(fp);
		fclose(fp);
		return OK;
	}
	return ERROR;
}

int os_read_file(const zpl_char *name, const zpl_char *string, zpl_uint32 len)
{
	FILE *fp = fopen(name, "r");
	if(fp)
	{
		fread(string, len, 1, fp);
		fclose(fp);
		return OK;
	}
	return ERROR;
}



int os_mkdir(const zpl_char *dirpath, zpl_uint32 mode, zpl_uint32 pathflag)
{
	int ret = 0;
	zpl_char tmp[128];
	zpl_char *p = NULL;
	static zpl_char cupwdtmp[128];
	static zpl_uchar cupflag = 0;
	if(cupflag == 0)
	{
		cupflag = 1;
		memset (cupwdtmp, '\0', sizeof(cupwdtmp));
		getcwd(cupwdtmp, sizeof(cupwdtmp));
	}
	if (strlen (dirpath) == 0 || dirpath == NULL)
	{
		cupflag = 0;
		printf ("strlen(dir) is 0 or dir is NULL.\n");
		return -1;
	}
	if(pathflag == 0)
	{
		if( access(dirpath, F_OK) != 0)
		{
			ret = mkdir(dirpath, mode?mode:(S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH));
			chdir (cupwdtmp);
			cupflag = 0;
			return ret;
		}
		cupflag = 0;
		return -1;
	}
	memset (tmp, '\0', sizeof(tmp));
	strncpy (tmp, dirpath, strlen (dirpath));
	if (tmp[0] == '/')
		p = strchr (tmp + 1, '/');
	else
		p = strchr (tmp, '/');

	if (p)
	{
		*p = '\0';
		//printf("===============%s=========0======%s\r\n", __func__, tmp);
		if( access(tmp, F_OK) != 0)
		{
			if(mkdir(tmp, mode?mode:(S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) == 0)
			{
				chdir (tmp);
			}
		}
		else
			chdir (tmp);
	}
	else
	{
		//printf("===============%s=========1======%s\r\n", __func__, tmp);
		if( access(tmp, F_OK) != 0)
		{
			if(mkdir(tmp, mode?mode:(S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) == 0)
			{
				chdir (tmp);
			}
		}
		chdir (cupwdtmp);
		cupflag = 0;
		return 0;
	}
	return os_mkdir (p + 1, mode, pathflag);
}


int os_rmdir(const zpl_char *dirpath, zpl_uint32 pathflag)
{
	if (strlen (dirpath) == 0 || dirpath == NULL)
	{
		printf ("strlen(dir) is 0 or dir is NULL.\n");
		return -1;
	}

	zpl_char tmp[128];
	memset (tmp, '\0', sizeof(tmp));
	snprintf (tmp, sizeof(tmp), "rm %s %s", pathflag?"-rf":" ",dirpath);
	if( access(dirpath, F_OK) == 0)
	{
		system(tmp);
		if( access(dirpath, F_OK) != 0)
			return -1;
		return 0;
	}
	return 0;
}

int os_getpwddir(const zpl_char *path, zpl_uint32 pathsize)
{
	if(getcwd(path, pathsize))
		return OK;
	return ERROR;
}

int os_file_size (const zpl_char *filename)
{
#if 0
	if(!filename)
		return ERROR;
	int filesize = -1;
	FILE *fp = NULL;
	fp = fopen(filename, "r");
	if(fp == NULL)
		return filesize;
	fseek(fp, 0L, SEEK_END);
	filesize = ftell(fp);
	fclose(fp);
	return filesize;
#else
	struct stat fsize;
	if(!filename)
		return ERROR;
	if (access (filename, F_OK) >= 0)
	{
		memset (&fsize, 0, sizeof(struct stat));
		if(stat (filename, &fsize) == 0)
			return fsize.st_size;
	}
#endif
	return ERROR;
}

#define KB_SIZE_MASK	(0X000003FF)

const zpl_char * os_file_size_string(zpl_ullong len)
{
	zpl_ullong glen = 0, mlen = 0, klen = 0, tlen = 0;
	static zpl_char buf[64];
	memset(buf, 0, sizeof(buf));
	tlen = (len >> 40) & KB_SIZE_MASK;
	glen = (len >> 30) & KB_SIZE_MASK;
	mlen = (len >> 20) & KB_SIZE_MASK;
	klen = (len >> 10) & KB_SIZE_MASK;
	if(tlen > 0)
	{
		snprintf(buf, sizeof(buf), "%d.%02d T",tlen, glen);
	}
	else if(glen > 0)
	{
		snprintf(buf, sizeof(buf), "%d.%02d G",glen, mlen);
	}
	else if(mlen > 0)
	{
		snprintf(buf, sizeof(buf), "%d.%02d M",mlen, klen);
	}
	else if(klen > 0)
	{
		snprintf(buf, sizeof(buf), "%u.%02u K",klen, (len) & KB_SIZE_MASK);
	}
	else
	{
		snprintf(buf, sizeof(buf), "%d Byte",len);
	}
	return buf;
}

/*const zpl_char * os_stream_size(long long len)
{
	return os_file_size(len);
}*/

#undef KB_SIZE_MASK


