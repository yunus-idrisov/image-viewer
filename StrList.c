#include <string.h>
#include <stdio.h>
#include "StrList.h"

void initStrList(StrList** list){
	*list = malloc(sizeof(StrList));
	(*list)->head = 0;
	(*list)->tail = 0;
}

void addStringToStrList(StrList* list, const char* str){
	// Пустая строка.
	if( strlen(str) == 0 )
		return;

	// Создаём новый элемент списка.
	ListNode* tmp = malloc(sizeof(ListNode));
	int len = strlen(str);
	tmp->str = malloc(sizeof(char)*(len + 1));
	strcpy(tmp->str, str);
	tmp->next = 0;

	// Если список пустой.
	if( (list->head == 0) && (list->tail == 0) ){
		tmp->prev = 0;
		list->head = tmp;
		list->tail = tmp;
	} // Если в списке уже имеются элементы.
	else {
		tmp->prev = list->tail;
		list->tail->next = tmp;
		list->tail = tmp;
	}
}

void deleteStrList(StrList** list){
	if( (*list) == 0 )
		return;
	ListNode* cur = (*list)->head;
	while( cur != 0 ){
		ListNode* next = cur->next;
		free(cur->str);
		free(cur);
		cur = next;
	}
	free(*list);
	*list = 0;
}

// Test.

void ShowStrList(StrList* list){
	if( list == 0 )
		return;
	ListNode* p = list->head;
	while( p != 0 ){
		printf("%s\n", p->str);
		p = p->next;
	}
}

void ShowRevStrList(StrList* list){
	if( list == 0 )
		return;
	ListNode* p = list->tail;
	while( p != 0 ){
		printf("%s\n", p->str);
		p = p->prev;
	}
}
