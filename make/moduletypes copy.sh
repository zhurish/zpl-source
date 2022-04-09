#!/bin/bash

module_node_file=$3/nsm/moduletable.c 

module_node_file=$3/lib/moduletypes.c 
module_def_file=$3/lib/moduletypes.h

module_node_tmp=$3/lib/moduletypes.tmp
module_table_tmp=$3/lib/moduletable.tmp

function awk_scanfile() {
    local scanfile
    scanfile=$1
    if test "${scanfile}" = "moduletypes.sh"
    then
        return;
    fi
    if test "${scanfile}" = "moduletypes.h"
     then
        return;
    fi
    if test "${scanfile}" = "moduletypes.c"
    then
        return;
    fi
    if test "${scanfile}" = "module.h"
    then
        return;
    fi
    if [ -e ${scanfile} ]
    then
        check=`grep -m 1 "module_list_" ${scanfile}`
        if test "${check}xx" != "xx"
        then
            if  [ "${scanfile}" = "module.c" ] || [ "${scanfile}" = "vty.c" ]
            then
            awk 'BEGIN{ORS=""} /module_list_./ {print "extern struct module_list " $3 ";\n";}' ${scanfile} >> $module_node_file
            awk 'BEGIN{ORS=""} /module_list_./ {print "  &" $3 ",\n";}' ${scanfile} >> $module_node_tmp
            #grep MODULE_ ${scanfile} | awk -F'=' 'BEGIN{ORS="\n"}  { if((gsub(" ",$1)==".module") && length($2)>1) print "  " $2 }' >> $module_def_file
            #awk 'BEGIN{ORS=""} /.module/&&/,/&&/MODULE_/ {print $1 "\n"}' ${scanfile} | awk -F'=' 'BEGIN{ORS="\n"} {if(length($2)>1)print "  " $2}'  >> $module_def_file
            awk 'BEGIN{ORS=""} /.module/&&/=/&&/,/&&/MODULE_/ {print  $0 "\n";}' ${scanfile}  | awk '{match($0, /MODULE_.*/,str); print "  " str[0];}' >> $module_def_file
            awk 'BEGIN{ORS=""} /.module/&&/=/&&/,/&&/MODULE_/ {print  $0 "\n";}' ${scanfile}  | awk '{match($0, /MODULE_.*,/,str); print "" str[0];}'|awk '{sub(/.{1}$/,"")}1'| awk -F "_" '{print "ZPL_MODULE_DESC("$2"),"}' >> $module_table_tmp
            #awk 'BEGIN{ORS=""} /.module/&&/=/&&/,/&&/MODULE_/ {print  $0 "\n";}' ${scanfile}  | awk '{match($0, /MODULE_.*/,str); print "ZPL_MODULE_DESC("str[0]"),";}' >> $module_table_tmp

            else    
            awk 'BEGIN{ORS=""} /module_list_./ {print "extern struct module_list " $3 ";\n";exit;}' ${scanfile} >> $module_node_file
            awk 'BEGIN{ORS=""} /module_list_./ {print "  &" $3 ",\n";exit;}' ${scanfile} >> $module_node_tmp
            #grep MODULE_ ${scanfile} | awk -F'=' 'BEGIN{ORS="\n"}  { if((gsub(" ",$1)==".module") && length($2)>1) print "  " $2 }' >> $module_def_file
            #awk 'BEGIN{ORS=""} /.module/&&/,/&&/MODULE_/ {print $1 "\n"}' ${scanfile} | awk -F'=' 'BEGIN{ORS="\n"} {if(length($2)>1)print "  " $2}'  >> $module_def_file
            awk 'BEGIN{ORS=""} /.module/&&/=/&&/,/&&/MODULE_/ {print  $0 "\n";exit;}' ${scanfile}  | awk '{match($0, /MODULE_.*/,str); print "  " str[0];exit;}' >> $module_def_file
            awk 'BEGIN{ORS=""} /.module/&&/=/&&/,/&&/MODULE_/ {print  $0 "\n";exit;}' ${scanfile}  | awk '{match($0, /MODULE_.*,/,str); print "" str[0];exit;}'|awk '{sub(/.{1}$/,"")}1'| awk -F "_" '{print "ZPL_MODULE_DESC("$2"),"}' >> $module_table_tmp
            #awk 'BEGIN{ORS=""} /.module/&&/=/&&/,/&&/MODULE_/ {print  $0 "\n";exit;}' ${scanfile}  | awk '{match($0, /MODULE_.*/,str); print "ZPL_MODULE_DESC("str[0]"),";exit;}' >> $module_table_tmp
            fi
        fi
    fi 
}

function awk_scandir() {
    local cur_dir parent_dir workdir
    workdir=$1
    cd ${workdir}
    #if [ ${workdir} = "/" ]
    #then
    #    cur_dir=""
    #else
        cur_dir=$(pwd)
    #fi

    for dirlist in $(ls ${cur_dir})
    do
        if test -d ${dirlist};then
            cd ${dirlist}
            awk_scandir ${cur_dir}/${dirlist}
            cd ..
        else
            #echo ${cur_dir}/${dirlist}
            if test "${dirlist}" = "moduletypes.sh"
            then
                continue;
            fi
            if test "${dirlist}" = "moduletypes.h"
            then
                continue;
            fi
            if test "${dirlist}" = "moduletypes.c"
            then
                continue;
            fi
            if test "${dirlist}" = "module.h"
            then
                continue;
            fi
            awk_scanfile ${cur_dir}/${dirlist}
        fi
    done
}

if test "header" == "$1" ;then
    rm $module_def_file $module_node_file $module_table_tmp

    echo "#ifndef _MODULE_TYPES_H" > $module_def_file
    echo "#define _MODULE_TYPES_H" >> $module_def_file
    echo " " >> $module_def_file
    echo "typedef enum {" >> $module_def_file
    echo "	MODULE_NONE = 0," >> $module_def_file
    echo "#include \"auto_include.h\"" > $module_node_file
    echo "#include \"zplos_include.h\"" > $module_node_file
    echo "#include \"module.h\"" >> $module_node_file
    echo "#include \"moduletypes.h\"" >> $module_node_file
    echo " " >> $module_node_file

    echo " " >> $module_node_file
    echo "#define ZPL_MODULE_DESC(m)    {(MODULE_ ##m),(#m),0 }" >> $module_node_file
    echo " " >> $module_node_file
fi


if test "tail" == "$1" ;then
    echo "	MODULE_MAX," >> $module_def_file
    echo "} module_t;"  >> $module_def_file
    echo " " >> $module_def_file
    echo "#endif /* _MODULE_TYPES_H */" >> $module_def_file

    echo " " >> $module_node_file
    echo "struct module_table module_tbl[MODULE_MAX] = {" >> $module_node_file
    cat $module_table_tmp >> $module_node_file
    echo "};"  >> $module_node_file
    echo "#undef ZPL_MODULE_DESC" >> $module_node_file

    echo " " >> $module_node_file
    echo "struct module_alllist module_lists_tbl[MODULE_MAX] = {" >> $module_node_file
    cat $module_node_tmp >> $module_node_file
    echo " NULL,"  >> $module_node_file
    echo "};"  >> $module_node_file
    rm $module_node_tmp
fi

if test "def" == "$1" ;then
    awk_scandir $2
fi

if test "scan" == "$1" ;then
    awk_scanfile $2
fi
