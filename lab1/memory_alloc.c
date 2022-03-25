#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/**
空闲块
    大小
    是否空闲 0为空闲，1为使用中
    首地址
    指向前、后空闲块的指针
*/
struct map
{
    unsigned m_size;
    int isUsed;    
    char *m_addr;
    struct map *next, *prior;
};

struct map *pointer;                //指向当前空闲区的指针
struct map *initmap;                //初始分配的区域

void initialize();
void print_freemap();
void print_help();
void handle_input();
char* lmalloc(unsigned size);
bool lfree(unsigned size, char *addr);

//程序初始化
void initialize() {
	// static struct map* coremap;
	initmap = malloc(sizeof(struct map));
	initmap->m_size = 1000;
    initmap->isUsed = 0;
	initmap->m_addr = (char*) malloc(sizeof(char) * 1000);
	initmap->prior = initmap;
	initmap->next = initmap;
	pointer = initmap;
	printf("Environment Initialized Successly\n");
	printf("malloc 1000: %p\n", initmap->m_addr);
}

//打印当前空闲存储区
void print_freemap() {
	struct map *ptr = pointer;
	printf ("---------------------------------------------\n");
	int cnt = 1;
    printf("Current Free Memory: num address size\n");
	do {
        if(ptr->isUsed == 0){
		    printf("Current Free Memory: %d\t%u\t%d\n", cnt, ptr->m_addr, ptr->m_size);
            ++cnt;
        }
		ptr = ptr->next;
	} while (ptr != pointer);
    printf ("---------------------------------------------\n");
};

//程序说明
void print_help(){
    printf("--------Help-----------\n");
    printf("alloc memory:  \'m [size] \' \n");
    printf("free memory:  \'f [size] [address]\' \n");
    printf("quit:  \'q\' \n");
    printf("---------end-----------\n");
}

//接收输入
void handle_input(){
    char choice;
    unsigned addr,size;
    do{
        scanf("%c",&choice);
        switch (choice){
        case 'm':
            printf("alloc\n");
            scanf("%u",&size);
            printf("size %u \n",size);
            lmalloc(size);
            print_freemap();
            break;
        case 'f':
            printf("free\n");
            scanf("%u %u", &size, &addr);
            printf("size:%u, addr:%u\n",size,addr);
            bool isFree = lfree(size, (char*) addr);
            if(isFree){
                print_freemap();
            }
            break;
        case 'q':
            printf("Bye~\n");
            exit(0);
            break;
        default:
            if(choice != '\n' && choice != '\t' && choice != ' ')
                print_help();
            break;
        }
    }while(true);
}

char* lmalloc(unsigned size) {
	bool flag = false;
    struct map *ptr = pointer;
    struct map *new_map = malloc(sizeof(struct map));
	do {
		if (pointer->m_size >= size) {
			new_map->m_size = size;
			new_map->m_addr = ptr->m_addr;
			new_map->isUsed = 1;
            new_map->prior = ptr->prior;
            new_map->next = ptr;

			pointer->m_size -= size;
			pointer->m_addr = (char*) ((unsigned) pointer->m_addr + size);
            ptr->prior->next = new_map;
            pointer->prior = new_map;
            flag = true;
			break;
		}
		else pointer = pointer->next;
	} while (pointer != ptr);
    if(flag)    
        return new_map->m_addr;
    else{
        printf("err: Failed to alloc memory--space is not enough.");
        free(new_map);
        return NULL; 
    }
}

bool lfree(unsigned size, char *addr){
    //检查size是否为一个区块的地址
    struct map *ptr = initmap;
	bool isFind = false;
	do {
        if(ptr->m_addr == addr && ptr->isUsed == 1){
            isFind = true;
            break;
        }
		ptr = ptr->next;
	} while (ptr != initmap);
    //如果free地址不是map地址，则报错
    if(!isFind){
        printf("err : Try to free invalid address!\n");
        return false;
    }
    //如果free大小大于目标map的大小，报错
    if(size > ptr->m_size){
        printf("err: Invalid size, too big!\n");
        return false;
    }
    //free大小与目标map大小相等
    if(size == ptr->m_size){
        struct map *front = ptr->prior;
        struct map *back = ptr->next;
        //如果后为占用区或ptr为队尾
        if(back->isUsed == 1 || back->m_addr < ptr->m_addr){
            if(front->isUsed == 1){
                ptr->isUsed = 0;
            }
            else{
                front->m_size += size;
                front->next = back;            
                back->prior = front;
                free(ptr);
            }
        }
        //后为空闲区且ptr不在队尾
        else{
            //前为占用区或ptr在队首
            if(front->isUsed == 1 || front->m_addr > ptr->m_addr){
                    ptr->m_size += back->m_size;
                    ptr->isUsed = 0;
                    ptr->next = back->next;
                    back->next->prior = ptr;
                    free(back);
                    pointer = ptr;
                }
            else{
                front->m_size = front->m_size + size + back->m_size;
                front->next = back->next;
                back->next->prior = front;
                free(ptr);
                free(back);
                pointer = front;
            }
        }

    /*  //最初版本，没有考虑首位分开问题     
        //前为空闲区，后为占用区
        if(front->isUsed == 0 && back->isUsed == 1){
            front->m_size += size;
            front->next = back;            
            back->prior = front;
            free(ptr);
        }
        //前后都为空闲区
        if(front->isUsed == 0 && back->isUsed == 0){
            front->m_size = front->m_size + size + back->m_size;
            front->next = back->next;
            back->next->prior = front;
            free(ptr);
            free(back);
        }
        //前为占用区，后为空闲区
        if(front->isUsed == 1 && back->isUsed == 0){
            ptr->m_size += back->m_size;
            ptr->isUsed = 0;
            ptr->next = back->next;
            back->next->prior = ptr;
            free(back);
        }
        //前后都为占用区
        if(front->isUsed == 1 && back->isUsed == 1){
            ptr->isUsed = 0;
        }
    */        
    }
    //free大小小于目标map大小
    else{
        struct map *front = ptr->prior;
        //前为占用区或者ptr为头部
        if(front->isUsed == 1 || front->m_addr > ptr->m_addr){
            struct map *new_map = malloc(sizeof(struct map));
            new_map->isUsed = 0;
            new_map->m_addr = ptr->m_addr;
            new_map->m_size = size;
            new_map->next = ptr;
            new_map->prior = front;

            front->next = new_map;
            ptr->m_size -= size;
            ptr->m_addr = (char*) ((unsigned) ptr->m_addr + size);
            ptr->prior = new_map; 
        }
        //前为空闲区且ptr不是头部
        else{
            front->m_size += size;
            ptr->m_size -= size;
            ptr->m_addr = (char*) ((unsigned) ptr->m_addr + size);
        }
    }
    printf("Freed successfully\n");
    return true;
}

int main(){
	initialize();
    printf("Initial address: %u\n", initmap->m_addr);
    print_help();
    handle_input();
    return 0;
}