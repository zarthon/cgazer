
#include "OutputMethods.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string>

int UP_COUNT=0; 
int DOWN_COUNT=0;
int LEFT_COUNT=0;
int RIGHT_COUNT=0;
char *shm = "*";
int INITIAL = 1;

AbstractStore::~AbstractStore() {
}

MmapStore::MmapStore(const char *filename) {
    fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    write(fd, &fd, sizeof(fd));
    write(fd, &fd, sizeof(fd));
    if (fd < 0) {perror("open");return;}
    positiontable = (int*) mmap(0, getpagesize(), PROT_READ | PROT_WRITE,
				MAP_SHARED, fd, 0);
}

void MmapStore::store(const TrackerOutput& output) {
    positiontable[0] = (int) output.gazepoint.x - 320;
    positiontable[1] = (int) output.gazepoint.y - 240;
}

MmapStore::~MmapStore() {
    munmap(positiontable, getpagesize());
    close(fd);
}
 
StreamStore::StreamStore(ostream &stream): stream(stream) {
}

StreamStore::~StreamStore() {
}

void initialise(){
    char c;
    int shmid;
    key_t key;

    /*
     * We'll name our shared memory segment
     * "5678".
     */
    key = 5678;

    /*
     * Create the segment.
     */
    if ((shmid = shmget(key, 27, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    /*
     * Now we attach the segment to our data space.
     */
    shm = (char*) shmat(shmid, NULL, 0);
    if ((char*) shm == (char *) (-1)) {
        perror("shmat");
        exit(1);
    }

    /*
     * Now put some things into the memory for the
     * other process to read.
     */
    INITIAL = 0;
}
void StreamStore::store(const TrackerOutput& output) {
    if (INITIAL ==1)
        initialise();

    
    char *s;
   // stream << (int) output.gazepoint.x << " " 
	//   << (int) output.gazepoint.y << " -> "<<endl;
    if((int)output.targetid ==5 || (int)output.targetid==1 || (int)output.targetid ==6 ){
        LEFT_COUNT +=1;
        if(LEFT_COUNT>=10){
            LEFT_COUNT=0;
            RIGHT_COUNT=0;
            UP_COUNT=0;
            DOWN_COUNT=0;
        	stream << (int)output.targetid << "LEFT" <<endl;
            string str( "LEFT\n\r");
            char *c;
            c = new char[str.size()+1];
            cout <<"SHM -> "<<*shm;
            if(*shm == '*' ){
                char*t;
                t = shm;
                strcpy(c,str.c_str());
                for(;*c!='\0';*c++){
                    cout << *c;
                    *t++ = *c;
                }
            }
        }
    }
    if((int)output.targetid ==8 || (int)output.targetid==2 || (int)output.targetid ==7 ){
        RIGHT_COUNT +=1;
        if(RIGHT_COUNT>=10){
            LEFT_COUNT=0;
            RIGHT_COUNT=0;
            UP_COUNT=0;
            DOWN_COUNT=0;  
        	stream << (int)output.targetid << "RIGHT"<< endl;
            string str( "RIGHT\n\r");
            char *c;
            c = new char[str.size()+1];
            cout <<"SHM -> "<<*shm;
            if(*shm == '*' ){
                char*t;
                t = shm;
                strcpy(c,str.c_str());
                for(;*c!='\0';*c++){
                    cout << *c;
                    *t++ = *c;
                }
                *t = NULL;
            }  
        } 
    }
    if((int)output.targetid ==3 || (int)output.targetid==9 || (int)output.targetid ==11 ){
        UP_COUNT +=1;
        if(UP_COUNT>=10){
            LEFT_COUNT=0;
            RIGHT_COUNT=0;
            UP_COUNT=0;
            DOWN_COUNT=0; 
        	stream << (int)output.targetid << "UP"<< endl;
            string str( "UP\n\r");
            char *c;
            c = new char[str.size()+1];
            cout <<"SHM -> "<<*shm;
            if(*shm == '*' ){
                char*t;
                t = shm;
                strcpy(c,str.c_str());
                for(;*c!='\0';*c++){
                    cout << *c;
                    *t++ = *c;
                }
                *t = NULL;

            }
        }
    }
    if((int)output.targetid ==10 || (int)output.targetid==4 || (int)output.targetid ==12 ){
        DOWN_COUNT +=1;
        if(DOWN_COUNT>=10){
            LEFT_COUNT=0;
            RIGHT_COUNT=0;
            UP_COUNT=0;
            DOWN_COUNT=0;
        	stream << (int)output.targetid << "DOWN"<< endl;
            string str( "DOWN\n\r");
            char *c;
            c = new char[str.size()+1];
            if(*shm == '*' ){
                char *t;
                t = shm;
                strcpy(c,str.c_str());
                for(;*c!='\0';*c++){
                    cout << *c;
                    *t++ = *c;
                }
                *t = NULL;

            }
        }       
    }
        stream.flush();
}

SocketStore::SocketStore(int port) {
    mysocket = socket(PF_INET, SOCK_DGRAM, 0);
	
    destaddr.sin_family = AF_INET;
    destaddr.sin_port = htons(port);
    destaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
}

void SocketStore::store(const TrackerOutput& output) {
    ostringstream stream;
    stream << "x " << (int) output.gazepoint.x << endl
	   << "y " << (int) output.gazepoint.y << endl;
    string str = stream.str();
    sendto(mysocket, str.c_str(), str.size(), 0, 
	   (sockaddr*)&destaddr, sizeof(destaddr));
}

SocketStore::~SocketStore(void) {
    close(mysocket);
}

