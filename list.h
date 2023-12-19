#define nodesin(L) (node *t=0, *c=(L); c && t!=(L)->prev; t=c, c=c->next)

typedef struct node node;
struct node {
        node * next;
        node * prev;
        void * data;
};

typedef struct {
        char   name[10];
        node * wlist;
} ws;

typedef struct {
        int    wid;
        node * ws;
} win;

extern node * ws_list;
extern node * win_list;

void   free_ws(void *d);
void   free_win(void *d);

void   mvnode(node **position, node *target);
node * mknode(node **position, void *data);
void * rmnode(node **list, node *del);
void   rmlist(node **list, void (*rmdata)(void *));
// vim:cc=81 ts=8 sts=8 sw=8:
