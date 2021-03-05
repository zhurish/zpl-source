/*
 * modem_operators.c
 *
 *  Created on: Aug 12, 2018
 *      Author: zhurish
 */

#include "zebra.h"
#include "log.h"
#include "memory.h"
#include "str.h"

#include "modem_operators.h"

static md_operators_t operators_table[] =
{
	{ 0x20201, "Cosmote", "GR" , "Greece"},//希腊
	{ 0x20205, "Vodafone GR", "GR" , "Greece"},
	{ 0x20210, "TELESTET", "GR" , "Greece"},
	{ 0x20404, "Vodafone NL", "NL", "Netherlands, The" },//荷兰
	{ 0x20408, "KPN", "NL" , "Netherlands, The" },
	{ 0x20412, "O2 - NL", "NL" , "Netherlands, The" },
	{ 0x20416, "Ben", "NL" , "Netherlands, The" },
	{ 0x20420, "dutchtone", "NL" , "Netherlands, The" },
	{ 0x20601, "Proximus", "BE" , "Belgium" },//比利时
	{ 0x20610, "Mobistar", "BE" , "Belgium" },
	{ 0x20620, "Orange", "BE" , "Belgium" },
	{ 0x20801, "Orange", "FR" , "France" },//法国
	{ 0x20810, "SFR", "FR" , "France" },
	{ 0x20820, "BOUYGTEL", "FR" , "France" },
	{ 0x21303, "Mobiland", "AD" , "Andorra" },//安道尔共和国
	{ 0x21401, "Vodafone E", "ES" , "Spain" },//西班牙
	{ 0x21402, "MoviStar", "ES" , "Spain" },
	{ 0x21403, "AMENA", "ES" , "Spain" },
	{ 0x21404, "Xfera", "ES" , "Spain" },
	{ 0x21407, "MoviStar", "ES" , "Spain" },
	{ 0x21601, "Pannon GSM", "HU" , "Hungary" },//匈牙利
	{ 0x21630, "Westel", "HU" , "Hungary" },
	{ 0x21670, "Vodafone", "HU" , "Hungary" },
	{ 0x21803, "ERONET", "BA" , "Bosnia and Herzegovina" },
	{ 0x21805, "Mobilna Srpska", "BA" , "Bosnia and Herzegovina" },
	{ 0x21890, "GSM BiH", "BA" , "Bosnia and Herzegovina" },
	{ 0x21901, "CRONET", "HR" , "Croatia" },//克罗地亚
	{ 0x21910, "VIP", "HR" , "Croatia" },
	//{ 0x22001, "MOBTEL", "YU" , "GR" },
	//{ 0x22002, "ProMonte", "YU" , "GR" },
	//{ 0x22003, "Telekom Srbija", "YU" , "GR" },
	//{ 0x22004, "MONET", "YU" , "GR" },
	{ 0x22201, "TIM", "IT" , "Italy" },//意大利
	{ 0x22210, "Vodafone IT", "IT" , "Italy" },
	{ 0x22288, "WIND", "IT" , "Italy" },
	{ 0x22298, "Blu SpA", "IT" , "Italy" },
	{ 0x22601, "CONNEX", "RO" , "Romania" },//罗马尼亚
	{ 0x22603, "Cosmorom", "RO" , "Romania" },
	{ 0x22610, "dialog", "RO" , "Romania" },
	{ 0x22801, "Swiss GSM", "CH" , "Switzerland" },//瑞士
	{ 0x22802, "sunrise", "CH" , "Switzerland" },
	{ 0x22803, "Orange", "CH" , "Switzerland" },
	{ 0x23001, "PAEGAS", "CZ" , "Czech Republic" },//捷克
	{ 0x23002, "EUROTEL", "CZ" , "Czech Republic" },
	{ 0x23003, "OSKAR", "CZ" , "Czech Republic" },
	{ 0x23101, "GLOBTEL", "SK" , "Slovakia" },//斯洛伐克
	{ 0x23102, "EUROTEL", "SK" , "Slovakia" },
	{ 0x23201, "A1", "AT" , "Austria" },//奥地利
	{ 0x23203, "T-Mobile", "AT" , "Austria" },
	{ 0x23205, "ONE", "AT" , "Austria" },
	{ 0x23207, "tele.ring", "AT" , "Austria" },
	{ 0x23410, "O2 - UK", "GB" , "United Kingdom" },//英国
	{ 0x23415, "Vodafone", "GB" , "United Kingdom" },
	{ 0x23430, "T-Mobile UK", "GB" , "United Kingdom" },
	{ 0x23431, "T-Mobile UK", "GB" , "United Kingdom" },
	{ 0x23432, "T-Mobile UK", "GB" , "United Kingdom" },
	{ 0x23433, "Orange", "GB" , "United Kingdom" },
	{ 0x23450, "JT GSM", "GB" , "United Kingdom" },
	{ 0x23455, "GUERNSEY TEL", "GB" , "United Kingdom" },
	{ 0x23458, "MANX", "GB" , "United Kingdom" },
	{ 0x23801, "TDK-MOBIL", "DK" , "Denmark" },//丹麦
	{ 0x23802, "SONOFON", "DK" , "Denmark" },
	{ 0x23820, "Telia", "DK" , "Denmark" },
	{ 0x23830, "Orange", "DK" , "Denmark" },
	{ 0x24001, "Telia S", "SE" , "Sweden" },//瑞典
	{ 0x24007, "IQ", "SE" , "Sweden" },
	{ 0x24008, "Vodafone", "SE" , "Sweden" },
	{ 0x24201, "TELENOR", "NO" , "Norway" },//挪威
	{ 0x24202, "NetCom", "NO" , "Norway" },
	{ 0x24403, "Telia", "FI" , "Finland" },//芬兰
	{ 0x24405, "RADIOLINJA", "FI" , "Finland" },
	{ 0x24409, "FINNET", "FI" , "Finland" },
	{ 0x24412, "2G", "FI" , "Finland" },
	{ 0x24414, "AMT", "FI" , "Finland" },
	{ 0x24491, "SONERA", "FI" , "Finland" },
	{ 0x24601, "OMNITEL", "LT" , "Lithuania" },//立陶宛
	{ 0x24602, "Bite GSM", "LT" , "Lithuania" },
	{ 0x24603, "TELE2", "LT" , "Lithuania" },
	{ 0x24701, "LMT GSM", "LV" , "Latvia" },//拉脱维亚
	{ 0x24702, "BALTCOM", "LV" , "Latvia" },
	{ 0x24801, "EMT GSM", "EE" , "Estonia" },//爱沙尼亚
	{ 0x24802, "RLE", "EE" , "Estonia" },
	{ 0x24803, "Q GSM", "EE" , "Estonia" },
	{ 0x25001, "MTS", "RU" , "Russia" },//俄罗斯
	{ 0x25002, "NorthWest GSM", "RU" , "Russia" },
	{ 0x25003, "NCC", "RU" , "Russia" },
	{ 0x25005, "SCS", "RU" , "Russia" },
	{ 0x25007, "SMARTS", "RU" , "Russia" },
	{ 0x25010, "DTC", "RU" , "Russia" },
	{ 0x25011, "Orensot", "RU" , "Russia" },
	{ 0x25012, "Far East", "RU" , "Russia" },
	{ 0x25013, "Kuban GSM", "RU" , "Russia" },
	{ 0x25016, "NTC", "RU" , "Russia" },
	{ 0x25017, "Ermak RMS", "RU" , "Russia" },
	{ 0x25028, "EXTEL", "RU" , "Russia" },
	{ 0x25039, "Uraltel", "RU" , "Russia" },
	{ 0x25044, "NC-GSM", "RU" , "Russia" },
	{ 0x25091, "Sonic Duo", "RU" , "Russia" },
	{ 0x25092, "Primtel", "RU" , "Russia" },
	{ 0x25093, "JSC Telecom XXI", "RU" , "Russia" },
	{ 0x25099, "Bee Line", "RU" , "Russia" },
	{ 0x25501, "UMC", "UA" , "Ukraine" },//乌克兰
	{ 0x25502, "WellCOM", "UA" , "Ukraine" },
	{ 0x25503, "Kyivstar", "UA" , "Ukraine" },
	{ 0x25505, "Golden Telecom", "UA" , "Ukraine" },
	{ 0x25701, "VELCOM", "BY" , "Belarus" },
	{ 0x25901, "VOXTEL", "MD" , "Moldova" },
	{ 0x25902, "MOLDCELL", "MD" , "Moldova" },
	{ 0x26001, "Plus GSM", "PL" , "Poland" },//波兰
	{ 0x26002, "Era GSM", "PL" , "Poland" },
	{ 0x26003, "IDEA", "PL" , "Poland" },
	{ 0x26201, "T-Mobile", "DE" , "Germany" },//德国
	{ 0x26202, "Vodafone D2", "DE" , "Germany" },
	{ 0x26203, "E-Plus", "DE" , "Germany" },
	{ 0x26207, "O2 - DE", "DE" , "Germany" },
	{ 0x26213, "Mobilcom", "DE" , "Germany" },
	{ 0x26601, "GIBTEL", "GI" , "Gibraltar" },//直布罗陀
	{ 0x26801, "Vodafone", "PT" , "Portugal" },//葡萄牙
	{ 0x26803, "OPTIMUS", "PT" , "Portugal" },
	{ 0x26806, "TMN", "PT" , "Portugal" },
	{ 0x27001, "LUXGSM", "LU" , "Luxembourg" },//卢森堡
	{ 0x27077, "TANGO", "LU" , "Luxembourg" },
	{ 0x27201, "Vodafone IRL", "IE" , "Ireland" },//爱尔兰
	{ 0x27202, "O2 - IRL", "IE" , "Ireland" },
	{ 0x27203, "METEOR", "IE" , "Ireland" },
	{ 0x27401, "SIMINN", "IS" , "Iceland" },//冰岛
	{ 0x27402, "TAL", "IS" , "Iceland" },
	{ 0x27404, "Viking", "IS" , "Iceland" },
	{ 0x27601, "AMC", "AL" , "Albania" },//阿尔巴尼亚
	{ 0x27602, "Vodafone", "AL" , "Albania" },
	{ 0x27801, "Vodafone Malta", "MT" , "Malta" },//马耳他
	{ 0x27821, "go mobile", "MT" , "Malta" },
	{ 0x28001, "CYTAGSM", "CY" , "Cyprus" },//塞浦路斯
	{ 0x28201, "GEOCELL", "GE" , "Georgia" },//乔治亚州
	{ 0x28202, "Magti GSM", "GE" , "Georgia" },
	{ 0x28203, "GEO 03", "GE" , "Georgia" },
	{ 0x28301, "ARMGSM", "AM" , "Armenia" },//亚美尼亚
	{ 0x28401, "M-TEL GSM", "BG" , "Bulgaria" },//保加利亚
	{ 0x28601, "TURKCELL", "TR" , "Turkey" },//土耳其
	{ 0x28602, "TELSIM", "TR" , "Turkey" },
	{ 0x28603, "ARIA", "TR" , "Turkey" },
	{ 0x28604, "AYCELL", "TR" , "Turkey" },
	{ 0x28801, "FT GSM", "FO" , "Faroe Islands" },
	{ 0x29001, "TELE Greenland", "GR" , "Greece" },
	{ 0x29340, "SI.MOBIL", "SI" , "Slovenia" },//斯洛文尼亚
	{ 0x29341, "MOBITEL", "SI" , "Slovenia" },
	{ 0x29401, "MOBIMAK", "MK" , "Macedonia" },
	{ 0x29501, "TELECOM", "LI" , "Liechtenstein" },
	{ 0x29502, "MONTEL", "LI" , "Liechtenstein" },
	{ 0x29505, "FL1", "LI" , "Liechtenstein" },
	{ 0x29577, "LI TANGO", "LI" , "Liechtenstein" },
	{ 0x30237, "Microcell", "CA" , "Canada" },//加拿大
	{ 0x30272, "Rogers AT&T", "CA" , "Canada" },
	{ 0x31001, "Cellnet", "US" , "United States" },//美国
	{ 0x31011, "Wireless 2000", "US" , "United States" },
	{ 0x31015, "Cingular", "US" , "United States" },
	{ 0x31016, "T-Mobile", "US" , "United States" },
	{ 0x31017, "Cingular", "US" , "United States" },
	{ 0x31018, "Cingular", "US" , "United States" },
	{ 0x31020, "T-Mobile", "US" , "United States" },
	{ 0x31021, "T-Mobile", "US" , "United States" },
	{ 0x31022, "T-Mobile", "US" , "United States" },
	{ 0x31023, "T-Mobile", "US" , "United States" },
	{ 0x31024, "T-Mobile", "US" , "United States" },
	{ 0x31025, "T-Mobile", "US" , "United States" },
	{ 0x31026, "T-Mobile", "US" , "United States" },
	{ 0x31027, "T-Mobile", "US" , "United States" },
	{ 0x31031, "T-Mobile", "US" , "United States" },
	{ 0x31034, "WestLink", "US" , "United States" },
	{ 0x31035, "Carolina", "US" , "United States" },
	{ 0x31038, "AT&T Wireless", "US" , "United States" },
	{ 0x31041, "Cingular", "US" , "United States" },
	{ 0x31046, "TMP Corp", "US" , "United States" },
	{ 0x31058, "T-Mobile", "US" , "United States" },
	{ 0x31061, "Epic Touch", "US" , "United States" },
	{ 0x31063, "AmeriLink", "US" , "United States" },
	{ 0x31064, "Einstein PCS", "US" , "United States" },
	{ 0x31066, "T-Mobile", "US" , "United States" },
	{ 0x31067, "Wireless 2000", "US" , "United States" },
	{ 0x31068, "NPI Wireless", "US" , "United States" },
	{ 0x31069, "Conestoga", "US" , "United States" },
	{ 0x31074, "Telemetrix", "US" , "United States" },
	{ 0x31076, "PTSI", "US" , "United States" },
	{ 0x31077, "Iowa Wireless", "US" , "United States" },
	{ 0x31078, "Airlink PCS", "US" , "United States" },
	{ 0x31079, "PinPoint", "US" , "United States" },
	{ 0x31080, "T-Mobile", "US" , "United States" },
	{ 0x31098, "AWS", "US" , "United States" },
/*	{ 0x31114, "Sprocket", "UNKNOWN_COUNTRY_CODE" , "GR" },
	{ 0x31601, "Nextel", "UNKNOWN_COUNTRY_CODE" , "GR" },*/
	{ 0x33805, "Digicel", "JM" , "Jamaica" },//牙买加
	{ 0x34001, "Orange ", "FW" , "French West Indies" },
	{ 0x34020, "Bouygues", "FW" , "French West Indies" },
	{ 0x34430, "APUA PCS", "AG" , "Antigua and Barbuda" },
	{ 0x35001, "Telecom", "BM" , "Bermuda" },//百慕大
	{ 0x36251, "Telcell GSM", "AN" , "Netherlands Antillies" },
	{ 0x36801, "C_Com", "CU" , "Cuba" },//古巴
	{ 0x37001, "Orange", "DO" , "Dominican Republic" },
	{ 0x40001, "AZERCELL GSM", "AZ" , "Azerbaijan" },
	{ 0x40002, "BAKCELL GSM 2000", "AZ" , "Azerbaijan" },
	{ 0x40101, "K-MOBILE", "KZ" , "Kazakhstan" },//哈萨克
	{ 0x40102, "K'CELL", "KZ" , "Kazakhstan" },
	{ 0x40401, "Hutch", "IN" , "India" },//印度
	{ 0x40402, "Airtel", "IN" , "India" },
	{ 0x40403, "Airtel", "IN" , "India" },
	{ 0x40404, "IDEA", "IN" , "India" },
	{ 0x40405, "Hutch", "IN" , "India" },
	{ 0x40407, "IDEA", "IN" , "India" },
	{ 0x40409, "Reliance", "IN" , "India" },
	{ 0x40410, "Airtel", "IN" , "India" },
	{ 0x40411, "Hutch", "IN" , "India" },
	{ 0x40412, "IDEA", "IN" , "India" },
	{ 0x40413, "Hutch", "IN" , "India" },
	{ 0x40414, "Spice", "IN" , "India" },
	{ 0x40415, "Hutch", "IN" , "India" },
	{ 0x40416, "Airtel", "IN" , "India" },
	{ 0x40418, "Reliance", "IN" , "India" },
	{ 0x40419, "IDEA", "IN" , "India" },
	{ 0x40420, "Hutch", "IN" , "India" },
	{ 0x40421, "BPL Mobile", "IN" , "India" },
	{ 0x40422, "IDEA", "IN" , "India" },
	{ 0x40424, "IDEA", "IN" , "India" },
	{ 0x40427, "Hutch", "IN" , "India" },
	{ 0x40429, "Aircel", "IN" , "India" },
	{ 0x40430, "Hutch", "IN" , "India" },
	{ 0x40431, "Airtel", "IN" , "India" },
	{ 0x40434, "Cellone", "IN" , "India" },
	{ 0x40436, "Reliance", "IN" , "India" },
	{ 0x40437, "Aircel", "IN" , "India" },
	{ 0x40438, "Cellone", "IN" , "India" },
	{ 0x40440, "Airtel", "IN" , "India" },
	{ 0x40441, "RPG", "IN" , "India" },
	{ 0x40443, "Hutch", "IN" , "India" },
	{ 0x40444, "Spice", "IN" , "India" },
	{ 0x40445, "Airtel", "IN" , "India" },
	{ 0x40446, "Hutch", "IN" , "India" },
	{ 0x40449, "Airtel", "IN" , "India" },
	{ 0x40450, "Reliance", "IN" , "India" },
	{ 0x40451, "Cellone", "IN" , "India" },
	{ 0x40452, "Reliance", "IN" , "India" },
	{ 0x40453, "Cellone", "IN" , "India" },
	{ 0x40454, "Cellone", "IN" , "India" },
	{ 0x40455, "Cellone", "IN" , "India" },
	{ 0x40456, "IDEA", "IN" , "India" },
	{ 0x40457, "Cellone", "IN" , "India" },
	{ 0x40458, "Cellone", "IN" , "India" },
	{ 0x40459, "Cellone", "IN" , "India" },
	{ 0x40460, "Hutch", "IN" , "India" },
	{ 0x40462, "Cellone", "IN" , "India" },
	{ 0x40464, "Cellone", "IN" , "India" },
	{ 0x40466, "Cellone", "IN" , "India" },
	{ 0x40467, "Reliance", "IN" , "India" },
	{ 0x40468, "DOLPHIN", "IN" , "India" },
	{ 0x40469, "MTNL", "IN" , "India" },
	{ 0x40470, "Airtel", "IN" , "India" },
	{ 0x40471, "Cellone", "IN" , "India" },
	{ 0x40472, "Cellone", "IN" , "India" },
	{ 0x40473, "Cellone", "IN" , "India" },
	{ 0x40474, "Cellone", "IN" , "India" },
	{ 0x40475, "Cellone", "IN" , "India" },
	{ 0x40476, "Cellone", "IN" , "India" },
	{ 0x40477, "Cellone", "IN" , "India" },
	{ 0x40478, "IDEA", "IN" , "India" },
	{ 0x40479, "Cellone", "IN" , "India" },
	{ 0x40480, "Cellone", "IN" , "India" },
	{ 0x40481, "Cellone", "IN" , "India" },
	{ 0x40482, "IDEA", "IN" , "India" },
	{ 0x40483, "Reliance", "IN" , "India" },
	{ 0x40484, "Hutch", "IN" , "India" },
	{ 0x40485, "Reliance", "IN" , "India" },
	{ 0x40486, "Hutch", "IN" , "India" },
	{ 0x40487, "IDEA", "IN" , "India" },
	{ 0x40488, "Hutch", "IN" , "India" },
	{ 0x40489, "IDEA", "IN" , "India" },
	{ 0x40490, "Airtel", "IN" , "India" },
	{ 0x40492, "Airtel", "IN" , "India" },
	{ 0x40493, "Airtel", "IN" , "India" },
	{ 0x40494, "Airtel", "IN" , "India" },
	{ 0x40495, "Airtel", "IN" , "India" },
	{ 0x40496, "Airtel", "IN" , "India" },
	{ 0x40497, "Airtel", "IN" , "India" },
	{ 0x40498, "Airtel", "IN" , "India" },
	{ 0x40551, "Airtel", "IN" , "India" },
	{ 0x40552, "Airtel", "IN" , "India" },
	{ 0x40553, "Airtel", "IN" , "India" },
	{ 0x40554, "Airtel", "IN" , "India" },
	{ 0x40555, "Airtel", "IN" , "India" },
	{ 0x40556, "Airtel", "IN" , "India" },
	{ 0x40566, "Hutch", "IN" , "India" },
	{ 0x40567, "Hutch", "IN" , "India" },
	{ 0x41001, "Mobilink", "PK" , "Pakistan" },//巴基斯坦
	{ 0x41302, "DIALOG", "LK" , "Sri Lanka" },
	{ 0x41303, "CELLTEL", "LK" , "Sri Lanka" },
	{ 0x41401, "MM 900", "MM" , "Myanmar" },//缅甸
	{ 0x41501, "Cellis", "LB" , "Lebanon" },//黎巴嫩
	{ 0x41503, "LibanCell", "LB" , "Lebanon" },
	{ 0x41601, "Fastlink", "JO" , "Jordan" },//约旦
	{ 0x41677, "MobileCom", "JO" , "Jordan" },
	{ 0x41702, "Spacetel", "SY" , "Syria" },//叙利亚
	{ 0x41709, "MOBILE SYRIA", "SY" , "Syria" },
	{ 0x41902, "MTCNet", "KW" , "Kuwait" },//科威特
	{ 0x41903, "WATANIYA", "KW" , "Kuwait" },
	{ 0x42001, "Al-Jawal", "SA" , "Saudi Arabia" },
	{ 0x42007, "EAE", "SA" , "Saudi Arabia" },
	{ 0x42102, "Spacetel", "YE" , "Yemen" },//也门
	{ 0x42202, "OMAN MOBILE", "OM" , "Oman" },//阿曼
	{ 0x42402, "ETISALAT", "AE" , "United Arab Emirates" },//阿拉伯联合酋长国
	{ 0x42501, "Orange", "IL" , "Israel" },//以色列
	{ 0x42505, "JAWWAL", "IL" , "Israel" },
	{ 0x42601, "MOBILE PLUS", "BH" , "Bahrain" },//巴林岛
	{ 0x42701, "QATARNET", "QA" , "Qatar" },//卡塔尔
	{ 0x42899, "MobiCom", "MN" , "Mongolia" },//蒙古
	{ 0x42901, "NTC", "NP" , "Nepal" },//尼泊尔
	{ 0x43211, "TCI", "IR" , "Iran" },//伊朗
	{ 0x43214, "KISH", "IR" , "Iran" },
	{ 0x43401, "Buztel", "UZ" , "Uzbekistan" },//乌兹别克斯坦
	{ 0x43402, "Uzmacom", "UZ" , "Uzbekistan" },
	{ 0x43404, "UZB DAEWOO-GSM", "UZ" , "Uzbekistan" },
	{ 0x43405, "Coscom", "UZ" , "Uzbekistan" },
	{ 0x43701, "BITEL", "KG" , "Kyrgyzstan" },
	{ 0x43801, "BCTI", "TM" , "Turkmenistan" },//土库曼斯坦
	{ 0x45201, "MOBIFONE", "VN" , "Vietnam" },//越南
	{ 0x45202, "VINAFONE", "VN" , "Vietnam" },
	{ 0x45400, "CSL", "HK" , "Hong Kong" },//香港
	{ 0x45401, "NEW WORLD", "HK" , "Hong Kong" },
	{ 0x45402, "CSL", "HK" , "Hong Kong" },
	{ 0x45404, "Orange", "HK" , "Hong Kong" },
	{ 0x45406, "SMC", "HK" , "Hong Kong" },
	{ 0x45410, "NEW WORLD", "HK" , "Hong Kong" },
	{ 0x45412, "PEOPLES", "HK" , "Hong Kong" },
	{ 0x45416, "SUNDAY", "HK" , "Hong Kong" },
	{ 0x45418, "HK TELECOM", "HK" , "Hong Kong" },
/*	{ 0x45500, "SmarTone", "UNKNOWN_COUNTRY_CODE" , "GR" },
	{ 0x45501, "CTMGSM", "UNKNOWN_COUNTRY_CODE" , "GR" },
	{ 0x45503, "HT Macau", "UNKNOWN_COUNTRY_CODE" , "GR" },*/
	{ 0x45601, "MobiTel", "KH" , "Cambodia" },//柬埔寨
	{ 0x45602, "SAMART-GSM", "KH" , "Cambodia" },
	{ 0x45701, "LAO GSM", "LA" , "Laos" },//老挝
	{ 0x46000, "CHINA MOBILE GSM", "CN" , "China" },
	{ 0x46001, "CHINA UNICOM GSM", "CN" , "China" },
	{ 0x46002, "CHINA MOBILE TD-S", "CN" , "China" },
	{ 0x46003, "CHINA TELECOM CDMA", "CN" , "China" },

	{ 0x46005, "CHINA TELECOM CDMA", "CN" , "China" },
	{ 0x46006, "CHINA UNICOM WCDMA", "CN" , "China" },
	{ 0x46007, "CHINA MOBILE TD-S", "CN" , "China" },
	{ 0x46011, "CHINA TELECOM FDD-LTE", "CN" , "China" },
	{ 0x46020, "CHINA GSM_R", "CN" , "China" },

	{ 0x46601, "Far EasTone", "TW" , "Taiwan" },//台湾
	{ 0x46606, "TUNTEX", "TW" , "Taiwan" },
	{ 0x46668, "ACeS", "TW" , "Taiwan" },
	{ 0x46688, "KGT", "TW" , "Taiwan" },
	{ 0x46692, "Chunghwa", "TW" , "Taiwan" },
	{ 0x46693, "MobiTai", "TW" , "Taiwan" },
	{ 0x46697, "TWN GSM", "TW" , "Taiwan" },
	{ 0x46699, "TransAsia", "TW" , "Taiwan" },
	{ 0x47001, "GrameemPhone", "BD" , "Bangladesh" },//孟加拉国
	{ 0x47002, "AKTEL", "BD" , "Bangladesh" },
	{ 0x47003, "ShebaWorld", "BD" , "Bangladesh" },
	{ 0x47019, "Mobile 2000", "BD" , "Bangladesh" },
	{ 0x47201, "DHIMOBILE", "MV" , "Maldives" },//马尔代夫
	{ 0x50212, "Maxis Mobile", "MY" , "Malaysia" },//马来西亚
	{ 0x50213, "TM Touch", "MY" , "Malaysia" },
	{ 0x50216, "DiGi", "MY" , "Malaysia" },
	{ 0x50217, "ADAM", "MY" , "Malaysia" },
	{ 0x50219, "CELCOM", "MY" , "Malaysia" },
	{ 0x50501, "MobileNet", "AU" , "Australia" },//澳大利亚
	{ 0x50502, "OPTUS", "AU" , "Australia" },
	{ 0x50503, "Vodafone", "AU" , "Australia" },
	{ 0x50508, "One.Tel", "AU" , "Australia" },
	{ 0x51000, "ACeS", "ID" , "Indonesia" },//印尼
	{ 0x51001, "SATELINDOCEL", "ID" , "Indonesia" },
	{ 0x51008, "LIPPO TEL", "ID" , "Indonesia" },
	{ 0x51010, "TELKOMSEL", "ID" , "Indonesia" },
	{ 0x51011, "GSM-XL", "ID" , "Indonesia" },
	{ 0x51021, "INDOSAT", "ID" , "Indonesia" },
	{ 0x51501, "ISLACOM", "PH" , "Philippines" },//菲律宾
	{ 0x51502, "Globe", "PH" , "Philippines" },
	{ 0x51503, "SMART", "PH" , "Philippines" },
	{ 0x51505, "Digitel", "PH" , "Philippines" },
	{ 0x51511, "ACeS", "PH" , "Philippines" },
	{ 0x52001, "AIS GSM", "TH" , "Thailand" },//泰国
	{ 0x52015, "ACT Mobile", "TH" , "Thailand" },
	{ 0x52018, "WP-1800", "TH" , "Thailand" },
	{ 0x52020, "ACeS", "TH" , "Thailand" },
	{ 0x52023, "HELLO", "TH" , "Thailand" },
	{ 0x52099, "Orange", "TH" , "Thailand" },
	{ 0x52501, "ST-GSM-SGP", "SG" , "Singapore" },//新加坡
	{ 0x52502, "ST-GSM1800-SGP", "SG" , "Singapore" },
	{ 0x52503, "M1-GSM-SGP", "SG" , "Singapore" },
	{ 0x52504, "SGP-M1-3GSM", "SG" , "Singapore" },
	{ 0x52505, "STARHUB-SGP", "SG" , "Singapore" },
	{ 0x52811, "BRU TSTCom", "BN" , "Brunei" },//汶莱
	{ 0x53001, "Vodafone", "NZ" , "New Zealand" },
	{ 0x53901, "Tonga Comm.", "TO" , "Tonga" },//东加
	{ 0x54100, "ACeS", "VU" , "Vanuatu" },
	{ 0x54101, "SMILE", "VU" , "Vanuatu" },
	{ 0x54201, "Vodafone", "FJ" , "Fiji Islands" },
	{ 0x54411, "Blue Sky", "AS" , "American Samoa" },//萨摩亚
	{ 0x54601, "MOBILIS", "NC" , "New Caledonia" },
	{ 0x54720, "VINI", "PF" , "French Polynesia" },//波利尼西亚
	{ 0x55001, "FSM", "FM" , "Micronesia" },
	{ 0x60201, "MobiNiL", "EG" , "Egypt" },//埃及
	{ 0x60202, "CLICK GSM", "EG" , "Egypt" },
	{ 0x60301, "AMN", "DZ" , "Algeria" },//阿尔及利亚
	{ 0x60400, "Meditel", "MA" , "Morocco" },//摩洛哥
	{ 0x60401, "IAM", "MA" , "Morocco" },
	{ 0x60502, "TUNICELL", "TN" , "Tunisia" },//突尼斯
	{ 0x60801, "ALIZE", "SN" , "Senegal" },//塞内加尔
	{ 0x60802, "SENTEL", "SN" , "Senegal" },
	{ 0x61001, "MALITEL", "ML" , "Mali" },//马里
	{ 0x61101, "MOBILIS", "GN" , "Guinea" },//几内亚
	{ 0x61102, "LAGUI", "GN" , "Guinea" },
/*	{ 0x61201, "CORA", "CI" , "GR" },
	{ 0x61203, "Ivoiris", "CI" , "GR" },
	{ 0x61205, "TELECEL", "CI" , "GR" },
	{ 0x61302, "CELTEL", "BF" , "Burkina Faso" },
	{ 0x61402, "CELTEL", "NE" , "Niger" },*/
	{ 0x61501, "TOGOCEL", "TG" , "Togo" },//多哥
	{ 0x61601, "LIBERCOM", "BJ" , "Benin" },//贝南
	{ 0x61602, "Telecel Benin", "BJ" , "Benin" },
	{ 0x61603, "BENINCELL", "BJ" , "Benin" },
	{ 0x61701, "CELLPLUS", "MU" , "Mauritius" },//毛里求斯
	{ 0x61710, "EMTEL", "MU" , "Mauritius" },
	{ 0x61801, "Omega", "LR" , "Liberia" },//利比里亚
	{ 0x62001, "SPACEFON", "GH" , "Ghana" },//加纳
	{ 0x62002, "ONEtouch", "GH" , "Ghana" },
	{ 0x62003, "MOBITEL", "GH" , "Ghana" },
	{ 0x62100, "MTN", "NG" , "Nigeria" },//尼日利亚
	{ 0x62120, "ECONET", "NG" , "Nigeria" },
	{ 0x62130, "MTN", "NG" , "Nigeria" },
	{ 0x62140, "NITEL GSM", "NG" , "Nigeria" },
	{ 0x62201, "CELTEL", "TD" , "Chad" },//乍得
	{ 0x62202, "LIBERTIS", "TD" , "Chad" },
	{ 0x62401, "MTN-CAM", "CM" , "Cameroon" },//喀麦隆
	{ 0x62402, "MOBILIS", "CM" , "Cameroon" },
	{ 0x62501, "CPV MOVEL", "CV" , "Cape Verde" },
	{ 0x62801, "LIBERTIS", "GA" , "Gabon" },//加蓬
	{ 0x62802, "GO Celtel", "GA" , "Gabon" },
	{ 0x62803, "CELTEL", "GA" , "Gabon" },
	{ 0x62901, "CELTEL", "CG" , "Congo" },//刚果
	{ 0x62910, "LIBERTIS", "CG" , "Congo" },
	{ 0x63001, "CELLNET", "CD" , "Congo (DRC)" },//刚果
	{ 0x63002, "CELTEL", "CD" , "Congo (DRC)" },
	{ 0x63004, "CELLCO", "CD" , "Congo (DRC)" },
	{ 0x63089, "OASIS", "CD" , "Congo (DRC)" },
	{ 0x63301, "SEYCEL", "SC" , "Seychelles" },
	{ 0x63310, "AIRTEL", "SC" , "Seychelles" },
	{ 0x63401, "MobiTel", "MZ" , "Mozambique" },//莫桑比克
	{ 0x63510, "Rwandacell", "RW" , "Rwanda" },//卢安达
	{ 0x63601, "ETMTN", "ET" , "Ethiopia" },//埃塞俄比亚
	{ 0x63701, "BARAKAAT", "SO" , "Somalia" },//索马里
	{ 0x63902, "Safaricom", "KE" , "Kenya" },//肯尼亚
	{ 0x63903, "KENCELL", "KE" , "Kenya" },
	{ 0x64001, "TRITEL", "TZ" , "Tanzania" },//坦桑尼亚
	{ 0x64002, "MobiTel", "TZ" , "Tanzania" },
	{ 0x64003, "ZANTEL", "TZ" , "Tanzania" },
	{ 0x64004, "Vodacom", "TZ" , "Tanzania" },
	{ 0x64005, "CELTEL", "TZ" , "Tanzania" },
	{ 0x64101, "CelTel", "UG" , "Uganda" },//乌干达
	{ 0x64110, "MTN-UGANDA", "UG" , "Uganda" },
	{ 0x64111, "UTL TELECEL", "UG" , "Uganda" },
	{ 0x64201, "Spacetel", "BI" , "Burundi" },//布隆迪
	{ 0x64202, "SAFARIS", "BI" , "Burundi" },
/*	{ 0x64301, "mCel", "UNKNOWN_COUNTRY_CODE" , "GR" },
	{ 0x64501, "ZAMCELL", "UNKNOWN_COUNTRY_CODE" , "GR" },
	{ 0x64502, "TELECEL", "UNKNOWN_COUNTRY_CODE" , "GR" },*/
	{ 0x64601, "Madacom", "MG" , "Madagascar" },//马达加斯加
	{ 0x64602, "ANTARIS", "MG" , "Madagascar" },
	{ 0x64700, "Orange Reunion", "RE" , "Reunion" },
	{ 0x64710, "SFR Reunion", "RE" , "Reunion" },
	{ 0x64801, "NET*ONE", "ZW" , "Zimbabwe" },//津巴布韦
	{ 0x64803, "TELECEL", "ZW" , "Zimbabwe" },
	{ 0x64804, "ECONET", "ZW" , "Zimbabwe" },
	{ 0x64901, "MTC", "NA" , "Namibia" },//纳米比亚
	{ 0x65001, "CALLPOINT 90", "MW" , "Malawi" },//马拉维
	{ 0x65010, "CelTel", "MW" , "Malawi" },
	{ 0x65101, "Vodacom", "LS" , "Lesotho" },
	{ 0x65201, "MASCOM", "BW" , "Botswana" },
	{ 0x65202, "VISTA", "BW" , "Botswana" },
	{ 0x65310, "SwaziMTN", "SZ" , "Swaziland" },
	{ 0x65501, "Vodacom", "ZA" , "South Africa" },
	{ 0x65507, "Cell C (Pty) Ltd", "ZA" , "South Africa" },
	{ 0x65510, "MTN", "ZA" , "South Africa" },
	{ 0x70601, "PERSONAL", "SV" , "El Salvador" },
	{ 0x70602, "DIGICEL", "SV" , "El Salvador" },
	{ 0x71610, "TIM", "PE" , "Peru" },//秘鲁
	{ 0x72207, "UNIFON", "AR" , "Argentina" },//阿根廷
	{ 0x72234, "Telecom Personal", "AR" , "Argentina" },
	{ 0x72235, "PORT-HABLE", "AR" , "Argentina" },
	{ 0x72402, "TIM BRASIL", "BR" , "Brazil" },//巴西
	{ 0x72403, "TIM BRASIL", "BR" , "Brazil" },
	{ 0x72404, "TIM BRASIL", "BR" , "Brazil" },
	{ 0x72405, "Claro", "BR" , "Brazil" },
	{ 0x72416, "BrTCel", "BR" , "Brazil" },
	{ 0x73001, "ENTEL PCS", "CL" , "Chile" },//智利
	{ 0x73010, "ENTEL PCS", "CL" , "Chile" },
	{ 0x73401, "INFONET", "VE" , "Venezuela" },//委内瑞拉
	{ 0x73402, "DIGITEL", "VE" , "Venezuela" },
	{ 0x73601, "NUEVATEL", "BO" , "Bolivia" },//玻利维亚
	{ 0x73602, "ENTEL", "BO" , "Bolivia" },
	{ 0x74401, "VOX", "PY" , "Paraguay" },//巴拉圭
	{ 0x74402, "PY 02", "PY" , "Paraguay" },
	{ 0x74601, "ICMS", "SR" , "Suriname" },
	{ 0x90105, "Thuraya", "UNKNOWN_COUNTRY_CODE" , "UNKNOWN_COUNTRY_CODE" },
};



/*const char * modem_operators_string(int code)
{
	int i = 0;
	static char opname[OPERATORS_NAME_MAX * 3];
	for(i = 0; i < sizeof(operators_table)/sizeof(operators_table[0]); i++)
	{
		if(operators_table[i].operator_code == code)
		{
			os_memset(opname, 0, sizeof(opname));
			os_snprintf(opname, sizeof(opname), "%s %s",
					operators_table[i].country,
					operators_table[i].name);
		}
	}
	return "UNKNOWN";
}*/

const char * modem_operators_string(ospl_uint32 code)
{
	ospl_uint32 i = 0;
	for(i = 0; i < sizeof(operators_table)/sizeof(operators_table[0]); i++)
	{
		if(operators_table[i].operator_code == code)
		{
			return operators_table[i].name;
		}
	}
	return "UNKNOWN";
}

const char * modem_country_string(ospl_uint32 code)
{
	ospl_uint32 i = 0;
	for(i = 0; i < sizeof(operators_table)/sizeof(operators_table[0]); i++)
	{
		if((operators_table[i].operator_code & 0xfff00) == (code << 8))
		{
			return operators_table[i].country;
		}
	}
	return "UNKNOWN";
}

const char * modem_nation_string(ospl_uint32 code)
{
	ospl_uint32 i = 0;
	for(i = 0; i < sizeof(operators_table)/sizeof(operators_table[0]); i++)
	{
		if((operators_table[i].operator_code & 0xfff00) == (code << 8))
		{
			return operators_table[i].nation;
		}
	}
	return "UNKNOWN";
}
