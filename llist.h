#ifndef _LLIST_H_
#define _LLIST_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
	int i;
	int j;
	void* k;
}ELE;

struct lnode{
	struct lnode *prev, *next;
	ELE* element;	
};

//包含整个链表元素个数和头结点
typedef struct{
	struct lnode head;
	int num;
}LLIST;

//函数指针，为了查找元素，key是要查找元素的关键字
typedef int(*list_find_p)(ELE* pELE, void *key);
typedef void(*list_travel_p)(ELE* pELE, void *key);

LLIST* lhandle;
list_travel_p travel_p;
list_find_p find_p;


LLIST* llist_creat(void);

//后插，插在链表的最后
int llist_append(LLIST* handle, ELE* pnode);

//前插,插在头结点的后面的第一个结点
int llist_preappend(LLIST* handle, ELE* pnode);
int llist_size(LLIST* handle);
void llist_destory(LLIST* handle);
int llist_delete(LLIST* handle, list_find_p find, void* key);
void llist_travel(LLIST* handle, list_travel_p trav, void* arg);


#ifdef __cplusplus
}
#endif

#endif //llist.h
