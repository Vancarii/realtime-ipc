// list.c
#include "list.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

// General Error Handling:
// Client code is assumed never to call these functions with a NULL List pointer, or 
// bad List pointer. If it does, any behaviour is permitted (such as crashing).
// HINT: Use assert(pList != NULL); just to add a nice check, but not required.


// pools for the nodes and the heads that are available
static Node nodes_pool[LIST_MAX_NUM_NODES];
static List lists_pool[LIST_MAX_NUM_HEADS];

// a stack / list of all the nodes that are available
Node *freeNodeHead = NULL;
List *freeListHead = NULL;

static int avail_nodes = LIST_MAX_NUM_NODES;
static int avail_lists = LIST_MAX_NUM_HEADS;

// Makes a new, empty list, and returns its reference on success. 
// Returns a NULL pointer on failure.
List* List_create() {

    // case for first time initialization
    if (freeListHead == NULL && avail_lists == LIST_MAX_NUM_HEADS) {

        for (int i = 0; i < LIST_MAX_NUM_HEADS; i++) {

            // initialize the freeListHead stack 
            lists_pool[i].next = freeListHead;
            freeListHead = &lists_pool[i];

            lists_pool[i].head = NULL;
            lists_pool[i].tail = NULL;
            lists_pool[i].current = NULL;
            lists_pool[i].count = 0;
        
        }

        // all nodes not initialized
        if (freeNodeHead == NULL && avail_nodes == LIST_MAX_NUM_NODES) {
            for (int i = 0; i < LIST_MAX_NUM_NODES; i++) {

                // initialize the freeNodeHead stack 
                nodes_pool[i].next = freeNodeHead;
                freeNodeHead = &nodes_pool[i];

                nodes_pool[i].prev = NULL;
                nodes_pool[i].data = NULL;
            
            }
        }


    }

    if (freeListHead != NULL){

        // pop off the freeListHead stack and return it
        List *list = freeListHead;
        freeListHead = freeListHead->next;
        avail_lists--;
        return list;
    }

    return NULL;

}


// Returns the number of items in pList.
int List_count(List* pList) {
    assert(pList != NULL);
    return pList->count;
}


// Returns a pointer to the first item in pList and makes the first item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_first(List* pList) {
    assert(pList != NULL);
    // check empty list
    if (pList->head == NULL) {
        pList->current = NULL;
        return NULL;
    }

    pList->current = pList->head;
    return pList->head->data;
}


// Returns a pointer to the last item in pList and makes the last item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_last(List* pList) {
    assert(pList != NULL);

    // check empty list
    if (pList->head == NULL) {
        pList->current = NULL;
        return NULL;
    }
    pList->current = pList->tail;
    return pList->tail->data;
}



// Advances pList's current item by one, and returns a pointer to the new current item.
// If this operation advances the current item beyond the end of the pList, a NULL pointer 
// is returned and the current item is set to be beyond end of pList.
void* List_next(List* pList) {
    assert(pList != NULL);

    // check out of bounds
    if (pList->current == NULL) {


        if (pList->oob == LIST_OOB_END) {
            pList->current = NULL;
            return NULL;
        } 
        else if (pList->oob == LIST_OOB_START) {
            if (pList->head == NULL) {
                pList->current = NULL;
                return NULL;
            }
            pList->current = pList->head;
            return pList->current->data;
        }

    }

    // case for out of bounds at end
    if (pList->current == pList->tail) {
        pList->oob = LIST_OOB_END;
        pList->current = NULL;
        return NULL;
    }

    // if no case is met, then we can just
    // move to next and return data
    pList->current = pList->current->next;
    return pList->current->data;
}


// Backs up pList's current item by one, and returns a pointer to the new current item. 
// If this operation backs up the current item beyond the start of the pList, a NULL pointer 
// is returned and the current item is set to be before the start of pList.
void* List_prev(List* pList) {
    assert(pList != NULL);

    // out of bounds check
    if (pList->current == NULL) {

        if (pList->oob == LIST_OOB_START) {
            pList->current = NULL;
            return NULL;
        } 
        else if (pList->oob == LIST_OOB_END) {
            if (pList->tail == NULL) {
                pList->current = NULL;
                return NULL;
            }
            pList->current = pList->tail;
            return pList->current->data;
        }

    }

    // case for out of bounds at start
    if (pList->current == pList->head) {
        pList->oob = LIST_OOB_START;
        pList->current = NULL;
        return NULL;
    }
    
    // case for prev node is null
    pList->current = pList->current->prev;
    return pList->current->data;
}


// Returns a pointer to the current item in pList.
void* List_curr(List* pList) {
    assert(pList != NULL);
    if (pList->current == NULL) {
        return NULL;
    }
    return pList->current->data;
}


// Adds the new item to pList directly after the current item, and makes item the current item. 
// If the current pointer is before the start of the pList, the item is added at the start. If 
// the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_insert_after(List* pList, void* pItem) {
    assert(pList != NULL);

     if (freeNodeHead == NULL || avail_nodes == 0) {
        // No available node
        return LIST_FAIL; 
     }

    // pop off the freeNodeHead stack to be used
    Node *newNode = freeNodeHead;
    freeNodeHead = freeNodeHead->next;
    newNode->data = pItem;

    // case for empty list
    if (pList->head == NULL) {
        pList->head = newNode;
        pList->tail = newNode;
        pList->head->prev = NULL;
        pList->tail->next = NULL;
        pList->current = newNode;
    } 
    // case for out of bounds at the start
    else if (pList->oob == LIST_OOB_START && pList->current == NULL) {
        newNode->next = pList->head;
        pList->head->prev = newNode;
        pList->head = newNode;
        pList->current = newNode;
    } 
    // case for out of bounds at the end
    else if (pList->current == pList->tail || (pList->oob == LIST_OOB_END && pList->current == NULL)) {
        newNode->prev = pList->tail;
        pList->tail->next = newNode;
        pList->tail = newNode;
        pList->current = newNode;
        pList->current->next = NULL;
    } else {
        newNode->next = pList->current->next;
        newNode->prev = pList->current;
        pList->current->next->prev = newNode;
        pList->current->next = newNode;
        pList->current = newNode;
    }

    pList->count++;
    avail_nodes--;

    return LIST_SUCCESS;

}



// Adds item to pList directly before the current item, and makes the new item the current one. 
// If the current pointer is before the start of the pList, the item is added at the start. 
// If the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_insert_before(List* pList, void* pItem){
   assert(pList != NULL);


     if (freeNodeHead == NULL && avail_nodes == 0) {
        // No available node
        return LIST_FAIL; 
     }

    Node *newNode = freeNodeHead;
    freeNodeHead = freeNodeHead->next;
    newNode->data = pItem;

    if (pList->head == NULL) {
        pList->head = newNode;
        pList->tail = newNode;
        pList->current = newNode;
    } else if (pList->current == pList->head || (pList->oob == LIST_OOB_START && pList->current == NULL)) {
        newNode->next = pList->head;
        pList->head->prev = newNode;
        pList->head = newNode;
        pList->current = newNode;
    } else if (pList->oob == LIST_OOB_END && pList->current == NULL) {
        newNode->prev = pList->tail;
        pList->tail->next = newNode;
        pList->tail = newNode;
        pList->current = newNode;
    } else {
        newNode->next = pList->current;
        newNode->prev = pList->current->prev;
        pList->current->prev->next = newNode;
        pList->current->prev = newNode;
        pList->current = newNode;
    }

    pList->count++;
    avail_nodes--;

    return LIST_SUCCESS;
}

// Adds item to the end of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_append(List* pList, void* pItem){

    // moves to the end, adds the new item at the end. 
    // dont increase count, function call already does it
    if (pList->tail == NULL) {
        pList->current = pList->head;
    } else {
        pList->current = pList->tail;
    }
    return List_insert_after(pList, pItem);
}

// Adds item to the front of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_prepend(List* pList, void* pItem){
    // moves to the start, adds the new item at the start. 
    // dont increase count, function call already does it
    pList->current = pList->head;
    return List_insert_before(pList, pItem);
}

// Return current item and take it out of pList. Make the next item the current one.
// If the current pointer is before the start of the pList, or beyond the end of the pList,
// then do not change the pList and return NULL.
void* List_remove(List* pList){
    assert(pList != NULL);

    if (pList->count == 0) {
        pList->current = NULL;
        pList->oob = LIST_OOB_END;
        return NULL;
    }
    
    Node* temp = pList->current;
    void* tempdata = temp->data;

    Node* next = temp->next;

    if (temp == pList->head) {
        // pop back on stack
        temp->next = freeNodeHead;
        freeNodeHead = temp;

        if (pList->count == 1){
            pList->head = NULL;
            pList->tail = NULL;
            pList->current = NULL;
        } else {
            pList->head = next;
            pList->current = next;
        }

    }

    else if (temp == pList->tail) {
        // pop back on stack
        temp->next = freeNodeHead;
        freeNodeHead = temp;

        pList->tail = pList->tail->prev;
        pList->current = NULL;
        pList->tail->next = NULL;
        pList->oob = LIST_OOB_END;

    }

    else {

        pList->current->prev->next = pList->current->next;
        pList->current->next->prev = pList->current->prev;
        pList->current = pList->current->next;

        // pop back on stack
        temp->next = freeNodeHead;
        freeNodeHead = temp;

    }

    pList->count--;
    avail_nodes++;

    return tempdata;
}

// Return last item and take it out of pList. Make the new last item the current one.
// Return NULL if pList is initially empty.
void* List_trim(List* pList){
    assert(pList != NULL);
    if (pList->tail == NULL) {
        return NULL;
    }

    pList->current = pList->tail;
    void* data = pList->tail->data;
    List_remove(pList);
    pList->current = pList->tail;
    return data;

}

// Adds pList2 to the end of pList1. The current pointer is set to the current pointer of pList1. 
// pList2 no longer exists after the operation; its head is available
// for future operations.
void List_concat(List* pList1, List* pList2){
    assert(pList1 != NULL);
    assert(pList2 != NULL);


    // case for empty list1
    if (pList1->head == NULL) {
        pList1->head = pList2->head;
        pList1->tail = pList2->tail;


        pList1->oob = LIST_OOB_START;
        pList1->current = NULL;
    } 
    // case for non empty list1
    else {
        // case for non empty list2
        if (pList2->head != NULL) {
            pList1->tail->next = pList2->head;
            pList2->head->prev = pList1->tail;
            pList1->tail = pList2->tail;
        } else {
            // empty list2
            pList1->tail->next = NULL;
        }
    }
    pList1->count += pList2->count;

    // push list back onto available stack
    pList2->next = freeListHead;
    freeListHead = pList2;
    pList2->head = NULL;
    pList2->tail = NULL;
    pList2->current = NULL;
    pList2->count = 0;
    avail_lists++;

}

// Delete pList. pItemFreeFn is a pointer to a routine that frees an item. 
// It should be invoked (within List_free) as: (*pItemFreeFn)(itemToBeFreedFromNode);
// pList and all its nodes no longer exists after the operation; its head and nodes are 
// available for future operations.
typedef void (*FREE_FN)(void* pItem);
void List_free(List* pList, FREE_FN pItemFreeFn){
    assert(pList != NULL);

    Node *temp = pList->head;
    Node *next;

    while (pList->count > 0) {
        next = temp->next;
        if (pItemFreeFn != NULL) {
            pItemFreeFn(temp->data);
        }

        // Reset the node
        temp->prev = NULL;
        temp->data = NULL;

        // pop node back on stack
        temp->next = freeNodeHead;
        freeNodeHead = temp;

        avail_nodes++;

        // iterate while loop
        temp = next;
        pList->count--;
    }

    pList->head = NULL;
    pList->tail = NULL;
    pList->current = NULL;
    pList->count = 0;

    // push list back onto stack
    pList->next = freeListHead;
    freeListHead = pList;
    avail_lists++;

}

// Search pList, starting at the current item, until the end is reached or a match is found. 
// In this context, a match is determined by the comparator parameter. This parameter is a
// pointer to a routine that takes as its first argument an item pointer, and as its second 
// argument pComparisonArg. Comparator returns 0 if the item and comparisonArg don't match, 
// or 1 if they do. Exactly what constitutes a match is up to the implementor of comparator. 
// 
// If a match is found, the current pointer is left at the matched item and the pointer to 
// that item is returned. If no match is found, the current pointer is left beyond the end of 
// the list and a NULL pointer is returned.
// 
// If the current pointer is before the start of the pList, then start searching from
// the first node in the list (if any).
typedef bool (*COMPARATOR_FN)(void* pItem, void* pComparisonArg);
void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg){
    assert(pList != NULL);

    if (pList->head == NULL && pList->count == 0) {
        pList->current = NULL;
        pList->oob = LIST_OOB_END;
        return NULL;
    }

    // out of bounds check 
    // only set current to head when out of bounds, otherwise start search
    // from current
    if (pList->oob == LIST_OOB_START && pList->current == NULL) {
        pList->current = pList->head;
    }

    while (pList->current != NULL) {
        if (pComparator(pList->current->data, pComparisonArg)) {
            return pList->current->data;
        }
        pList->current = pList->current->next;
    }

    pList->current = NULL;
    pList->oob = LIST_OOB_END;

    return NULL;
}
