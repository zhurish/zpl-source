#!/bin/bash


module_node_file=$3/lib/moduletypes.c 
module_def_file=$3/lib/moduletypes.h
module_node_tmp=$3/lib/moduletypes.tmp

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

            #filename=${cur_dir}/${dirlist}
            #exittype=`echo "${dirlist##*.}"`
            #echo " ${cur_dir}/${dirlist}  $exittype"
            if test "${dirlist##*.}" = "c"
            then
               check=`grep "module_list_" ${cur_dir}/${dirlist}`
               #echo " check  --$check--"
               if test "${check}xx" != "xx"
               then
                  #echo "check-----$check--"
                  eemodule_node=`awk 'BEGIN{ORS=""} /module_list_./ {print "extern struct module_list " $3 ";\n"}' ${cur_dir}/${dirlist}`
                  module_node=`awk 'BEGIN{ORS=""} /module_list_./ {print "  &" $3 ",\n"}' ${cur_dir}/${dirlist}`
                  #module_def=`awk 'BEGIN{IGNORECASE=0;ORS=""} /MODULE_[A-Z0-9]/' ${cur_dir}/${dirlist} | awk -F'=' 'BEGIN{ORS=""} {print $2}'`
                  #module_def=`awk 'BEGIN{IGNORECASE=0;ORS=""} /MODULE_[A-Z0-9]/ {print $2}' ${cur_dir}/${dirlist}`
                  module_def=`grep =MODULE_ ${cur_dir}/${dirlist} | awk -F'=' 'BEGIN{ORS="\n"}  { if(length($2)>1) print "  " $2 }' >> $module_def_file` 
                  if test "${module_def}xx" != "xx"
                  then
                        #echo "	$module_def" >> $module_def_file
                        echo "	$module_def"
                  fi
                  if test "${module_node}xx" != "xx"
                  then
                        echo "${eemodule_node}" >> $module_node_file
                        echo "${module_node}" >> $module_node_tmp
                        
                  fi
               fi
            fi   
        fi
    done
}

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

    check=`grep "module_list_" ${scanfile}`
    if test "${check}xx" != "xx"
    then
        awk 'BEGIN{ORS=""} /module_list_./ {print "extern struct module_list " $3 ";\n"}' ${scanfile} >> $module_node_file
        awk 'BEGIN{ORS=""} /module_list_./ {print "  &" $3 ",\n"}' ${scanfile} >> $module_node_tmp
        #grep MODULE_ ${scanfile} | awk -F'=' 'BEGIN{ORS="\n"}  { if((gsub(" ",$1)==".module") && length($2)>1) print "  " $2 }' >> $module_def_file
        #awk 'BEGIN{ORS=""} /.module/&&/,/&&/MODULE_/ {print $1 "\n"}' ${scanfile} | awk -F'=' 'BEGIN{ORS="\n"} {if(length($2)>1)print "  " $2}'  >> $module_def_file
        awk 'BEGIN{ORS=""} /.module/&&/=/&&/,/&&/MODULE_/ {print  $0 "\n"}' ${scanfile}  | awk '{match($0, /MODULE_.*/,str); print "  " str[0]}' >> $module_def_file
    fi  
}

if test "header" == "$1" ;then
    rm $module_def_file $module_node_file

    echo "#ifndef _MODULE_TYPES_H" > $module_def_file
    echo "#define _MODULE_TYPES_H" >> $module_def_file
    echo " " >> $module_def_file
    echo "typedef enum {" >> $module_def_file
    echo "	MODULE_NONE = 0," >> $module_def_file

    echo "#include \"zebra.h\"" > $module_node_file
    echo "#include \"moduletypes.h\"" >> $module_node_file
    echo " " >> $module_node_file
fi

if test "tail" == "$1" ;then
    echo "	MODULE_MAX," >> $module_def_file
    echo "} module_t;"  >> $module_def_file
    echo " " >> $module_def_file
    echo "#endif /* _MODULE_TYPES_H */" >> $module_def_file

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
