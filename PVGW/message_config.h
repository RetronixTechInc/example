
#define DEBUG_PRINT

#ifdef DEBUG_PRINT
 #define dbgprintf  printf
#else
 #define dbgprintf(...)
#endif

#define socket_max 3

typedef int BOOL;  
#define FALSE 0  
#define TRUE  1  

enum{
	SOCKET_INIT = 0,
	SOCKET_LOOP = 1,
	SOCKET_EXIT = 9
};

/*
 * socket manager use link method
 * 
 */
typedef struct
{
	int fd;
	struct CLIENT_ITEM* next;
	struct CLIENT_ITEM* prev;
}CLIENT_ITEM;

/*
 * socket manager for loop method
 * 
 */
typedef struct
{
	int socket_fd ;
}SOCKET_ITEM;
