#include "list.h"
#include "debug.h"


private inline bool_t is_head(list_elem * elem)
{
    return elem != nullptr 
        && elem->prev == nullptr 
        && elem->next != nullptr; 
}
private inline bool_t is_tail(list_elem * elem)
{
    return elem != nullptr 
        && elem->prev != nullptr 
        && elem->next == nullptr; 
}
private inline bool_t is_interior(list_elem * elem)
{
    return elem != nullptr 
        && elem->prev != nullptr 
        && elem->next != nullptr; 
}
public void list_init(list * list)
{
    assert(list != nullptr);
    list->head.next = &list->tail;
    list->head.prev = nullptr;
    list->tail.next = nullptr;
    list->tail.prev = &list->head;
}

public list_elem * list_begin(list * list)
{
    assert(list != nullptr);
    return list->head.next;
}

public list_elem * list_end(list * list)
{
    assert(list != nullptr);
    return &list->tail;
}

public list_elem * list_next(list_elem * elem)
{
    assert(is_head(elem) || is_interior(elem));
    return elem->next;
}

public list_elem * list_rbegin(list * list)
{
    assert(list != nullptr);
    return list->tail.prev;
}

public list_elem * list_rend(list * list)
{
    assert(list != nullptr);
    return &list->head;
}

public list_elem * list_prev(list_elem * elem)
{
    assert(is_interior(elem) || is_tail(elem));
    return elem->prev;
}

public list_elem * list_head(list * list)
{
    assert(list != nullptr);
    return &list->head;
}

public list_elem * list_tail(list * list)
{
    assert(list != nullptr);
    return &list->tail;
}

public list_elem * list_insert(list_elem * before, list_elem * elem)
{
    // 将elem插在before的前面
    assert(is_interior(before) || is_tail(before));
    assert(elem != nullptr);

    elem->prev = before->prev;
    elem->next = before;
    before->prev->next = elem;
    before->prev = elem;
    return elem;
}

public list_elem * list_splice (list_elem *before,
            list_elem *first, list_elem *last)
{
    assert(is_interior(before) || is_tail(before));

    if(first == last) return first;
    last = list_prev(last);

    assert(is_interior(first));
    assert(is_interior(last));

    first->prev->next = last->next;
    last->next->prev = first->prev;

    first->prev = before->prev;
    last->next = before;
    before->prev->next = first;
    before->prev = last;
    return first;
}

public list_elem * list_push_front(list * list, list_elem * elem)
{
    assert(list != nullptr);
    assert(elem != nullptr);

    elem->next = list->head.next;
    list->head.next->prev = elem;
    list->head.next = elem;
    elem->prev = &list->head;
    return elem;
}

public list_elem * list_push_back(list * list, list_elem * elem)
{
    assert(list != nullptr);
    assert(elem != nullptr);

    elem->next = &list->tail;
    elem->prev = list->tail.prev;
    list->tail.prev->next = elem;
    list->tail.prev = elem;
    return elem;
}

public list_elem * list_remove(list_elem * elem)
{
    assert(is_interior(elem));
    elem->prev->next = elem->next;
    elem->next->prev = elem->prev;
    return elem->next;
}

public list_elem * list_pop_back(list * list)
{
    assert(list != nullptr);

    list_elem * back = list->tail.prev;
    back->prev->next = &list->tail;
    list->tail.prev = back->prev;
    return back;
}

public list_elem * list_pop_front(list * list)
{
    assert(list != nullptr);

    list_elem * front = list->head.next;
    front->next->prev = &list->head;
    list->head.next = front->next;
    return front;
}

public list_elem * list_front(list * list)
{
    assert(!list_empty(list));
    return list->head.next;
}

public list_elem * list_back(list * list)
{
    assert(!list_empty(list));
    return list->tail.prev;
}

public size_t list_size(list * list)
{
    assert(list != nullptr);

    size_t size = 0;
    for(list_elem * e = list_begin(list); e != list_end(list);e = list_next(e)) size++;
    return size;
}

public bool_t list_empty(list * list)
{
    assert(list != nullptr);

    return list->head.next == &list->tail 
        && list->tail.prev == &list->head;
}

