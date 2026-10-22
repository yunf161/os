#include"list.h"

List::List(){ 
    head.prev = nullptr;
    head.next = nullptr;
}
void List::initialize(){
    head.prev = nullptr;
    head.next = nullptr;
}
  int List:: size(){
    int count=0;
    ListItem *temp=&head;
    while(temp->next!=nullptr){
        count++;
        temp=temp->next;
    }
    return count;
  }


bool List::isEmpty(){
    return this->size()==0;
}

ListItem* List::back(){
    ListItem *temp=&head;
    if(temp->next==nullptr){
        return nullptr;
    }
    while(temp->next!=nullptr){
        temp=temp->next;
    }
    return temp;
}

void List::push_back(ListItem *item){
    if(head.next==nullptr){
        head.next=item;
        item->prev=&head;
        item->next=nullptr;
    }
    else{
        ListItem *temp=this->back();
       
        temp->next=item;
        item->prev=temp;
        item->next=nullptr;
    }
}

void List::pop_back(){

    ListItem *temp=this->back();
    if(temp){
    ListItem *temp1=temp->prev;
    temp->prev=nullptr;
    temp1->next=nullptr;}
}

ListItem* List::front(){
    return head.next;
}

void List::push_front(ListItem *item){
    ListItem *temp=this->front();
    head.next=item;
    item->prev=&head;
    item->next=temp;
    if(temp){
        temp->prev=item;
    }
}

 void List::pop_front(){
    ListItem *temp=this->front();
    if(temp){
        head.next=temp->next;
        if(temp->next){
            temp->next->prev=&head;
        }
        temp->next=nullptr;
        temp->prev=nullptr;
    }
 }

  ListItem * List::at(int pos){
    if (pos>=this->size()){
        return nullptr;
    }
    else{
        ListItem *temp=head.next;
        while(pos--){
            temp=temp->next;
        }
        return temp;
    }
  }

    void List::insert(ListItem *item, int position){
        int length=this->size();
        if(position>length){
            return;
        }
        else{
            ListItem *temp=this->at(position);
            ListItem *temp1= temp->prev;
            temp1->next=item;
            temp->prev=item;
            item->prev=temp1;
            item->next=temp;
        }
    }

    void List::erase(int pos){
       ListItem *temp=head.next;
       while(pos>=0&&temp){
        temp=temp->next;
        pos--;
       }
       if(pos>=0){
        return;
       }
            ListItem *front;
            ListItem *next;
       if(temp){
        front=temp->prev;
        next=temp->next;
         temp->prev=nullptr;
        temp->next=nullptr;
        if(front){
            front->next=next;
        }
        if(next){
            next->prev=front;
        }
       
       }
    }

    void List::erase(ListItem *itemPtr){
        ListItem *temp = head.next;
        while (temp != nullptr) {
            if (temp == itemPtr) {
                ListItem *prevItem = temp->prev;
                ListItem *nextItem = temp->next;

                if (prevItem != nullptr) {
                    prevItem->next = nextItem;
                }
                if (nextItem != nullptr) {
                    nextItem->prev = prevItem;
                }

                temp->prev = nullptr;
                temp->next = nullptr;
                return;
            }
            temp = temp->next;
        }
    }
    
   
    
    int List::find(ListItem *itemPtr){
        ListItem *temp = head.next;
        int index = 0;
        while (temp != nullptr) {
            if (temp == itemPtr) {
                return index;
            }
            temp = temp->next;
            index++;
        }
        return -1; // Not found
    }

    void List::push_priority(ListItem *item){
        if(head.next==nullptr){
        head.next=item;
        item->prev=&head;
        item->next=nullptr;
         }
          else{
        ListItem *temp=head.next;
        while(temp->next!=nullptr&&temp->priority>=item->priority){
            temp=temp->next;
        }
        if(temp->next==nullptr&&temp->priority>=item->priority){
            temp->next=item;
            item->prev=temp;
            item->next=nullptr;
        }
        else{
        ListItem *temp1=temp->prev;
        temp1->next=item;
        item->prev=temp1;
        item->next=temp;
        temp->prev=item;
    }
    }
}