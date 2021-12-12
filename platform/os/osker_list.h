
#ifndef __OSKER_LIST_H__
#define __OSKER_LIST_H__

#ifdef	__cplusplus
extern "C" {
#endif	/* __cplusplus */



#ifndef INLINE
#define INLINE //__inline__
#endif



/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#ifndef _offsetof
#define _offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif
#ifndef container_of
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - _offsetof(type,member) );})
#endif	


/*
 * Simple doubly linked osker_list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole osker_lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

typedef struct osker_list_head {
	struct osker_list_head *next, *prev;
}osker_list_head_t;


#define OSKER_LIST_HEAD_INIT(name) { &(name), &(name) }
/*
#define OSKER_LIST_HEAD(name) \
	struct osker_list_head name = OSKER_LIST_HEAD_INIT(name)*/

 INLINE void INIT_OSKER_LIST_HEAD(struct osker_list_head *osker_list);

/*
 * Insert a newlst entry between two known consecutive entries.
 *
 * This is only for internal osker_list manipulation where we know
 * the prev/next entries already!
 */
 INLINE void __osker_list_add(struct osker_list_head *newlst,
			      struct osker_list_head *prev,
			      struct osker_list_head *next);


/**
 * osker_list_add - add a newlst entry
 * @newlst: newlst entry to be added
 * @head: osker_list head to add it after
 *
 * Insert a newlst entry after the specified head.
 * This is good for implementing stacks.
 */
 INLINE void osker_list_add(struct osker_list_head *newlst, struct osker_list_head *head);

/**
 * osker_list_add_tail - add a newlst entry
 * @newlst: newlst entry to be added
 * @head: osker_list head to add it before
 *
 * Insert a newlst entry before the specified head.
 * This is useful for implementing queues.
 */
INLINE void osker_list_add_tail(struct osker_list_head *newlst, struct osker_list_head *head);
/*
 * Delete a osker_list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal osker_list manipulation where we know
 * the prev/next entries already!
 */
INLINE void __osker_list_del(struct osker_list_head * prev, struct osker_list_head * next);

/**
 * osker_list_del - deletes entry from osker_list.
 * @entry: the element to delete from the osker_list.
 * Note: osker_list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */

INLINE void osker_list_del(struct osker_list_head *entry);

/**
 * osker_list_replace - replace old entry by newlst one
 * @old : the element to be replaced
 * @newlst : the newlst element to insert
 *
 * If @old was empty, it will be overwritten.
 */
 INLINE void osker_list_replace(struct osker_list_head *old,
				struct osker_list_head *newlst);
 INLINE void osker_list_replace_init(struct osker_list_head *old,
					struct osker_list_head *newlst);

/**
 * osker_list_del_init - deletes entry from osker_list and reinitialize it.
 * @entry: the element to delete from the osker_list.
 */
INLINE void osker_list_del_init(struct osker_list_head *entry);
/**
 * osker_list_move - delete from one osker_list and add as another's head
 * @osker_list: the entry to move
 * @head: the head that will precede our entry
 */
 INLINE void osker_list_move(struct osker_list_head *osker_list, struct osker_list_head *head);
/**
 * osker_list_move_tail - delete from one osker_list and add as another's tail
 * @osker_list: the entry to move
 * @head: the head that will follow our entry
 */
INLINE void osker_list_move_tail(struct osker_list_head *osker_list,
				  struct osker_list_head *head);

/**
 * osker_list_is_last - tests whether @osker_list is the last entry in osker_list @head
 * @osker_list: the entry to test
 * @head: the head of the osker_list
 */
INLINE int osker_list_is_last(const struct osker_list_head *osker_list,
				const struct osker_list_head *head);
/**
 * osker_list_empty - tests whether a osker_list is empty
 * @head: the osker_list to test.
 */
INLINE int osker_list_empty(const struct osker_list_head *head);

/**
 * osker_list_empty_careful - tests whether a osker_list is empty and not being modified
 * @head: the osker_list to test
 *
 * Description:
 * tests whether a osker_list is empty _and_ checks that no other CPU might be
 * in the process of modifying either member (next or prev)
 *
 * NOTE: using osker_list_empty_careful() without synchronization
 * can only be safe if the only activity that can happen
 * to the osker_list entry is osker_list_del_init(). Eg. it cannot be used
 * if another CPU could re-osker_list_add() it.
 */
INLINE int osker_list_empty_careful(const struct osker_list_head *head);

/**
 * osker_list_is_singular - tests whether a osker_list has just one entry.
 * @head: the osker_list to test.
 */
INLINE int osker_list_is_singular(const struct osker_list_head *head);

INLINE void __osker_list_cut_position(struct osker_list_head *osker_list,
		struct osker_list_head *head, struct osker_list_head *entry);
/**
 * osker_list_cut_position - cut a osker_list into two
 * @osker_list: a newlst osker_list to add all removed entries
 * @head: a osker_list with entries
 * @entry: an entry within head, could be the head itself
 *	and if so we won't cut the osker_list
 *
 * This helper moves the initial part of @head, up to and
 * including @entry, from @head to @osker_list. You should
 * pass on @entry an element you know is on @head. @osker_list
 * should be an empty osker_list or a osker_list you do not care about
 * losing its data.
 *
 */
INLINE void osker_list_cut_position(struct osker_list_head *osker_list,
		struct osker_list_head *head, struct osker_list_head *entry);
INLINE void __osker_list_splice(const struct osker_list_head *osker_list,
				 struct osker_list_head *prev,
				 struct osker_list_head *next);
/**
 * osker_list_splice - join two osker_lists, this is designed for stacks
 * @osker_list: the newlst osker_list to add.
 * @head: the place to add it in the first osker_list.
 */
INLINE void osker_list_splice(const struct osker_list_head *osker_list,
				struct osker_list_head *head);
/**
 * osker_list_splice_tail - join two osker_lists, each osker_list being a queue
 * @osker_list: the newlst osker_list to add.
 * @head: the place to add it in the first osker_list.
 */
INLINE void osker_list_splice_tail(struct osker_list_head *osker_list,
				struct osker_list_head *head);
/**
 * osker_list_splice_init - join two osker_lists and reinitialise the emptied osker_list.
 * @osker_list: the newlst osker_list to add.
 * @head: the place to add it in the first osker_list.
 *
 * The osker_list at @osker_list is reinitialised
 */
INLINE void osker_list_splice_init(struct osker_list_head *osker_list,
				    struct osker_list_head *head);
/**
 * osker_list_splice_tail_init - join two osker_lists and reinitialise the emptied osker_list
 * @osker_list: the newlst osker_list to add.
 * @head: the place to add it in the first osker_list.
 *
 * Each of the osker_lists is a queue.
 * The osker_list at @osker_list is reinitialised
 */
INLINE void osker_list_splice_tail_init(struct osker_list_head *osker_list,
					 struct osker_list_head *head);
/**
 * osker_list_entry - get the struct for this entry
 * @ptr:	the &struct osker_list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the osker_list_struct within the struct.
 */
#define osker_list_entry(ptr, type, member) \
	container_of(ptr, type, member)

/**
 * osker_list_first_entry - get the first element from a osker_list
 * @ptr:	the osker_list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the osker_list_struct within the struct.
 *
 * Note, that osker_list is expected to be not empty.
 */
#define osker_list_first_entry(ptr, type, member) \
	osker_list_entry((ptr)->next, type, member)

/**
 * osker_list_for_each	-	iterate over a osker_list
 * @pos:	the &struct osker_list_head to use as a loop cursor.
 * @head:	the head for your osker_list.
 */
#define osker_list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); \
        	pos = pos->next)

/**
 * __osker_list_for_each	-	iterate over a osker_list
 * @pos:	the &struct osker_list_head to use as a loop cursor.
 * @head:	the head for your osker_list.
 *
 * This variant differs from osker_list_for_each() in that it's the
 * simplest possible osker_list iteration code, no prefetching is done.
 * Use this for code that knows the osker_list to be very zpl_int16 (empty
 * or 1 entry) most of the time.
 */
#define __osker_list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * osker_list_for_each_prev	-	iterate over a osker_list backwards
 * @pos:	the &struct osker_list_head to use as a loop cursor.
 * @head:	the head for your osker_list.
 */
#define osker_list_for_each_prev(pos, head) \
	for (pos = (head)->prev;  pos != (head); \
        	pos = pos->prev)

/**
 * osker_list_for_each_safe - iterate over a osker_list safe against removal of osker_list entry
 * @pos:	the &struct osker_list_head to use as a loop cursor.
 * @n:		another &struct osker_list_head to use as temporary storage
 * @head:	the head for your osker_list.
 */
#define osker_list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

/**
 * osker_list_for_each_prev_safe - iterate over a osker_list backwards safe against removal of osker_list entry
 * @pos:	the &struct osker_list_head to use as a loop cursor.
 * @n:		another &struct osker_list_head to use as temporary storage
 * @head:	the head for your osker_list.
 */
#define osker_list_for_each_prev_safe(pos, n, head) \
	for (pos = (head)->prev, n = pos->prev; \
	     pos != (head); \
	     pos = n, n = pos->prev)

/**
 * osker_list_for_each_entry	-	iterate over osker_list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your osker_list.
 * @member:	the name of the osker_list_struct within the struct.
 */
#define osker_list_for_each_entry(pos, head, member)				\
	for (pos = osker_list_entry((head)->next, __typeof__(*pos), member);	\
	     &pos->member != (head); 	\
	     pos = osker_list_entry(pos->member.next, __typeof__(*pos), member))

/**
 * osker_list_for_each_entry_reverse - iterate backwards over osker_list of given type.
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your osker_list.
 * @member:	the name of the osker_list_struct within the struct.
 */
#define osker_list_for_each_entry_reverse(pos, head, member)			\
	for (pos = osker_list_entry((head)->prev, __typeof__(*pos), member);	\
	     &pos->member != (head); 	\
	     pos = osker_list_entry(pos->member.prev, __typeof__(*pos), member))

/**
 * osker_list_prepare_entry - prepare a pos entry for use in osker_list_for_each_entry_continue()
 * @pos:	the type * to use as a start point
 * @head:	the head of the osker_list
 * @member:	the name of the osker_list_struct within the struct.
 *
 * Prepares a pos entry for use as a start point in osker_list_for_each_entry_continue().
 */
#define osker_list_prepare_entry(pos, head, member) \
	((pos) ? : osker_list_entry(head, __typeof__(*pos), member))

/**
 * osker_list_for_each_entry_continue - continue iteration over osker_list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your osker_list.
 * @member:	the name of the osker_list_struct within the struct.
 *
 * Continue to iterate over osker_list of given type, continuing after
 * the current position.
 */
#define osker_list_for_each_entry_continue(pos, head, member) 		\
	for (pos = osker_list_entry(pos->member.next, __typeof__(*pos), member);	\
	     &pos->member != (head);	\
	     pos = osker_list_entry(pos->member.next, __typeof__(*pos), member))

/**
 * osker_list_for_each_entry_continue_reverse - iterate backwards from the given point
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your osker_list.
 * @member:	the name of the osker_list_struct within the struct.
 *
 * Start to iterate over osker_list of given type backwards, continuing after
 * the current position.
 */
#define osker_list_for_each_entry_continue_reverse(pos, head, member)		\
	for (pos = osker_list_entry(pos->member.prev, __typeof__(*pos), member);	\
	     &pos->member != (head);	\
	     pos = osker_list_entry(pos->member.prev, __typeof__(*pos), member))

/**
 * osker_list_for_each_entry_from - iterate over osker_list of given type from the current point
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your osker_list.
 * @member:	the name of the osker_list_struct within the struct.
 *
 * Iterate over osker_list of given type, continuing from current position.
 */
#define osker_list_for_each_entry_from(pos, head, member) 			\
	for (; &pos->member != (head);	\
	     pos = osker_list_entry(pos->member.next, __typeof__(*pos), member))

/**
 * osker_list_for_each_entry_safe - iterate over osker_list of given type safe against removal of osker_list entry
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your osker_list.
 * @member:	the name of the osker_list_struct within the struct.
 */
#define osker_list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = osker_list_entry((head)->next, __typeof__(*pos), member),	\
		n = osker_list_entry(pos->member.next, __typeof__(*pos), member);	\
	     &pos->member != (head); 					\
	     pos = n, n = osker_list_entry(n->member.next, __typeof__(*n), member))

/**
 * osker_list_for_each_entry_safe_continue
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your osker_list.
 * @member:	the name of the osker_list_struct within the struct.
 *
 * Iterate over osker_list of given type, continuing after current point,
 * safe against removal of osker_list entry.
 */
#define osker_list_for_each_entry_safe_continue(pos, n, head, member) 		\
	for (pos = osker_list_entry(pos->member.next, __typeof__(*pos), member), 		\
		n = osker_list_entry(pos->member.next, __typeof__(*pos), member);		\
	     &pos->member != (head);						\
	     pos = n, n = osker_list_entry(n->member.next, __typeof__(*n), member))

/**
 * osker_list_for_each_entry_safe_from
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your osker_list.
 * @member:	the name of the osker_list_struct within the struct.
 *
 * Iterate over osker_list of given type from current point, safe against
 * removal of osker_list entry.
 */
#define osker_list_for_each_entry_safe_from(pos, n, head, member) 			\
	for (n = osker_list_entry(pos->member.next, __typeof__(*pos), member);		\
	     &pos->member != (head);						\
	     pos = n, n = osker_list_entry(n->member.next, __typeof__(*n), member))

/**
 * osker_list_for_each_entry_safe_reverse
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your osker_list.
 * @member:	the name of the osker_list_struct within the struct.
 *
 * Iterate backwards over osker_list of given type, safe against removal
 * of osker_list entry.
 */
#define osker_list_for_each_entry_safe_reverse(pos, n, head, member)		\
	for (pos = osker_list_entry((head)->prev, __typeof__(*pos), member),	\
		n = osker_list_entry(pos->member.prev, __typeof__(*pos), member);	\
	     &pos->member != (head); 					\
	     pos = n, n = osker_list_entry(n->member.prev, __typeof__(*n), member))

#ifdef	__cplusplus
}
#endif	/* __cplusplus */

#endif  /* __OSKER_LIST_H__ */
