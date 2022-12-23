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

#ifndef ORTP_LIST_H_
#define ORTP_LIST_H_



#ifdef __cplusplus
extern "C"{
#endif

typedef struct ortp_list {
        struct ortp_list *next;
        struct ortp_list *prev;
        void *data;
} ortp_list_t;

typedef int (*ortp_compare_func)(const void *, const void*);
typedef void (*ortp_list_iterate_func)(void *);
typedef void (*ortp_list_iterate2_func)(void *, void *);
typedef void (*ortp_list_free_func)(void *);
typedef void* (*ortp_list_copy_func)(void *);

ORTP_PUBLIC ortp_list_t * ortp_list_new(void *data);
ORTP_PUBLIC ortp_list_t * ortp_list_append(ortp_list_t * elem, void * data);
ORTP_PUBLIC ortp_list_t * ortp_list_append_link(ortp_list_t * elem, ortp_list_t *new_elem);
ORTP_PUBLIC ortp_list_t * ortp_list_prepend(ortp_list_t * elem, void * data);
ORTP_PUBLIC ortp_list_t * ortp_list_prepend_link(ortp_list_t* elem, ortp_list_t *new_elem);
ORTP_PUBLIC ortp_list_t * ortp_list_last_elem(const ortp_list_t *l);
ORTP_PUBLIC ortp_list_t * ortp_list_first_elem(const ortp_list_t *l);
ORTP_PUBLIC ortp_list_t * ortp_list_free(ortp_list_t * elem);
ORTP_PUBLIC ortp_list_t * ortp_list_concat(ortp_list_t * first, ortp_list_t * second);
ORTP_PUBLIC ortp_list_t * ortp_list_remove(ortp_list_t * first, void *data);
ORTP_PUBLIC ortp_list_t * ortp_list_remove_custom(ortp_list_t *first, ortp_compare_func compare_func, const void *user_data);
ORTP_PUBLIC ortp_list_t * ortp_list_pop_front(ortp_list_t *list, void **front_data);
ORTP_PUBLIC unsigned int ortp_list_size(const ortp_list_t * first);
ORTP_PUBLIC void ortp_list_for_each(const ortp_list_t * list, ortp_list_iterate_func func);
ORTP_PUBLIC void ortp_list_for_each2(const ortp_list_t * list, ortp_list_iterate2_func func, void *user_data);
/**
 * Removes the element pointed by elem from the list. The element itself is not freed, allowing
 * to be chained in another list for example.
 * Use ortp_list_erase_link() if you simply want to delete an element of a list.
**/
ORTP_PUBLIC ortp_list_t * ortp_list_unlink(ortp_list_t * list, ortp_list_t * elem);
/**
 * Delete the element pointed by 'elem' from the list.
**/
ORTP_PUBLIC ortp_list_t * ortp_list_erase_link(ortp_list_t * list, ortp_list_t * elem);
ORTP_PUBLIC ortp_list_t * ortp_list_find(ortp_list_t *list, const void *data);
//ORTP_PUBLIC ortp_list_t * ortp_list_free(ortp_list_t *list);
/*frees list elements and associated data, using the supplied function pointer*/
ORTP_PUBLIC ortp_list_t * ortp_list_free_with_data(ortp_list_t *list, ortp_list_free_func freefunc);

ORTP_PUBLIC ortp_list_t * ortp_list_find_custom(const ortp_list_t * list, ortp_compare_func cmp, const void *user_data);
ORTP_PUBLIC void * ortp_list_nth_data(const ortp_list_t * list, int index);
ORTP_PUBLIC int ortp_list_position(const ortp_list_t * list, ortp_list_t * elem);
ORTP_PUBLIC int ortp_list_index(const ortp_list_t * list, void *data);
ORTP_PUBLIC ortp_list_t * ortp_list_insert_sorted(ortp_list_t * list, void *data, ortp_compare_func cmp);
ORTP_PUBLIC ortp_list_t * ortp_list_insert(ortp_list_t * list, ortp_list_t * before, void *data);
ORTP_PUBLIC ortp_list_t * ortp_list_copy(const ortp_list_t * list);
/*copy list elements and associated data, using the supplied function pointer*/
ORTP_PUBLIC ortp_list_t* ortp_list_copy_with_data(const ortp_list_t* list, ortp_list_copy_func copyfunc);
/*Same as ortp_list_copy_with_data but in reverse order*/
ORTP_PUBLIC ortp_list_t* ortp_list_copy_reverse_with_data(const ortp_list_t* list, ortp_list_copy_func copyfunc);

ORTP_PUBLIC ortp_list_t* ortp_list_next(const ortp_list_t *elem);
ORTP_PUBLIC void* ortp_list_get_data(const ortp_list_t *elem);

#ifdef __cplusplus
}
#endif

#endif /* BCTLBX_LIST_H_ */