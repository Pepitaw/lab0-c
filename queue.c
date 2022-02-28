#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "harness.h"
#include "shuffle.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *q = malloc(sizeof(struct list_head));
    if (!q)
        return NULL;
    INIT_LIST_HEAD(q);
    return q;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;
    element_t *pos, *n;
    list_for_each_entry_safe (pos, n, l, list)
        q_release_element(pos);
    free(l);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *e = malloc(sizeof(element_t));
    if (!e)
        return false;
    e->value = strdup(s);
    if (!e->value) {
        free(e);
        return false;
    }
    list_add(&e->list, head);
    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *e = malloc(sizeof(element_t));
    if (!e)
        return false;
    e->value = strdup(s);
    if (!e->value) {
        free(e);
        return false;
    }
    list_add_tail(&e->list, head);
    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || head->next == head)
        return NULL;
    element_t *e = list_first_entry(head, element_t, list);
    list_del(&e->list);
    if (sp) {
        strncpy(sp, e->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return e;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || head->next == head)
        return NULL;
    element_t *e = list_last_entry(head, element_t, list);
    list_del(&e->list);
    if (sp) {
        strncpy(sp, e->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return e;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head)
        return false;
    int i = 0;
    struct list_head *lh;
    list_for_each (lh, head)
        i++;
    return i;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    struct list_head *fast = head->next, *slow = head->next;
    for (; fast != head->prev && fast->next != head->prev;
         fast = fast->next->next, slow = slow->next)
        ;
    /*while(fast != NULL && fast->next != NULL){
        fast = fast->next->next;
    slow = slow->next;
    }*/
    slow->prev->next = slow->next;
    slow->next->prev = slow->prev;
    q_release_element(list_entry(slow, element_t, list));
    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head)
        return false;
    element_t *pos, *n;
    bool used = false;
    list_for_each_entry_safe (pos, n, head, list) {
        if (pos->list.next != head &&
            !strcmp(pos->value,
                    list_entry(pos->list.next, element_t, list)->value)) {
            list_del(&pos->list);
            q_release_element(pos);
            used = true;
        } else if (used) {
            list_del(&pos->list);
            q_release_element(pos);
            used = false;
        }
    }
    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    bool odd = q_size(head) & 0x01;
    for (int i = q_size(head) >> 1; i > 0; i--) {
        list_move_tail(head->next->next, head);
        list_move_tail(head->next, head);
    }
    if (odd)
        list_move_tail(head->next, head);
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head || head->next->next == head)
        return;
    struct list_head *pos = head->prev;
    for (int i = q_size(head); i > 1; i--)
        list_move_tail(pos->prev, head);
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */

/*struct list_head *partition(struct list_head *i, struct list_head *j)
{
    struct list_head *pivot = i, *pos = i->next;
    for(; pos != j;){
    if(strcmp(list_entry(pos, element_t, list)->value, list_entry(pivot,
element_t, list)->value) < 0){ struct list_head *tmp = pos; pos = pos->next;
        //list_move_tail(tmp, i);
        list_del(tmp);
        list_add(tmp, pivot->prev);
    }else
        pos = pos->next;
    }
    return pivot;
}

void qs(struct list_head *head, struct list_head *tail)
{
    if(head->next != tail){
    struct list_head *p = partition(head, tail);
    struct list_head *lh;
    qs(head, p->prev);
    qs(p->next, tail);
    }
}

struct list_head *merge(struct list_head *l1, struct list_head *l2)
{
    struct list_head *tmp = l1->prev;
    while(l1 && l2){
        if(strcmp(list_entry(l1, element_t, list)->value, list_entry(l2,
element_t, list)->value) > 0) tmp->next = l2; else tmp->next = l1; tmp =
tmp->next;
    }
    if(l1)
    tmp->next = l1;
    else
    tmp->next = l2;
    return tmp;
}*/

struct list_head *mergesort(struct list_head *node)
{
    if (!node || !node->next)
        return node;
    struct list_head *fast = node, *slow = node;
    for (; fast->next && fast->next->next;
         fast = fast->next->next, slow = slow->next)
        ;
    fast = slow->next;
    slow->next = NULL;

    struct list_head *l1 = mergesort(node);
    struct list_head *l2 = mergesort(fast);
    // printf("%s\n", list_entry(l1->next, element_t, list)->value);

    struct list_head t_head, *tmp = &t_head;
    while (l1 && l2) {
        if (strcmp(list_entry(l1, element_t, list)->value,
                   list_entry(l2, element_t, list)->value) > 0) {
            tmp->next = l2;
            l2 = l2->next;
        } else {
            tmp->next = l1;
            l1 = l1->next;
        }
        tmp = tmp->next;
    }
    if (l1)
        tmp->next = l1;
    else
        tmp->next = l2;
    return t_head.next;
}

void q_sort(struct list_head *head)
{
    if (!head || !head->next)
        return;
    head->prev->next = NULL;
    struct list_head *list = head->next;
    list = mergesort(list);
    head->next = list;

    struct list_head *i = head;
    while (i->next != NULL) {
        i->next->prev = i;
        i = i->next;
    }
    head->prev = i;
    i->next = head;
}

void q_shuffle(struct list_head *head)
{
    srand(time(NULL));
    for (int i = q_size(head); i > 0; i--) {
        struct list_head *tmp = head->next, *tail = head->prev;
        for (int x = rand() % i; x > 0; x--)
            tmp = tmp->next;
        list_del(tail);
        list_add(tail, tmp);
        list_move_tail(tmp, head);
    }
}
