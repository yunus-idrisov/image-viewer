#include <string.h>
#include <stdio.h>
#include "StrList.h"

void InitStrList(StrList** list){
	*list = malloc(sizeof(StrList));
	(*list)->head = 0;
	(*list)->tail = 0;
}

void AddStringToStrList(StrList* list, const char* str){
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

void DeleteStrList(StrList** list){
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

static void Merge(StrList* list, ListNode* l, ListNode* m, ListNode* h, int num){
	char* str[num];
	int cur = 0;
	m = m->next;
	ListNode* p1 = l;
	ListNode* p2 = m;
	while( p1 != m && p2 != h->next ){
		if( strcmp(p1->str, p2->str) <= 0 ){
			str[cur] = p1->str;
			cur++;
			p1 = p1->next;
		}
		else{
			str[cur] = p2->str;
			cur++;
			p2 = p2->next;
		}
	}
	while( p1 != m ){
		str[cur] = p1->str;
		cur++;
		p1 = p1->next;
	}
	while( p2 != h->next ){
		str[cur] = p2->str;
		cur++;
		p2 = p2->next;
	}
	for(int i = 0; i < num; i++){
		l->str = str[i];
		l = l->next;
	}
}

static void MergeSort(StrList* list, ListNode* l, ListNode* h, int num){
	int mid = num/2;
	ListNode* m = l;
	for(int i = 0; i < mid - 1; i++)
		m = m->next;
	if( num > 1 ){
		MergeSort(list, l, m, num/2);
		MergeSort(list, m->next, h, num - num/2);
		Merge(list, l, m, h, num);
	}
}

void SortStrList(StrList* list){
	if( list == 0 )
		return;
	// Находим количество элементов в списке.
	int elemCount = 0;
	ListNode* p = list->head;
	while( p != 0 ){
		elemCount++;
		p = p->next;
	}
	// Собственно сортировка.
	MergeSort( list, list->head, list->tail, elemCount );
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
