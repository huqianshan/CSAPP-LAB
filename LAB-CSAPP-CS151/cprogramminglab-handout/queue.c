/* 
 * Code for basic C skills diagnostic.
 * Developed for courses 15-213/18-213/15-513 by R. E. Bryant, 2017
 * Modified to store strings, 2018
 */

/*
 * This program implements a queue supporting both FIFO and LIFO
 * operations.
 *
 * It uses a singly-linked list to represent the set of queue elements
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/*
  Create empty queue.
  Return NULL if could not allocate space.
*/
queue_t *q_new()
{
    queue_t *q =  malloc(sizeof(queue_t));
    /* What if malloc returned NULL? */
	if (q == NULL)
		return NULL;
	//q->tail = NULL;
	//q->size = NULL;
    q->head = NULL;
	q->tail = NULL;
	q->size = 0;
    return q;
}

/* Free all storage used by queue */
void q_free(queue_t *q)
{	
	list_ele_t *ele;
	if (q == NULL) {
		return ;
	}
	ele = q->head;
	while (ele != NULL) {
		list_ele_t* tem = ele;
		ele = ele->next;
		//free(tem->next);
		free(tem->value);
		free(tem);
	}
    /* How about freeing the list elements and the strings? */
    /* Free queue structure */
    free(q);
	return;
}

/*
  Attempt to insert element at head of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
  Argument s points to the string to be stored.
  The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(queue_t *q, char *s)
{
    list_ele_t *newh;
    /* What should you do if the q is NULL? */
	if (q == NULL) {
		return false;
	}
    newh = malloc(sizeof(list_ele_t));
	if (newh == NULL) {
		return false;
	}
	newh->value = malloc(strlen(s)+1);
	/* if malloc failed here*/
	if (newh->value == NULL) {
		free(newh);
		return false;
	}
	strcpy(newh->value, s);
	newh->next = q->head;

	q->size++;
	if (q->size == 1) {
		q->tail = newh;
	}
    /* Don't forget to allocate space for the string and copy it */
    /* What if either call to malloc returns NULL? */
    
    q->head = newh;
    return true;
}


/*
  Attempt to insert element at tail of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
  Argument s points to the string to be stored.
  The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(queue_t *q, char *s)
{
    /* You need to write the complete code for this function */
    /* Remember: It should operate in O(1) time */
	if (q == NULL) {
		return false;
	}
	list_ele_t* newT;
	newT = malloc(sizeof(list_ele_t));
	if (newT == NULL) {
		return false;
	}
	newT->value = malloc(strlen(s) + 1);
	if (newT->value == NULL) {
		free(newT);
		return false;
	}
	strcpy(newT->value, s);
	newT->next = NULL;

	q->size++;
	if (q->size == 1) {
		q->head = newT;
	}
	else {
		q->tail->next = newT;
	}
	q->tail = newT;
    return true;
}

/*
  Attempt to remove element from head of queue.
  Return true if successful.
  Return false if queue is NULL or empty.
  If sp is non-NULL and an element is removed, copy the removed string to *sp
  (up to a maximum of bufsize-1 characters, plus a null terminator.)
  The space used by the list element and the string should be freed.
*/
bool q_remove_head(queue_t *q, char *sp, size_t bufsize)
{
    /* You need to fix up this code. */
	if (q == NULL||q->size==0) {
		return false;
	}

	// from head
	list_ele_t* tem = q->head;
	if (sp != NULL) {
		strncpy(sp,tem->value,bufsize-1);
		if (strlen(tem->value) > bufsize) {
			sp[bufsize - 1] = '\0';
		}
	}

	q->size--;
	if (q->size == 0) {
		q->head = NULL;
		q->tail = NULL;
	}
	else {
		q->head = q->head->next;
	}
	free(tem->value);
	free(tem);

    return true;
}

/*
  Return number of elements in queue.
  Return 0 if q is NULL or empty
 */
int q_size(queue_t *q)
{
    /* You need to write the code for this function */
    /* Remember: It should operate in O(1) time */
	if (q == NULL) {
		return 0;
	}
    return q->size;
}

/*
  Reverse elements in queue
  No effect if q is NULL or empty
  This function should not allocate or free any list elements
  (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
  It should rearrange the existing ones.
 */
void q_reverse(queue_t *q)
{
    /* You need to write the code for this function */
	if (q == NULL||q->size==0) {
		return;
	}
	list_ele_t* bef = NULL;
	list_ele_t* tem = q->head;
	list_ele_t* next = NULL;
	while (tem->next != NULL) {
		next = tem->next;
		tem->next = bef;
		bef = tem;
		tem = next;
	}
	tem->next = bef;
	q->tail = q->head;
	q->head = tem;
	return;
}

int main() {}