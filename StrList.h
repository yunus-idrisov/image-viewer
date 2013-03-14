#ifndef STRLIST_H
#define STRLIST_H

/*
  Двунаправленный список из строк в стиле C.
  Список объязательно должен быть инициализирован
  функцией InitStrList(...) и удалён с помощью
  DeleteStrList(...).
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
 при работе со списком. Для аргумента list не должна
 быть выделена память.
*/
void InitStrList(StrList** list);

// Вставка строки в список(c выделением для неё памяти).
void AddStringToStrList(StrList* list, const char* str);

/*
 Очистка списка. ОБРАТИТЕ ВНИМАНИЕ, что в функцию
 необходимо передать адрес указателя на список. 
 Ошибки компиляции не будет, если вы передадите просто 
 указатель, а ошибка выполнения будет.
*/
void DeleteStrList(StrList** list);

// Сортировка списка.
void SortStrList(StrList* list);

// Test funs.
void ShowStrList(StrList* list);
void ShowRevStrList(StrList* list);

#endif // STRLIST_H
