#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#ifndef PORT
  #define PORT 56699
#endif

int setup (void) {
  int on = 1, status;
  struct sockaddr_in self;
  int listenfd;
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }

  // Make sure we can reuse the port immediately after the
  // server terminates.
  status = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                      (const char *) &on, sizeof(on));
  if(status == -1) {
    perror("setsockopt -- REUSEADDR");
  }

  memset(&self, '\0', sizeof(self));
  self.sin_family = AF_INET;
  self.sin_addr.s_addr = INADDR_ANY;
  self.sin_port = htons(PORT);
  printf("Listening on %d\n", PORT);

  if (bind(listenfd, (struct sockaddr *)&self, sizeof(self)) == -1) {
    perror("bind"); // probably means port is in use
    exit(1);
  }

  if (listen(listenfd, 5) == -1) {
    perror("listen");
    exit(1);
  }
  return listenfd;
}

/*Search the first inbuf characters of buf for a network newline ("\r\n").
  Return the location of the '\r' if the network newline is found,
  or -1 otherwise.
  Definitely do not use strchr or any other string function in here. (Why not?)
*/

int find_network_newline(char *buf, int inbuf) {
  // Step 1: write this function
    int i;
    for(i=0;i<inbuf;i++){
      if(!strcmp(&buf[i], "\r")){
        if(buf[i+1]){
          if(!strcmp(&buf[i+1], "\n")){
            return i;
          }
        }
      }
    } 
  return -1; // return the location of '\r' if found
}

int main(void) {
  int listenfd;
  int fd, nbytes;
  int inbuf; // how many bytes currently in buffer?
  int room; // how much room left in buffer?
  char *after; // pointer to position after the data in buf
  int where; // location of network newline

  struct sockaddr_in peer;
  socklen_t socklen;
  char buf[30];

  listenfd = setup();
  while (1) {
    socklen = sizeof(peer);
    if ((fd = accept(listenfd, (struct sockaddr *)&peer, &socklen)) < 0) {
      perror("accept");

    } else {
      printf("New connection on port %d\n", ntohs(peer.sin_port));

      // Receive messages
      inbuf = 0;          // buffer is empty; has no bytes
      room = sizeof(buf); // room == capacity of the whole buffer
      after = buf;        // start writing at beginning of buf

      while ((nbytes = read(fd, after, room)) > 0) {
        // Step 2: update inbuf
        if(nbytes <= room){
          inbuf += nbytes;
        }
        
        // Step 3: call find_network_newline, store result in where
        where = find_network_newline(after, strlen(after));

        
        if (where >= 0) { // OK. we have a full line

          // Step 4: output the full line using print statment below
          // Be sure to put a '\0' in the correct place first;
          // otherwise you'll get junk in the output.
          // (Replace the "\r\n" with appropriate characters so the 
          // message prints correctly to stdout.)
          printf ("Next message: %.*s\n", where, buf);
          
          // Note that we could have also used write to avoid having to
          // put the '\0' in the buffer. Try using write later!
          
          
          // Step 5: update inbuf and remove the full line from the buffer
          // There might be stuff after the line, so don't just do inbuf = 0
          inbuf = inbuf - where;



          
          // You want to move the stuff after the full line to the beginning 
          // of the buffer.  A loop can do it, or you can use memmove.
          // memmove(destination, source, number_of_bytes)
          memmove(&buf[0], &buf[where+2], strlen(buf)-strlen(after));

            
        }
        // Step 6: update room and after, in preparation for the next read
        room = sizeof(buf) - strlen(buf);
        after = &buf[strlen(buf)];


       
      }
      close (fd);
    }
  }
  return 0;
}
   
