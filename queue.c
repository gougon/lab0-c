#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

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
    struct list_head *head = malloc(sizeof(*head));
    if (head)
        INIT_LIST_HEAD(head);

    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;

    element_t *node = NULL, *safe = NULL;
    list_for_each_entry_safe (node, safe, l, list)
        q_release_element(node);
    free(l);
}

element_t *new_element(char *s)
{
    element_t *ele = malloc(sizeof(element_t));
    if (!ele)
        return NULL;

    int len = strlen(s) + 1;
    ele->value = malloc(len);
    if (!ele->value) {
        free(ele);
        return NULL;
    }

    strncpy(ele->value, s, len);
    return ele;
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

    element_t *ele = new_element(s);
    if (!ele)
        return false;

    list_add(&ele->list, head);
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

    element_t *ele = new_element(s);
    if (!ele)
        return false;

    list_add_tail(&ele->list, head);
    return true;
}

element_t *remove_element(struct list_head *pos, char *sp, size_t bufsize)
{
    element_t *target = container_of(pos, element_t, list);
    list_del(pos);

    if (sp) {
        int len = strlen(sp) > bufsize - 1 ? bufsize - 1 : strlen(sp);
        strncpy(sp, target->value, len);
        sp[len - 1] = '\0';
    }
    return target;
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
    return q_size(head) ? remove_element(head->next, sp, bufsize) : NULL;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    return q_size(head) ? remove_element(head->prev, sp, bufsize) : NULL;
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
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
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
    if (!q_size(head))
        return false;

    struct list_head *fwd = head->next, *bwd = head->prev;
    while (fwd != bwd && fwd->next != bwd) {
        fwd = fwd->next;
        bwd = bwd->prev;
    }
    q_release_element(remove_element(bwd, NULL, 0));
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

    if (q_size(head) < 2)
        return true;

    element_t *prev = NULL, *cur = NULL;
    list_for_each_entry_safe (prev, cur, head, list) {
        char *pval = prev->value;
        char *cval = cur->value;
        int plen = strlen(pval), clen = strlen(cval);
        if (plen == clen && !strncmp(pval, cval, (size_t) plen))
            q_release_element(remove_element(&prev->list, NULL, 0));
    }
    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (q_size(head) < 2)
        return;

    for (struct list_head *front = head->next, *back = head->next->next;
         front != head && back != head;
         front = head->next, back = head->next->next) {
        list_del(back);
        list_add_tail(back, head);
        list_del(front);
        list_add_tail(front, head);
    }
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
    struct list_head *tail = head->prev;
    for (struct list_head *cur = tail->prev; cur != head; cur = tail->prev)
        list_move_tail(cur, head);
}

struct list_head *merge_two_lists(struct list_head *l, struct list_head *r)
{
    struct list_head *head = NULL, **ptr = &head, *pprev = NULL, **node = NULL;
    for (; l && r; *node = (*node)->next) {
        char *lval = container_of(l, element_t, list)->value;
        char *rval = container_of(r, element_t, list)->value;
        int llen = strlen(lval), rlen = strlen(rval);
        int n = llen > rlen ? llen : rlen;
        node = (strncmp(lval, rval, n) > 0) ? &r : &l;
        *ptr = *node;
        (*ptr)->prev = pprev;
        ptr = &(*ptr)->next;
        pprev = *node;
    }
    *ptr = l ? l : r;
    (*ptr)->prev = pprev;
    return head;
}

struct list_head *mergesort_list(struct list_head *node)
{
    if (!node || !node->next)
        return node;

    struct list_head *slow = node;
    for (struct list_head *fast = node->next; fast && fast->next;
         fast = fast->next->next)
        slow = slow->next;
    struct list_head *mid = slow->next;
    slow->next = NULL;
    mid->prev = NULL;

    struct list_head *left = mergesort_list(node);
    struct list_head *right = mergesort_list(mid);
    return merge_two_lists(left, right);
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head)
{
    if (!head || !head->next)
        return;

    // Delete circular & Take out head
    struct list_head *first = head->next, *last = head->prev;
    last->next = NULL;
    head->prev = NULL;
    first->prev = NULL;
    head->next = NULL;

    first = mergesort_list(first);
    last = first;
    while (last->next)
        last = last->next;
    head->next = first;
    head->prev = last;
    first->prev = head;
    last->next = head;
}
