
#include "osker_list.h"



 INLINE void INIT_OSKER_LIST_HEAD(struct osker_list_head *osker_list)
{
	osker_list->next = osker_list;
	osker_list->prev = osker_list;
}

/*
 * Insert a newlst entry between two known consecutive entries.
 *
 * This is only for internal osker_list manipulation where we know
 * the prev/next entries already!
 */
 INLINE void __osker_list_add(struct osker_list_head *newlst,
			      struct osker_list_head *prev,
			      struct osker_list_head *next)
{
	next->prev = newlst;
	newlst->next = next;
	newlst->prev = prev;
	prev->next = newlst;
}


/**
 * osker_list_add - add a newlst entry
 * @newlst: newlst entry to be added
 * @head: osker_list head to add it after
 *
 * Insert a newlst entry after the specified head.
 * This is good for implementing stacks.
 */
 INLINE void osker_list_add(struct osker_list_head *newlst, struct osker_list_head *head)
{
	__osker_list_add(newlst, head, head->next);
}


/**
 * osker_list_add_tail - add a newlst entry
 * @newlst: newlst entry to be added
 * @head: osker_list head to add it before
 *
 * Insert a newlst entry before the specified head.
 * This is useful for implementing queues.
 */
 INLINE void osker_list_add_tail(struct osker_list_head *newlst, struct osker_list_head *head)
{
	__osker_list_add(newlst, head->prev, head);
}

/*
 * Delete a osker_list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal osker_list manipulation where we know
 * the prev/next entries already!
 */
 INLINE void __osker_list_del(struct osker_list_head * prev, struct osker_list_head * next)
{	
	if(next)
		next->prev = prev;
	if(prev)
		prev->next = next;
}

/**
 * osker_list_del - deletes entry from osker_list.
 * @entry: the element to delete from the osker_list.
 * Note: osker_list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */

 INLINE void osker_list_del(struct osker_list_head *entry)
{
	__osker_list_del(entry->prev, entry->next);
}

/**
 * osker_list_replace - replace old entry by newlst one
 * @old : the element to be replaced
 * @newlst : the newlst element to insert
 *
 * If @old was empty, it will be overwritten.
 */
 INLINE void osker_list_replace(struct osker_list_head *old,
				struct osker_list_head *newlst)
{
	newlst->next = old->next;
	newlst->next->prev = newlst;
	newlst->prev = old->prev;
	newlst->prev->next = newlst;
}

 INLINE void osker_list_replace_init(struct osker_list_head *old,
					struct osker_list_head *newlst)
{
	osker_list_replace(old, newlst);
	INIT_OSKER_LIST_HEAD(old);
}

/**
 * osker_list_del_init - deletes entry from osker_list and reinitialize it.
 * @entry: the element to delete from the osker_list.
 */
 INLINE void osker_list_del_init(struct osker_list_head *entry)
{
	__osker_list_del(entry->prev, entry->next);
	INIT_OSKER_LIST_HEAD(entry);
}

/**
 * osker_list_move - delete from one osker_list and add as another's head
 * @osker_list: the entry to move
 * @head: the head that will precede our entry
 */
 INLINE void osker_list_move(struct osker_list_head *osker_list, struct osker_list_head *head)
{
	__osker_list_del(osker_list->prev, osker_list->next);
	osker_list_add(osker_list, head);
}

/**
 * osker_list_move_tail - delete from one osker_list and add as another's tail
 * @osker_list: the entry to move
 * @head: the head that will follow our entry
 */
 INLINE void osker_list_move_tail(struct osker_list_head *osker_list,
				  struct osker_list_head *head)
{
	__osker_list_del(osker_list->prev, osker_list->next);
	osker_list_add_tail(osker_list, head);
}

/**
 * osker_list_is_last - tests whether @osker_list is the last entry in osker_list @head
 * @osker_list: the entry to test
 * @head: the head of the osker_list
 */
 INLINE int osker_list_is_last(const struct osker_list_head *osker_list,
				const struct osker_list_head *head)
{
	return osker_list->next == head;
}

/**
 * osker_list_empty - tests whether a osker_list is empty
 * @head: the osker_list to test.
 */
 INLINE int osker_list_empty(const struct osker_list_head *head)
{
	return head->next == head;
}

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
 INLINE int osker_list_empty_careful(const struct osker_list_head *head)
{
	struct osker_list_head *next = head->next;
	return (next == head) && (next == head->prev);
}

/**
 * osker_list_is_singular - tests whether a osker_list has just one entry.
 * @head: the osker_list to test.
 */
 INLINE int osker_list_is_singular(const struct osker_list_head *head)
{
	return !osker_list_empty(head) && (head->next == head->prev);
}

 INLINE void __osker_list_cut_position(struct osker_list_head *osker_list,
		struct osker_list_head *head, struct osker_list_head *entry)
{
	struct osker_list_head *newlst_first = entry->next;
	osker_list->next = head->next;
	osker_list->next->prev = osker_list;
	osker_list->prev = entry;
	entry->next = osker_list;
	head->next = newlst_first;
	newlst_first->prev = head;
}

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
		struct osker_list_head *head, struct osker_list_head *entry)
{
	if (osker_list_empty(head))
		return;
	if (osker_list_is_singular(head) &&
		(head->next != entry && head != entry))
		return;
	if (entry == head)
		INIT_OSKER_LIST_HEAD(osker_list);
	else
		__osker_list_cut_position(osker_list, head, entry);
}

 INLINE void __osker_list_splice(const struct osker_list_head *osker_list,
				 struct osker_list_head *prev,
				 struct osker_list_head *next)
{
	struct osker_list_head *first = osker_list->next;
	struct osker_list_head *last = osker_list->prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

/**
 * osker_list_splice - join two osker_lists, this is designed for stacks
 * @osker_list: the newlst osker_list to add.
 * @head: the place to add it in the first osker_list.
 */
 INLINE void osker_list_splice(const struct osker_list_head *osker_list,
				struct osker_list_head *head)
{
	if (!osker_list_empty(osker_list))
		__osker_list_splice(osker_list, head, head->next);
}

/**
 * osker_list_splice_tail - join two osker_lists, each osker_list being a queue
 * @osker_list: the newlst osker_list to add.
 * @head: the place to add it in the first osker_list.
 */
 INLINE void osker_list_splice_tail(struct osker_list_head *osker_list,
				struct osker_list_head *head)
{
	if (!osker_list_empty(osker_list))
		__osker_list_splice(osker_list, head->prev, head);
}

/**
 * osker_list_splice_init - join two osker_lists and reinitialise the emptied osker_list.
 * @osker_list: the newlst osker_list to add.
 * @head: the place to add it in the first osker_list.
 *
 * The osker_list at @osker_list is reinitialised
 */
 INLINE void osker_list_splice_init(struct osker_list_head *osker_list,
				    struct osker_list_head *head)
{
	if (!osker_list_empty(osker_list)) {
		__osker_list_splice(osker_list, head, head->next);
		INIT_OSKER_LIST_HEAD(osker_list);
	}
}

/**
 * osker_list_splice_tail_init - join two osker_lists and reinitialise the emptied osker_list
 * @osker_list: the newlst osker_list to add.
 * @head: the place to add it in the first osker_list.
 *
 * Each of the osker_lists is a queue.
 * The osker_list at @osker_list is reinitialised
 */
 INLINE void osker_list_splice_tail_init(struct osker_list_head *osker_list,
					 struct osker_list_head *head)
{
	if (!osker_list_empty(osker_list)) {
		__osker_list_splice(osker_list, head->prev, head);
		INIT_OSKER_LIST_HEAD(osker_list);
	}
}
