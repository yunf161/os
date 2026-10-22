#ifndef LIST_H
#define LIST_H

struct ListItem{
    ListItem *prev;
    ListItem *next;
    int priority;
};

class List{ 

    ListItem head;

public:
    List();
    void initialize();

    int size();

    bool isEmpty();

    ListItem* back();

    void push_back(ListItem *item);

    void pop_back();

    ListItem* front();

    void push_front(ListItem *item);

    void pop_front();

    ListItem *at(int pos);

    void insert(ListItem *item, int position);

    void erase(int pos);

    void erase(ListItem *itemPtr);
    void push_priority(ListItem *item);
    
    
    int find(ListItem *itemPtr);
    void inc();

};





#endif