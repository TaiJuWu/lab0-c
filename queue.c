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

static int size = 0;

static int compare(char *s, char *t)
{
    return strcmp(s, t);
}

struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (!head) {
        return NULL;
    }
    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head) {
        return;
    }

    element_t *node, *next;
    list_for_each_entry_safe (node, next, head, list) {
        list_del(&node->list);
        free(node->value);
        free(node);
    }
    free(head);
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
    if (!head) {
        return false;
    }
    element_t *newh = malloc(sizeof(element_t));
    if (!newh) {
        return false;
    }
    memset(newh, 0, sizeof(element_t));

    int str_len = (strlen(s) + 1) * sizeof(char);
    newh->value = malloc(str_len);
    if (!newh->value) {
        free(newh);
        return false;
    }
    memset(newh->value, 0, str_len);
    strncpy(newh->value, s, str_len);

    list_add(&newh->list, head);
    size += 1;
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
    if (!head) {
        return false;
    }
    element_t *newt = malloc(sizeof(element_t));
    if (!newt) {
        return false;
    }
    memset(newt, 0, sizeof(element_t));

    int str_len = (strlen(s) + 1) * sizeof(char);
    newt->value = malloc(str_len);
    if (!newt->value) {
        free(newt);
        return false;
    }
    memset(newt->value, 0, str_len);
    strncpy(newt->value, s, str_len);

    list_add_tail(&newt->list, head);
    size += 1;

    return true;
}

element_t *q_remove(struct list_head *ele, char *sp, size_t bufsize)
{
    element_t *node = list_entry(ele, element_t, list);
    list_del_init(ele);
    if (sp && bufsize) {
        memset(sp, 0, bufsize);
        strncpy(sp, node->value, bufsize - 1);
    }

    size -= 1;
    return node;
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
    if (!head || list_empty(head)) {
        return NULL;
    }

    return q_remove(head->next, sp, bufsize);
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head)) {
        return NULL;
    }

    return q_remove(head->prev, sp, bufsize);
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
    return size;
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
    if (!head || list_empty(head)) {
        return false;
    }

    struct list_head *slow = head->next, *fast = head->next;
    while (fast != head && fast->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }
    list_del(slow);
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
    if (!head || list_empty(head)) {
        return false;
    }

    element_t *node = NULL, *next = NULL;
    char *prev_str = "";
    list_for_each_entry_safe (node, next, head, list) {
        if (!compare(prev_str, node->value)) {
            list_del(&node->list);
            q_release_element(node);
        } else {
            prev_str = node->value;
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
    if (!head || list_empty(head) || list_is_singular(head)) {
        return;
    }
    struct list_head *ptr = head->next;
    while (ptr != head && ptr->next != head) {
        element_t *tmp1 = list_entry(ptr, element_t, list);
        element_t *tmp2 = list_entry(ptr->next, element_t, list);

        char *tmp = tmp1->value;
        tmp1->value = tmp2->value;
        tmp2->value = tmp;

        ptr = ptr->next->next;
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
    if (!head || list_empty(head)) {
        return;
    }

    struct list_head *curr = head;
    do {
        struct list_head *tmp = curr->next;
        curr->next = curr->prev;
        curr->prev = tmp;

        curr = curr->prev;
    } while (curr != head);
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */

struct list_head *mergeTwoList(struct list_head *left, struct list_head *right)
{
    struct list_head dummy;
    struct list_head *prev = &dummy;
    memset(prev, 0, sizeof(struct list_head));

    while (left && right) {
        if (compare(list_entry(left, element_t, list)->value,
                    list_entry(right, element_t, list)->value) < 0) {
            prev->next = left;
            left = left->next;
        } else {
            prev->next = right;
            right = right->next;
        }
        prev = prev->next;
    }
    prev->next = left ? left : right;

    return dummy.next;
}


struct list_head *merge_sort(struct list_head *head)
{
    if (!head || !head->next) {
        return head;
    }

    struct list_head *slow = head, *fast = head->next;
    for (; fast && fast->next; fast = fast->next->next) {
        slow = slow->next;
    }
    fast = slow->next;
    slow->next = NULL;

    return mergeTwoList(merge_sort(head), merge_sort(fast));
}


void q_sort(struct list_head *head)
{
    if (!head || list_empty(head)) {
        return;
    }
    head->prev->next = NULL;
    head->next = merge_sort(head->next);

    struct list_head *prev = head, *curr = head->next;
    for (; curr; prev = curr, curr = curr->next) {
        curr->prev = prev;
    }

    head->prev = prev;
    prev->next = head;
}
