    #include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

typedef struct node
{
    int data;
    struct node *next;
    struct node *prev;
}Node,*DLnode;

DLnode head;

typedef struct list 
{
    int size;
}List;

List *initLinkList() //链表初始化
{
    List *list = (List *)malloc(sizeof(List));//向堆区申请内存
    if (list == NULL)
        return NULL;
    list->size = 0;
    return list;
}

DLnode getnode(int data)
{
    DLnode node = (DLnode)malloc(sizeof(DLnode));
    node->data=data;
    node->next=NULL;
    node->prev=NULL;
    return node;
}

void headnode(int data,List *list)
{
    DLnode t=getnode(data);
    if(head==NULL){
        head=t;
        list->size++;
        return;
    }
    head->prev=t;
    t->next=head;
    head=t;
    list->size++;
}

void print(){
    DLnode p=head;
    printf("正向打印：");
  while(p!=NULL){
        printf("%d",p->data);
        p=p->next;
    }
    printf("\n");
}

void insert_node(int n,int m,List *list){
    DLnode ptr = getnode(n);
    DLnode p;
    p=head;
    int i=1;
    while(p!=NULL&&i!=m){
         p=p->next;
         ++i;
    }
    if(p==NULL){
         printf("插入元素：指定位置超出链表.\n");
         return;
    }
        ptr->next=p->next;
        ptr->prev=p;
        p->next=ptr; 
        list->size++;
printf("插入元素：在第%d个位置后面加入元素:%d.\n",m,n);              
}

int deleteElement(int n,List *list){
    DLnode p,q,r;
    p = head;
    int i=1;
    int o;
    while(p->next!=NULL&&i!=n){
        i++;
        p=p->next;  
    }
//判断是否是最后元素
    if(p->next==NULL&&i==n){
            o=p->data;
            q=p->prev;
            r=p->next;
            q->next=r;
            printf("删除指定第%d个元素：%d\n",i,o);
            list->size--;
    free(p); 
    return 0; 
    } 
//判断是否是开始元素
    if(p->next!=NULL&&i==1){
       o=p->data;  
       q=p->next;
       r=q->next;
       head=p->next;
       free(p);
      printf("删除指定第%d个元素：%d\n",i,o);
      list->size--;
      return 0; 
    }
//判断是否超出链表长度 
    if(p->next==NULL){
        printf("指定位置无元素.\n");
        return 0;
    }
//判断是否是指定值
    if(i==n){
    o=p->data;
    q=p->prev;
    r=p->next;
    q->next=r;
    printf("删除指定第%d个元素：%d\n",i,o);
    list->size--;
    }
        if(r!=NULL){
        r->prev = q;
    }
    free(p); 
    return 0;
}

int clear(List *list)
{
    while(list->size>0){
        deleteElement(1,list);
    }
    free(list);
    return list->size;
}

int main()
{
    List *list = initLinkList();
    headnode(4,list);
    headnode(4,list);
    headnode(6,list);
    headnode(4,list);
    headnode(4,list);
    headnode(3,list);
    headnode(4,list);
    headnode(4,list);
    print();
    insert_node(5,4,list);
    print();
    deleteElement(5,list);
    print();
    clear(list);
return 0 ;
}