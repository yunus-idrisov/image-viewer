#ifndef STRLIST_H
#define STRLIST_H

/*
  Двунаправленный список из строки в стиле C.
  Список объязательно должен быть инициализирован
  функцией initStrList(...) и удалён с помощью
  deleteStrList(...).
*/

typedef struct ListNode{
	char* str;
	struct ListNode* next;
	struct ListNode* prev;
} ListNode;

typedef struct{
	ListNode* head;
	ListNode* tail;
} StrList;

/*
 Инициализация списка. Должна быть вызвана первой
 при работе со списком. Для list не должна
 быть выделена память.
*/
void initStrList(StrList** list);

// Вставка строки в список(c выделением для неё памяти).
void addStringToStrList(StrList* list, const char* str);

/*
 Очистка списка. ОБРАТИТЕ ВНИМАНИЕ, что в функцию
 необходимо передать адрес указателя на список. 
 Ошибки компиляции не будет, если вы передадите просто 
 указатель, а ошибка выполнения будет.
*/
void deleteStrList(StrList** list);

// Test funs.
void ShowStrList(StrList* list);
void ShowRevStrList(StrList* list);

#endif // STRLIST_H
