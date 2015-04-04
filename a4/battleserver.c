/*
 * socket demonstrations:
 * This is the server side of an "internet domain" socket connection, for
 * communicating over the network.
 *
 * In this case we are willing to wait either for chatter from the client
 * _or_ for a new connection.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 #include <time.h>

#ifndef PORT
    #define PORT 32422
#endif

struct client {
    int fd;    //File-descriptor
    struct in_addr ipaddr; 
    
    char *name; 
     //Address of the client
    struct client *next;
      //Next-client
    int hitpoints;
    int powermoves;
    int status;//-1:defend, 1:attack, 0:waiting,no game
    struct client *opponent;
    struct client *lastplayed;
    char buf[256];
    char name_buf[256];


};
//Create a client struct and put to the head of TOP
static struct client *addclient(struct client *top, int fd, struct in_addr addr);
//Remove specific client with fd
static struct client *removeclient(struct client *top, int fd);
//write each client->fd to a char s (so client can get soemthing from the server)
static void broadcast(struct client *top, char *s, int size);

int handleclient(struct client *p, struct client *top);
//Give a listening socket descriptor
int bindandlisten(void); //Bind and listen(server)
void moveToEnd(struct client *top, struct client *p);


int main(void) {
    int clientfd, maxfd, nready;
    struct client *p;
    struct client *head = NULL;
    socklen_t len;
    struct sockaddr_in q;
    struct timeval tv;
    fd_set allset;
    fd_set rset;
    srand (time(NULL));

    int i;

    //Initialize a listening fd of the server.
    int listenfd = bindandlisten();

    // initialize allset and add listenfd to the
    // set of file descriptors passed into select
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    
    // maxfd identifies how far into the set to search
    maxfd = listenfd;

    while (1) {//keep running
        // make a copy of the set before we pass it into select
        rset = allset;
        /* timeout in seconds (You may not need to use a timeout for
        * your assignment)*/
        tv.tv_sec = 2; // change to 0 from 10
        tv.tv_usec = 0;  /* and microseconds */

        nready = select(maxfd + 1, &rset, NULL, NULL, &tv); //Select a ready fd descriptor(client) to read.
        if (nready == 0) {
            printf("No response from clients in %ld seconds\n", tv.tv_sec);
            continue;
        }

        if (nready == -1) {
            perror("select");
            continue;
        }

        if (FD_ISSET(listenfd, &rset)){
            printf("a new client is connecting\n");
            len = sizeof(q);
            if ((clientfd = accept(listenfd, (struct sockaddr *)&q, &len)) < 0) {
                perror("accept");
                exit(1);
            }
            FD_SET(clientfd, &allset);
            if (clientfd > maxfd) {
                maxfd = clientfd;
            }
            printf("connection from %s\n", inet_ntoa(q.sin_addr));
            //sprintf("what is your name?");
            head = addclient(head, clientfd, q.sin_addr);
        }
        //Handle every client
        //printf("running\n");
        for(i = 0; i <= maxfd; i++) {
            if (FD_ISSET(i, &rset)) {
                for (p = head; p != NULL; p = p->next) {
                    if (p->fd == i) {//
                        int result = handleclient(p, head);
                        if (result == -1) {
                            int tmp_fd = p->fd;
                            head = removeclient(head, p->fd);
                            FD_CLR(tmp_fd, &allset);
                            close(tmp_fd);
                        }
                        break;
                    }    
                }
            }
        }
    }
    return 0;
}
int find_network_newline(char *buf, int inbuf) {
  // Step 1: write this function 
  int i;
  for (i=0;i<inbuf;i++){
    if (buf[i] == '\n'){
      return i;
    }
  }
  return -1; // return the location of '\r' if found
}

int handleclient(struct client *p, struct client *top) {
    char outbuf[512];
    struct client *oppo;
    int len;
    //New client, ask the name.
    if (p->name == NULL){//Ask the name

        int inbuf = 0; 
        int where;
                 // buffer is empty; has no bytes
        int room = sizeof(p->name_buf); // room == capacity of the whole buffer
        char *after;
        after = p->name_buf;   
        //Return number of bytes read
        //nbytes = len
        while (( len = read(p->fd, after, sizeof(p->name_buf) - 1)) > 0) {
            inbuf += len;
            where = find_network_newline(p->name_buf, inbuf);
        
            if (where >= 0) { // OK. we have a full line
              p->name_buf[where] = '\0';
              p->name_buf[where+1] = '\0';  
              inbuf -= where + 2 ;
              break;                
            }
            room = sizeof(p->name_buf) - inbuf;
            after = p->name_buf + inbuf;
        }
        p->name = p->name_buf;

    
        //Tell the world someone joined.
        sprintf(outbuf, "\r\n**%s joined the area.**\r\n", p->name);
        broadcast(top, outbuf, strlen(outbuf));
        sprintf(outbuf, "Welcome, %s! Awaiting opponent...\n", p->name);
        write(p->fd, outbuf, strlen(outbuf));
    }  
    
    //Start matching.
    if(p->opponent == NULL){
        for (oppo = top; oppo != NULL; oppo = oppo->next){
            if(oppo != p){
                if(oppo->opponent == NULL && oppo->lastplayed != p){
                    p->opponent = oppo;           
                    p->hitpoints = rand() % 11 + 20;
                    p->powermoves = rand() % 3 + 1;
                    p->status = 1;//Attacking first
                    
                    oppo->opponent = p;
                    oppo->hitpoints = rand() % 11 + 20;
                    oppo->powermoves = rand() % 3 + 1;
                    oppo->status = -1;//Defend first.
                    sprintf(outbuf, "You engage %s!\n", p->opponent->name);
                    write(p->fd, outbuf, strlen(outbuf));
                    sprintf(outbuf, "You engage %s!\n", p->name);
                    write(p->opponent->fd, outbuf, strlen(outbuf));
                    sprintf(outbuf, "\nReady for battle?(Please type 'y')\n");
                    write(p->fd, outbuf, strlen(outbuf));              
                }
            }      
        }
        return 0; 
    }

    len = read(p->fd, p->buf, sizeof(p->buf) - 1);
    //Handle battle.
    if(p->opponent){
         if (p->status == 1){
            sprintf(outbuf, "Your hitpoints:%d\nYour powermoves: %d\n\n%s's hitpoints:%d\n\n", 
                p->hitpoints, p->powermoves, p->opponent->name, p->opponent->hitpoints);
            write(p->fd, outbuf, strlen(outbuf));
            sprintf(outbuf, "Your hitpoints:%d\nYour powermoves: %d\n\n%s's hitpoints:%d\n\n", 
                p->opponent->hitpoints, p->opponent->powermoves, p->name, p->hitpoints);
            write(p->opponent->fd, outbuf, strlen(outbuf));
            sprintf(outbuf, "(a)ttack\n(p)owermove\n(s)peak something\n");
            write(p->fd, outbuf, strlen(outbuf));
            sprintf(outbuf, "Waiting for %s to strike...\n", p->name);
            write(p->opponent->fd, outbuf, strlen(outbuf));
            len = read(p->fd, p->buf, sizeof(p->buf) - 1); 
            p->status = -1;
            p->opponent->status = 1;
        }else if(p->status == -1){
            return 0;
        }
        if (len > 0) {
            if(p->buf[0] == 'a'){
                p->buf[0] = '\0';
                int damage;
                damage = rand() % 5 + 2;
                p->opponent->hitpoints -= damage;
                sprintf(outbuf, "\nYou hit %s for %d damage!\n", p->opponent->name, damage);
                write(p->fd, outbuf, strlen(outbuf));//write outbuf to every client in top.
                sprintf(outbuf, "\n%s hits you for %d damage!\n", p->name, damage);
                write(p->opponent->fd, outbuf, strlen(outbuf));
                sprintf(outbuf, "\nReady for attacking/speaking?(Please say 'y')\n");
                write(p->opponent->fd, outbuf, strlen(outbuf));
                
                if(p->opponent->hitpoints <= 0){
                    sprintf(outbuf, "\n%s gives up. You win!\n", p->opponent->name);
                    write(p->fd, outbuf, strlen(outbuf));
                    sprintf(outbuf, "\nYou are no match for %s. You scurry away...\n", p->name);
                    write(p->opponent->fd, outbuf, strlen(outbuf));
                    sprintf(outbuf, "Awaiting opponent...\n");
                    write(p->fd, outbuf, strlen(outbuf));
                    write(p->opponent->fd, outbuf, strlen(outbuf));    
                    p->lastplayed = p->opponent;
                    p->opponent->lastplayed = p;
                    moveToEnd(top, p);
                    moveToEnd(top, p->opponent);
                    p->opponent->opponent = NULL;
                    p->opponent = NULL;
                }

                return 0;
            }else if(p->buf[0] == 'p'){
                p->buf[0] = '\0';
                int possibility;
                int damage;
                possibility = rand() % 2 + 0;
                p->powermoves -= 1;
                if(possibility == 0){//Miss it;
                    sprintf(outbuf, "\nYou missed %s!\n", p->opponent->name);
                    write(p->fd, outbuf, strlen(outbuf));
                    sprintf(outbuf, "\n%s missed you!\n", p->name);
                    write(p->opponent->fd, outbuf, strlen(outbuf));
                    sprintf(outbuf, "\nReady for attacking/speaking?(Please say 'y')\n");
                    write(p->opponent->fd, outbuf, strlen(outbuf));
              
                    return 0;
                }else{
                    damage = 3 * (rand() % 5 + 2);
                    p->opponent->hitpoints -= damage;
                    sprintf(outbuf, "\nYou powermoves %s for %d damage!\n", p->opponent->name, damage);
                    write(p->fd, outbuf, strlen(outbuf));//write outbuf to every client in top.
                    sprintf(outbuf, "\n%s powermoves you for %d damage!\n", p->name, damage);
                    write(p->opponent->fd, outbuf, strlen(outbuf));
                    sprintf(outbuf, "\nReady for attacking/speaking?(Please say 'y')\n");
                    write(p->opponent->fd, outbuf, strlen(outbuf));
                    if(p->opponent->hitpoints <= 0){
                        sprintf(outbuf, "\n%s gives up. You win!\n", p->opponent->name);
                        write(p->fd, outbuf, strlen(outbuf));
                        sprintf(outbuf, "\nYou are no match for %s. You scurry away...\n", p->name);
                        write(p->opponent->fd, outbuf, strlen(outbuf));
                        sprintf(outbuf, "Awaiting opponent...\n");
                        write(p->fd, outbuf, strlen(outbuf));
                        write(p->opponent->fd, outbuf, strlen(outbuf));    
                        p->lastplayed = p->opponent;
                        p->opponent->lastplayed = p;
                        moveToEnd(top, p);
                        moveToEnd(top, p->opponent); 
                        p->opponent->opponent = NULL;
                        p->opponent = NULL;                     
                    }
          
                    return 0;
                }
            }else if(p->buf[0] == 's'){
                    p->buf[0] = '\0';
                    sprintf(outbuf, "Speak:");
                    write(p->fd, outbuf, strlen(outbuf));
                    int inbuf = 0; 
                    int where;
                    int room = sizeof(p->buf); 
                    char *after;
                    //char name_buf[256];
                    p->buf[0] = '\0';
                    after = p->buf;   
            
                    while (( len = read(p->fd, after, sizeof(p->buf) - 1)) > 0) {
                        inbuf += len;
                        where = find_network_newline(p->buf, inbuf);
            
                        if (where >= 0) { // OK. we have a full line
                            p->buf[where] = '\0';
                            p->buf[where+1] = '\0';  
                            inbuf -= where + 2 ;
                             break;                
                        }
                        room = sizeof(p->buf) - inbuf;
                        after = p->buf + inbuf;
                    }
                sprintf(outbuf, "You spoke:%s\n", p->buf);
                write(p->fd, outbuf, strlen(outbuf));
                sprintf(outbuf, "%s spoke:%s\n", p->name, p->buf);
                write(p->opponent->fd, outbuf, strlen(outbuf));
                sprintf(outbuf, "\nReady for attacking/speaking?(Please say 'y')\n");
                write(p->opponent->fd, outbuf, strlen(outbuf));

                return 0;
            }    
        } else if (len == 0) {//nothing in client p's fd
            // socket is closed
            printf("active\n");
            sprintf(outbuf, "Awaiting opponent...\n");
            write(p->opponent->fd, outbuf, strlen(outbuf));
            p->opponent->opponent = NULL;
            sprintf(outbuf, "--%s dropped. You win!\n", p->name);
            write(p->opponent->fd, outbuf, strlen(outbuf));
            printf("Disconnect from %s\n", inet_ntoa(p->ipaddr));
            return -1;
        } else { // shouldn't happen
            perror("read");
            return -1;
        }
    }
    return -1;
}

 /* bind and listen, abort on error
  * returns FD of listening socket
  */
void moveToEnd(struct client *top, struct client *p){
    struct client *temp;
    for(temp = top; temp != NULL; temp=temp->next){
        if(temp->next == p){
            temp->next = p->next;
            p->next = NULL;
        }
        if(temp->next == NULL){
            temp->next = p;
        }
    }
}
int bindandlisten(void) {
    struct sockaddr_in r;
    int listenfd;
    //Return a socket descriptor.
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
    int yes = 1;
    if ((setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) == -1) {
        perror("setsockopt");
    }
    memset(&r, '\0', sizeof(r));
    r.sin_family = AF_INET;
    r.sin_addr.s_addr = INADDR_ANY;
    r.sin_port = htons(PORT);

    if (bind(listenfd, (struct sockaddr *)&r, sizeof r)) {
        perror("bind");
        exit(1);
    }

    if (listen(listenfd, 5)) {
        perror("listen");
        exit(1);
    }
    return listenfd;
}

static struct client *addclient(struct client *top, int fd, struct in_addr addr) {
    char outbuf[512];
    //char name_buf[254];
    struct client *p = malloc(sizeof(struct client));
    if (!p) {
        perror("malloc");
        exit(1);
    }

    printf("Adding client %s\n", inet_ntoa(addr));

    p->fd = fd;
    p->ipaddr = addr;
    p->next = top;
    p->lastplayed = NULL;
    p->hitpoints = 0;
    //p->buf = NULL;
    p->powermoves = 0;
    p->opponent = NULL;
    //p->status = 0;//waiting
    p->name = NULL;
    int len;
    //Read the client fd to buf
    sprintf(outbuf, "What is your name?");
    write(p->fd, outbuf, strlen(outbuf));
    
    top = p;
    return top;
}

static struct client *removeclient(struct client *top, int fd) {
    struct client **p;
    char outbuf[512];
    for (p = &top; *p && (*p)->fd != fd; p = &(*p)->next)
        ;
    // Now, p points to (1) top, or (2) a pointer to another client
    // This avoids a special case for removing the head of the list
    if (*p) {
        struct client *t = (*p)->next;
        printf("Removing client %d %s\n", fd, inet_ntoa((*p)->ipaddr));
        free(*p);
        *p = t;
        sprintf(outbuf, "**%s left the arena.**\n", (*p)->name);//Put a message in outbuf
        broadcast(top, outbuf, strlen(outbuf));//write outbuf to every client in top.
    } else {
        fprintf(stderr, "Trying to remove fd %d, but I don't know about it\n",
                 fd);
    }
    return top;
}


static void broadcast(struct client *top, char *s, int size) {
    struct client *p;
    for (p = top; p; p = p->next) {
        write(p->fd, s, size);//Wrire s to p->fd
    }
    /* should probably check write() return value and perhaps remove client */
}
