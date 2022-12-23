/*
	bctoolbox
    Copyright (C) 2016  Belledonne Communications SARL

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef HAVE_CONFIG_H
#include "ortp-config.h"
#endif

#define _CRT_RAND_S
#include <stddef.h>
#include <stdlib.h>
#include <sys/stat.h>

#ifndef _WIN32
#include <unistd.h>
#include <sys/time.h> /*for gettimeofday*/
#include <dirent.h> /* available on POSIX system only */
#else
#include <direct.h>
#endif

#include <ortp/port.h>
#include <ortp/logging.h>
#include <ortp/ortp_list.h>



ortp_list_t* ortp_list_new(void *data){
	ortp_list_t* new_elem=ortp_malloc(sizeof(ortp_list_t));
        if(new_elem)
        {
            memset(new_elem, 0, sizeof(ortp_list_t));
            new_elem->data=data;
            return new_elem;
        }
        return NULL;
}
ortp_list_t* ortp_list_next(const ortp_list_t *elem) {
	return elem->next;
}
void* ortp_list_get_data(const ortp_list_t *elem) {
	return elem->data;
}
ortp_list_t*  ortp_list_append_link(ortp_list_t* elem,ortp_list_t *new_elem){
	ortp_list_t* it=elem;
	if (elem==NULL)  return new_elem;
	if (new_elem==NULL)  return elem;
	while (it->next!=NULL) it=ortp_list_next(it);
	it->next=new_elem;
	new_elem->prev=it;
	return elem;
}

ortp_list_t*  ortp_list_append(ortp_list_t* elem, void * data){
	ortp_list_t* new_elem=ortp_list_new(data);
	return ortp_list_append_link(elem,new_elem);
}

ortp_list_t*  ortp_list_prepend_link(ortp_list_t* elem, ortp_list_t *new_elem){
	if (elem!=NULL) {
		new_elem->next=elem;
		elem->prev=new_elem;
	}
	return new_elem;
}

ortp_list_t*  ortp_list_prepend(ortp_list_t* elem, void *data){
	return ortp_list_prepend_link(elem,ortp_list_new(data));
}

ortp_list_t * ortp_list_last_elem(const ortp_list_t *l){
	if (l==NULL) return NULL;
	while(l->next){
		l=l->next;
	}
	return (ortp_list_t*)l;
}

ortp_list_t * ortp_list_first_elem(const ortp_list_t *l){
	if (l==NULL) return NULL;
	while(l->prev){
		l=l->prev;
	}
	return (ortp_list_t*)l;
}

ortp_list_t*  ortp_list_concat(ortp_list_t* first, ortp_list_t* second){
	ortp_list_t* it=first;
	if (it==NULL) return second;
	if (second==NULL) return first;
	while(it->next!=NULL) it=ortp_list_next(it);
	it->next=second;
	second->prev=it;
	return first;
}

ortp_list_t*  ortp_list_free(ortp_list_t* list){
	ortp_list_t* elem = list;
	ortp_list_t* tmp;
	if (list==NULL) return NULL;
	while(elem->next!=NULL) {
		tmp = elem;
		elem = elem->next;
		ortp_free(tmp);
	}
	ortp_free(elem);
	return NULL;
}

ortp_list_t * ortp_list_free_with_data(ortp_list_t *list, ortp_list_free_func freefunc){
	ortp_list_t* elem = list;
	ortp_list_t* tmp;
	if (list==NULL) return NULL;
	while(elem->next!=NULL) {
		tmp = elem;
		elem = elem->next;
		freefunc(tmp->data);
		ortp_free(tmp);
	}
	freefunc(elem->data);
	ortp_free(elem);
	return NULL;
}


static ortp_list_t*  _ortp_list_remove(ortp_list_t* first, void *data, int warn_if_not_found){
	ortp_list_t* it;
	it=ortp_list_find(first,data);
	if (it) return ortp_list_erase_link(first,it);
	else if (warn_if_not_found){
		ortp_warning("ortp_list_remove: no element with %p data was in the list", data);
	}
	return first;
}

ortp_list_t*  ortp_list_remove(ortp_list_t* first, void *data){
        return _ortp_list_remove(first, data, TRUE);
}

ortp_list_t * ortp_list_remove_custom(ortp_list_t *first, ortp_compare_func compare_func, const void *user_data) {
	ortp_list_t *cur;
	ortp_list_t *elem = first;
	while (elem != NULL) {
		cur = elem;
		elem = elem->next;
		if (compare_func(cur->data, user_data) == 0) {
			first = ortp_list_remove(first, cur->data);
		}
	}
	return first;
}

unsigned int ortp_list_size(const ortp_list_t* first){
	unsigned int n=0;
	while(first!=NULL){
		++n;
		first=first->next;
	}
	return n;
}

void ortp_list_for_each(const ortp_list_t* list, ortp_list_iterate_func func){
	for(;list!=NULL;list=list->next){
		func(list->data);
	}
}

void ortp_list_for_each2(const ortp_list_t* list, ortp_list_iterate2_func func, void *user_data){
	for(;list!=NULL;list=list->next){
		func(list->data,user_data);
	}
}

ortp_list_t * ortp_list_pop_front(ortp_list_t *list, void **front_data){
	ortp_list_t *front_elem=list;
	if (front_elem==NULL){
		*front_data=NULL;
		return NULL;
	}
	*front_data=front_elem->data;
	list=ortp_list_unlink(list,front_elem);
	ortp_free(front_elem);
	return list;
}

ortp_list_t* ortp_list_unlink(ortp_list_t* list, ortp_list_t* elem){
	ortp_list_t* ret;
	if (elem==list){
		ret=elem->next;
		elem->prev=NULL;
		elem->next=NULL;
		if (ret!=NULL) ret->prev=NULL;
		return ret;
	}
	elem->prev->next=elem->next;
	if (elem->next!=NULL) elem->next->prev=elem->prev;
	elem->next=NULL;
	elem->prev=NULL;
	return list;
}

ortp_list_t * ortp_list_erase_link(ortp_list_t* list, ortp_list_t* elem){
	ortp_list_t *ret=ortp_list_unlink(list,elem);
	ortp_free(elem);
	return ret;
}

ortp_list_t* ortp_list_find(ortp_list_t* list, const void *data){
	for(;list!=NULL;list=list->next){
		if (list->data==data) return list;
	}
	return NULL;
}

ortp_list_t* ortp_list_find_custom(const ortp_list_t* list, ortp_compare_func compare_func, const void *user_data){
	for(;list!=NULL;list=list->next){
		if (compare_func(list->data,user_data)==0) return (ortp_list_t *)list;
	}
	return NULL;
}
/*
ortp_list_t *ortp_list_delete_custom(ortp_list_t *list, ortp_compare_func compare_func, const void *user_data){
	ortp_list_t *elem=ortp_list_find_custom(list,compare_func,user_data);
	if (elem!=NULL){
		list=ortp_list_erase_link(list,elem);
	}
	return list;
}
*/

void * ortp_list_nth_data(const ortp_list_t* list, int index){
	int i;
	for(i=0;list!=NULL;list=list->next,++i){
		if (i==index) return list->data;
	}
	ortp_error("ortp_list_nth_data: no such index in list.");
	return NULL;
}

int ortp_list_position(const ortp_list_t* list, ortp_list_t* elem){
	int i;
	for(i=0;list!=NULL;list=list->next,++i){
		if (elem==list) return i;
	}
	ortp_error("ortp_list_position: no such element in list.");
	return -1;
}

int ortp_list_index(const ortp_list_t* list, void *data){
	int i;
	for(i=0;list!=NULL;list=list->next,++i){
		if (data==list->data) return i;
	}
	ortp_error("ortp_list_index: no such element in list.");
	return -1;
}

ortp_list_t* ortp_list_insert_sorted(ortp_list_t* list, void *data, ortp_compare_func compare_func){
	ortp_list_t* it,*previt=NULL;
	ortp_list_t* nelem;
	ortp_list_t* ret=list;
	if (list==NULL) return ortp_list_append(list,data);
	else{
		nelem=ortp_list_new(data);
		for(it=list;it!=NULL;it=it->next){
			previt=it;
			if (compare_func(data,it->data)<=0){
				nelem->prev=it->prev;
				nelem->next=it;
				if (it->prev!=NULL)
					it->prev->next=nelem;
				else{
					ret=nelem;
				}
				it->prev=nelem;
				return ret;
			}
		}
		previt->next=nelem;
		nelem->prev=previt;
	}
	return ret;
}

ortp_list_t* ortp_list_insert(ortp_list_t* list, ortp_list_t* before, void *data){
	ortp_list_t* elem;
	if (list==NULL || before==NULL) return ortp_list_append(list,data);
	for(elem=list;elem!=NULL;elem=ortp_list_next(elem)){
		if (elem==before){
			if (elem->prev==NULL)
				return ortp_list_prepend(list,data);
			else{
				ortp_list_t* nelem=ortp_list_new(data);
				nelem->prev=elem->prev;
				nelem->next=elem;
				elem->prev->next=nelem;
				elem->prev=nelem;
			}
		}
	}
	return list;
}

ortp_list_t* ortp_list_copy(const ortp_list_t* list){
	ortp_list_t* copy=NULL;
	const ortp_list_t* iter;
	for(iter=list;iter!=NULL;iter=ortp_list_next(iter)){
		copy=ortp_list_append(copy,iter->data);
	}
	return copy;
}

ortp_list_t* ortp_list_copy_with_data(const ortp_list_t* list, ortp_list_copy_func copyfunc){
	ortp_list_t* copy=NULL;
	const ortp_list_t* iter;
	for(iter=list;iter!=NULL;iter=ortp_list_next(iter)){
		copy=ortp_list_append(copy,copyfunc(iter->data));
	}
	return copy;
}

ortp_list_t* ortp_list_copy_reverse_with_data(const ortp_list_t* list, ortp_list_copy_func copyfunc){
        ortp_list_t* copy=NULL;
        const ortp_list_t* iter;
        for(iter=list;iter!=NULL;iter=ortp_list_next(iter)){
                copy=ortp_list_prepend(copy,copyfunc(iter->data));
        }
        return copy;
}
