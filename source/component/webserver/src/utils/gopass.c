/*
    gopass.c -- Authorization password management.

    gopass [--cipher md5|blowfish] [--file filename] [--password password] realm user roles...

    This file provides facilities for creating passwords in a route configuration file. 
    It supports either MD5 or Blowfish ciphers.
 */

/********************************* Includes ***********************************/
#define HAS_BOOL 1
#include "goahead.h"
#include "webutil.h"
/********************************** Locals ************************************/

#define MAX_PASS    64

/********************************* Forwards ***********************************/
#if 0
static char *getPassword(void);
static void printUsage(void);
#endif
static int writeAuthFile(char *path);

#if ME_WIN_LIKE || VXWORKS
static char *getpass(char *prompt);
#endif

/*********************************** Code *************************************/
#if 0
int main(int argc, char *argv[])
{
    WebsBuf     buf;
    char        *cipher, *password, *authFile, *username, *encodedPassword, *realm, *cp, *roles;
    int         i, errflg, nextArg;

    username = 0;
    errflg = 0;
    password = 0;
    authFile = 0;
    cipher = "blowfish";

    for (i = 1; i < argc && !errflg; i++) {
        if (argv[i][0] != '-') {
            break;
        }
        for (cp = &argv[i][1]; *cp && !errflg; cp++) {
            if (*cp == '-') {
                cp++;
            }
            if (smatch(cp, "cipher")) {
                if (++i == argc) {
                    errflg++;
                } else {
                    cipher = argv[i];
                    if (!smatch(cipher, "md5") && !smatch(cipher, "blowfish")) {
                        web_error("Unknown cipher \"%s\". Use \"md5\" or \"blowfish\".", cipher);
                    }
                    break;
                }
            } else if (smatch(cp, "file") || smatch(cp, "f")) {
                if (++i == argc) {
                    errflg++;
                } else {
                    authFile = argv[i];
                    break;
                }

            } else if (smatch(cp, "password") || smatch(cp, "p")) {
                if (++i == argc) {
                    errflg++;
                } else {
                    password = argv[i];
                    break;
                }

            } else {
                errflg++;
            }
        }
    }
    nextArg = i;

    if ((nextArg + 3) > argc) {
        errflg++;
    }
    if (errflg) {
        printUsage();
        exit(2);
    }
    realm = argv[nextArg++];
    username = argv[nextArg++];

    bufCreate(&buf, 0, 0);
    for (i = nextArg; i < argc; ) {
        bufPutStr(&buf, argv[i]);
        if (++i < argc) {
            bufPutc(&buf, ',');
        }
    }
    roles = sclone(buf.servp);

    logOpen();
    websOpenAuth(1);

    if (authFile && access(authFile, R_OK) == 0) {
        if (websLoad(authFile) < 0) {
            exit(2);
        }
        if (access(authFile, W_OK) < 0) {
            web_error("Cannot write to %s", authFile);
            exit(4);
        }
    }
    if (!password && (password = getPassword()) == 0) {
        exit(1);
    }
    encodedPassword = websMD5(sfmt("%s:%s:%s", username, realm, password));

    if (smatch(cipher, "md5")) {
        encodedPassword = websMD5(sfmt("%s:%s:%s", username, realm, password));
    } else {
        /* This uses the more secure blowfish cipher */
        encodedPassword = websMakePassword(sfmt("%s:%s:%s", username, realm, password), 16, 128);
    }
    if (authFile) {
        websRemoveUser(username);
        if (websAddUser(username, encodedPassword, roles) == 0) {
            exit(7);
        }
        if (writeAuthFile(authFile) < 0) {
            exit(6);
        }
    } else {
        printf("%s\n", encodedPassword);
    }
    websCloseAuth();
    return 0;
}
#endif

/*
static void printUsage(void)
{
    web_error("usage: gopass [--cipher cipher] [--file path] [--password password] realm user roles...\n"
        "Options:\n"
        "    --cipher md5|blowfish Select the encryption cipher. Defaults to md5\n"
        "    --file filename       Modify the password file\n"
        "    --password password   Use the specified password\n"
        "\n");
}
*/
//webserver encoded cipher md5 realm goahead.com username admin password admin
//webserver encryption username root password admintsl123456! cipher md5 realm goahead.com
int web_gopass(const char *username, const char *password, const char *cipher,
		const char *realm,  char *encodedPassword)
{
    //WebsBuf     buf;
	char *encoded = NULL;
	encoded = websMD5(sfmt("%s:%s:%s", username, realm, password));
	if(encoded == NULL)
		return -1;
    if (smatch(cipher, "md5")) {
    	encoded = websMD5(sfmt("%s:%s:%s", username, realm, password));
    } else {
        /* This uses the more secure blowfish cipher */
    	encoded = websMakePassword(sfmt("%s:%s:%s", username, realm, password), 16, 128);
    }
	if(encoded == NULL)
		return -1;
    //printf("%s:(%s)\n", __func__, encoded);
    //f9bab09668b61f553807d1157c393272
/*  else
    {
        printf("%s\n", encodedPassword);
    }*/
    if(encodedPassword)
    	strcpy(encodedPassword, encoded);
    wfree(encoded);
    return 0;
}

int web_gopass_roles(const char *authFile, const char *username, const char *roles[])
{
	WebsBuf buf;
	char *webroles = NULL;
	int i = 0;

	bufCreate(&buf, 0, 0);
	for (i = 0; roles[i] != NULL;)
	{
		bufPutStr(&buf, roles[i]);
		i++;
		if (roles[i] != NULL)
		{
			bufPutc(&buf, ',');
		}
	}
	webroles = sclone(buf.servp);
	if (websSetUserRoles(username, webroles) == 0)
	{
		//exit(7);
		//websCloseAuth();
		return -1;
	}
	if (writeAuthFile(authFile) < 0)
	{
		return -1;
	}
	return 0;
}

int web_gopass_save(const char *authFile, const char *username, const char *roles[], char *encodedPassword)
{
	WebsBuf buf;
	char *webroles = NULL;
	int i = 0;

	bufCreate(&buf, 0, 0);
	for (i = 0; roles[i] != NULL;)
	{
		bufPutStr(&buf, roles[i]);
		i++;
		if (roles[i] != NULL)
		{
			bufPutc(&buf, ',');
		}
	}
	webroles = sclone(buf.servp);

	/*    websOpenAuth(1);

	 if (authFile && access(authFile, R_OK) == 0) {
	 if (websLoad(authFile) < 0) {
	 return -1;
	 }
	 if (access(authFile, W_OK) < 0) {
	 web_error("Cannot write to %s", authFile);
	 return -1;
	 }
	 }
	 if (authFile) {*/
	websRemoveUser(username);
	if (websAddUser(username, encodedPassword, webroles) == 0)
	{
		//exit(7);
		//websCloseAuth();
		return -1;
	}
	if (writeAuthFile(authFile) < 0) {
		return -1;
	}
	return 0;
}

int web_auth_save(const char *authFile)
{
	if (writeAuthFile(authFile) < 0) {
		return -1;
	}
	return 0;
}

static int writeAuthFile(char *path)
{
    FILE        *fp = NULL;
    WebsKey     *kp = NULL, *ap = NULL;
    WebsRole    *role = NULL;
    WebsUser    *user = NULL;
    WebsHash    roles, users;
    char        *tempFile = NULL;
    char        temp_cmd[512];
    web_assert(path && *path);

    tempFile = websTempFile(NULL, "gp");
    if ((fp = fopen(tempFile, "w" FILE_TEXT)) == 0) {
        web_error("Cannot open %s", tempFile);
        return -1;
    }
    fprintf(fp, "#\n#   %s - Authorization data\n#\n\n", basename(path));

    roles = websGetRoles();
    if (roles >= 0) {
        for (kp = hashFirst(roles); kp; kp = hashNext(roles, kp)) {
            role = kp->content.value.symbol;
            fprintf(fp, "role name=%s abilities=", kp->name.value.string);
            for (ap = hashFirst(role->abilities); ap; ap = hashNext(role->abilities, ap)) {
                fprintf(fp, "%s,", ap->name.value.string);
            }
            fputc('\n', fp);
        }
        fputc('\n', fp);
    }
    users = websGetUsers();
    if (users >= 0) {
        for (kp = hashFirst(users); kp; kp = hashNext(users, kp)) {
            user = kp->content.value.symbol;
            fprintf(fp, "user name=%s password=%s roles=%s", user->name, user->password, user->roles);
            fputc('\n', fp);
        }
    }
    fclose(fp);
    //unlink(path);
    memset(temp_cmd, 0, sizeof(temp_cmd));
    snprintf(temp_cmd, sizeof(temp_cmd), "cp %s %s.old", path, path);
    system(temp_cmd);
    memset(temp_cmd, 0, sizeof(temp_cmd));
    snprintf(temp_cmd, sizeof(temp_cmd), "mv %s %s", tempFile, path);
    system(temp_cmd);
/*    if (rename(tempFile, path) < 0) {
        web_error("Cannot create new %s from %s ipstack_errno %d", path, tempFile, ipstack_errno);
        return -1;
    }*/
    return 0;
}

#if 0
static char *getPassword(void)
{
    char    *password, *confirm;

    password = getpass("New password: ");
    confirm = getpass("Confirm password: ");
    if (smatch(password, confirm)) {
        return password;
    }
    web_error("Password not verified");
    return 0;
}



#if WINCE
static char *getpass(char *prompt)
{
    return "NOT-SUPPORTED";
}

#elif ME_WIN_LIKE || VXWORKS
static char *getpass(char *prompt)
{
    static char password[MAX_PASS];
    int     c, i;

    fputs(prompt, stdout);
    for (i = 0; i < (int) sizeof(password) - 1; i++) {
#if VXWORKS
        c = getchar();
#else
        c = _getch();
#endif
        if (c == '\r' || c == EOF) {
            break;
        }
        if ((c == '\b' || c == 127) && i > 0) {
            password[--i] = '\0';
            fputs("\b \b", stdout);
            i--;
        } else if (c == 26) {           /* Control Z */
            c = EOF;
            break;
        } else if (c == 3) {            /* Control C */
            fputs("^C\n", stdout);
            exit(255);
        } else if (!iscntrl((uchar) c) && (i < (int) sizeof(password) - 1)) {
            password[i] = c;
            fputc('*', stdout);
        } else {
            fputc('', stdout);
            i--;
        }
    }
    if (c == EOF) {
        return "";
    }
    fputc('\n', stdout);
    password[i] = '\0';
    return sclone(password);
}

#endif /* ME_WIN_LIKE */
 
/*
    Display the usage
 */

static void printUsage(void)
{
    web_error("usage: gopass [--cipher cipher] [--file path] [--password password] realm user roles...\n"
        "Options:\n"
        "    --cipher md5|blowfish Select the encryption cipher. Defaults to md5\n"
        "    --file filename       Modify the password file\n"
        "    --password password   Use the specified password\n"
        "\n");
}
#endif

/*
    Copyright (c) Embedthis Software. All Rights Reserved.
    This software is distributed under commercial and open source licenses.
    You may use the Embedthis GoAhead open source license or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.
 */
