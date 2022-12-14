/*
 * modem_operators.h
 *
 *  Created on: Aug 12, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_OPERATORS_H__
#define __MODEM_OPERATORS_H__

#define OPERATORS_NAME_MAX	64

typedef struct  md_operators_s
{
    int operator_code;	//运营商代码
    char *name;			//运营商英文名简称
    char *country; 		//运营商所在国家英文名
    char *nation;		//运营商所在国家中文名

/*
    char name[OPERATORS_NAME_MAX];		//运营商英文名简称
    char country[OPERATORS_NAME_MAX]; 	//运营商所在国家英文名
    char nation[OPERATORS_NAME_MAX];	//运营商所在国家中文名
*/
}md_operators_t;


extern const char * modem_operators_string(int code);
extern const char * modem_country_string(int code);
extern const char * modem_nation_string(int code);

#endif /* __MODEM_OPERATORS_H__ */
