#include <stdio.h>
#include <stdlib.h>
#include<string.h>
/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional or bidirectional
   data transfer protocols (from A to B. Bidirectional transfer of data
   is for extra credit and is not required).  Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

#define BIDIRECTIONAL 0    /* change to 1 if you're doing extra credit */
                           /* and write a routine called B_output */

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
struct msg {
  char data[20];
  };

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
struct pkt {
   int seqnum;
   int acknum;
   int checksum;
   char payload[20];
    };

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

//define a Packet type
typedef struct pkt Packet;
//define a Message type
typedef struct msg Message;

//Time out
#define TIME_OUT 30.0
//Window Size
#define Window_Size 8
//Maximum Buffer Size
#define Max_Buffer_Size 50

//buffer
Packet buffer[Max_Buffer_Size];
//pointing at the start of the buffer 
int buffer_Head;
//pointing at the end of the buffer 
int buffer_Tail;
//number of elements in the buffer
int bufferCount;

//window
Packet window[Window_Size];
//pointing at the start of the window 
int window_Head;
//pointing at the end of the window 
int window_Tail;
//number of elements in the window
int windowCount;

//packet sent by B
Packet pktB;

//sequence number at A
int seqA;
//sequence number at B
int seqB;
//number of packets positively acknowledged
int base;

//calculate the checksum
int calculateCheckSum(Packet packet)
{
  int check_sum = 0;
  check_sum += packet.seqnum;
	check_sum += packet.acknum;
	for (int i = 0; i < 20; i++) {
		check_sum += packet.payload[i];
	}
	return ~check_sum;
}

// print packetA contents
void printPacketA(char *init, Packet packet) 
{
	printf("%s: ", init);
	printf("sequence = %d, checksum = %x, ", packet.seqnum, packet.checksum);

	for (int i = 0; i < 20; i++)
	{
		printf("%c", packet.payload[i]);
	}
	printf("\n");
}

// print packetB contents
void printPacketB(char *init, Packet packet) 
{
	printf("%s: ", init);
	printf("acknowledgement = %d, checksum = %x, ", packet.acknum, packet.checksum);

	for (int i = 0; i < 20; i++)
	{
		printf("%c", packet.payload[i]);
	}
	printf("\n");
}

//create the packet
Packet createpacket(Message message) {
  Packet packetSent;

  //fill the payload
  memcpy(packetSent.payload, message.data, sizeof(message.data));
  //fill the sequence
  packetSent.seqnum = seqA;
  //calculate checksum
  packetSent.checksum = calculateCheckSum(packetSent);
  return packetSent;
}

/* called from layer 5, passed the data to be sent to other side */
A_output(Message message)
{
  
   //create the packet to be sent
   Packet pktA = createpacket(message);

  //if window isn't full
  if (seqA - base <  Window_Size) 
  {
        //insert the packet in window 
        window[window_Tail] = pktA;
        //modify the window tail
        window_Tail = (window_Tail + 1) % Window_Size;
        //increment the number of packets in the window
        windowCount++;

        //send the packet onto the network
	    tolayer3(0, pktA);
        printPacketA("This packet was sent by A:",pktA);
        
        // start the timer when the first packet in the window is sent
        if (seqA == base) 
        {
            starttimer(0, TIME_OUT);
            printf("Start Timer\n");
        }
        //increment the sequence of A
        seqA ++;
  } 

  //if window isn't full
  else 
  {
      printf("The window is full\n");

      //if buffer is full
      if (bufferCount == Max_Buffer_Size) 
      {
          printf("The buffer is full\n");
          return;
      }

      //if buffer isn't full
      else
      {
        //insert the packet in buffer 
        buffer[buffer_Tail] = pktA;
        //modify the buffer tail
        buffer_Tail= (buffer_Tail + 1) % Max_Buffer_Size;
        //increment the number of packets in the buffer
        bufferCount++;
      }
      
  }
}

B_output(message)  /* need be completed only for extra credit */
  struct msg message;
{
/*do nothing */
}

/* called from layer 3, when a packet arrives for layer 4 */
A_input(Packet packet)
{ 

   // if the acknowledgement sent from B is greater or equal to the sequence number and the chekcum value is valid, the packet is positively acknowledged 
   if (packet.acknum >= window[window_Head].seqnum && packet.checksum == calculateCheckSum(packet))
   {
        //stop timer
        stoptimer(0);

        //empty the window of the packets that were positively acknowledged
        window_Head = (window_Head + (packet.acknum + 1 - base)) % Window_Size;
        windowCount = windowCount - (packet.acknum + 1 - base);

        //alter the base to be equal to the number of the packets that were positively acknowledged
        base = packet.acknum + 1;

        //start the timer when there is still packets that aren't positively acknowledged
        if (base < seqA) 
        {
            starttimer(0, TIME_OUT);

        }
      
        //while the buffer isn't empty and the window isn't full
        while ((bufferCount!= 0) && (seqA - base <  Window_Size)) 
        {
           Packet pktA;
           
           //create a packet whose sequence is the next sequence value, it's data will be copied from the first packet in the buffer and it's checksum will be calculated 
           pktA.seqnum = seqA;
           for (int i =0; i<20; i++)
           {
               pktA.payload[i] = buffer[buffer_Head].payload[i];
           }
           pktA.checksum = calculateCheckSum(pktA);

           //modify the buffer head
           buffer_Head = (buffer_Head + 1) % Max_Buffer_Size;

           //insert the packet in window 
           window[window_Tail] = pktA;
           //modify the window tail
           window_Tail = (window_Tail + 1) % Window_Size;

           //increment the number of packets in the window
           windowCount++;
           //decrement the count of the buffer
           bufferCount --;

           //send the packet onto the network
	       tolayer3(0, pktA);
           printPacketA("This packet was sent by A:",pktA);
           
           // start the timer when the first packet in the window is sent
           if (base == seqA)
           {
               starttimer(0,TIME_OUT);
           } 

           //increment the sequence of A
           seqA++; 
           
        }
     
   }

   else 
   {
     printf("Something went wrong\n");
   }

}

/* called when A's timer goes off */
A_timerinterrupt()
{
  printf("A timed out\n");

  //send the packets inside the window
  for (int i = 0; i < windowCount; i++) 
  {
     tolayer3(0, window[(window_Head + i) % Window_Size]);
     printPacketA("This packet is resent:", window[(window_Head + i) % Window_Size]);
  }
  starttimer(0, TIME_OUT);

}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
A_init()
{
    window_Head = 0;
    window_Tail = 0;
    windowCount = 0;
    base = 0;

    buffer_Head = 0;
    buffer_Tail = 0;
    bufferCount = 0;
    seqA = 0;
}


/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
B_input(Packet packet)
{
  
  //if the packet recieved has the same anticipated  sequence number asnd wasn't corrupted
  if (packet.seqnum == seqB && packet.checksum == calculateCheckSum(packet))
	{
		printf("The packet is valid\n");

        //acknowledgement sent with the same sequence number
		pktB.acknum = seqB;
		pktB.checksum = calculateCheckSum(pktB);

        //B sequence number will be incremented to expect the next packet
        seqB ++;

        //send data to the application layer above
		tolayer5(1, packet.payload);
        //send packetB to A
	    tolayer3(1, pktB);
        printPacketB("This packet was sent by B:",pktB);
	}

	else 
    {
		printf("The packet is invalid\n");
        //send packetB to A
	    tolayer3(1, pktB);
        printPacketB("This packet was sent by B:",pktB);
	}
   
}

/* called when B's timer goes off */
B_timerinterrupt()
{
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
B_init()
{
  seqB = 0;
}


/*****************************************************************
***************** NETWORK EMULATION CODE STARTS BELOW ***********
The code below emulates the layer 3 and below network environment:
  - emulates the tranmission and delivery (possibly with bit-level corruption
    and packet loss) of packets across the layer 3/4 interface
  - handles the starting/stopping of a timer, and generates timer
    interrupts (resulting in calling students timer handler).
  - generates message to be sent (passed from later 5 to 4)

THERE IS NOT REASON THAT ANY STUDENT SHOULD HAVE TO READ OR UNDERSTAND
THE CODE BELOW.  YOU SHOLD NOT TOUCH, OR REFERENCE (in your code) ANY
OF THE DATA STRUCTURES BELOW.  If you're interested in how I designed
the emulator, you're welcome to look at the code - but again, you should have
to, and you defeinitely should not have to modify
******************************************************************/

struct event {
   float evtime;           /* event time */
   int evtype;             /* event type code */
   int eventity;           /* entity where event occurs */
   struct pkt *pktptr;     /* ptr to packet (if any) assoc w/ this event */
   struct event *prev;
   struct event *next;
 };
struct event *evlist = NULL;   /* the event list */

/* possible events: */
#define  TIMER_INTERRUPT 0
#define  FROM_LAYER5     1
#define  FROM_LAYER3     2

#define  OFF             0
#define  ON              1
#define   A    0
#define   B    1



int TRACE = 1;             /* for my debugging */
int nsim = 0;              /* number of messages from 5 to 4 so far */
int nsimmax = 0;           /* number of msgs to generate, then stop */
float time = 0.000;
float lossprob;            /* probability that a packet is dropped  */
float corruptprob;         /* probability that one bit is packet is flipped */
float lambda;              /* arrival rate of messages from layer 5 */
int   ntolayer3;           /* number sent into layer 3 */
int   nlost;               /* number lost in media */
int ncorrupt;              /* number corrupted by media*/

main()
{
   struct event *eventptr;
   struct msg  msg2give;
   struct pkt  pkt2give;

   int i,j;
   char c;

   init();
   A_init();
   B_init();

   while (1) {
        eventptr = evlist;            /* get next event to simulate */
        if (eventptr==NULL)
           goto terminate;
        evlist = evlist->next;        /* remove this event from event list */
        if (evlist!=NULL)
           evlist->prev=NULL;
        if (TRACE>=2) {
           printf("\nEVENT time: %f,",eventptr->evtime);
           printf("  type: %d",eventptr->evtype);
           if (eventptr->evtype==0)
	       printf(", timerinterrupt  ");
             else if (eventptr->evtype==1)
               printf(", fromlayer5 ");
             else
	     printf(", fromlayer3 ");
           printf(" entity: %d\n",eventptr->eventity);
           }
        time = eventptr->evtime;        /* update time to next event time */
        if (nsim==nsimmax)
	  break;                        /* all done with simulation */
        if (eventptr->evtype == FROM_LAYER5 ) {
            generate_next_arrival();   /* set up future arrival */
            /* fill in msg to give with string of same letter */
            j = nsim % 26;
            for (i=0; i<20; i++)
               msg2give.data[i] = 97 + j;
            if (TRACE>2) {
               printf("          MAINLOOP: data given to student: ");
                 for (i=0; i<20; i++)
                  printf("%c", msg2give.data[i]);
               printf("\n");
	     }
            nsim++;
            if (eventptr->eventity == A)
               A_output(msg2give);
             else
               B_output(msg2give);
            }
          else if (eventptr->evtype ==  FROM_LAYER3) {
            pkt2give.seqnum = eventptr->pktptr->seqnum;
            pkt2give.acknum = eventptr->pktptr->acknum;
            pkt2give.checksum = eventptr->pktptr->checksum;
            for (i=0; i<20; i++)
                pkt2give.payload[i] = eventptr->pktptr->payload[i];
	    if (eventptr->eventity ==A)      /* deliver packet by calling */
   	       A_input(pkt2give);            /* appropriate entity */
            else
   	       B_input(pkt2give);
	    free(eventptr->pktptr);          /* free the memory for packet */
            }
          else if (eventptr->evtype ==  TIMER_INTERRUPT) {
            if (eventptr->eventity == A)
	       A_timerinterrupt();
             else
	       B_timerinterrupt();
             }
          else  {
	     printf("INTERNAL PANIC: unknown event type \n");
             }
        free(eventptr);
        }

terminate:
   printf(" Simulator terminated at time %f\n after sending %d msgs from layer5\n",time,nsim);
}



init()                         /* initialize the simulator */
{
  int i;
  float sum, avg;
  float jimsrand();


   printf("-----  Stop and Wait Network Simulator Version 1.1 -------- \n\n");
   printf("Enter the number of messages to simulate: ");
   scanf("%d",&nsimmax);
   printf("Enter  packet loss probability [enter 0.0 for no loss]:");
   scanf("%f",&lossprob);
   printf("Enter packet corruption probability [0.0 for no corruption]:");
   scanf("%f",&corruptprob);
   printf("Enter average time between messages from sender's layer5 [ > 0.0]:");
   scanf("%f",&lambda);
   printf("Enter TRACE:");
   scanf("%d",&TRACE);

   srand(9999);              /* init random number generator */
   sum = 0.0;                /* test random number generator for students */
   for (i=0; i<1000; i++)
      sum=sum+jimsrand();    /* jimsrand() should be uniform in [0,1] */
   avg = sum/1000.0;
   if (avg < 0.25 || avg > 0.75) {
    printf("It is likely that random number generation on your machine\n" );
    printf("is different from what this emulator expects.  Please take\n");
    printf("a look at the routine jimsrand() in the emulator code. Sorry. \n");
    exit(0);
    }

   ntolayer3 = 0;
   nlost = 0;
   ncorrupt = 0;

   time=0.0;                    /* initialize time to 0.0 */
   generate_next_arrival();     /* initialize event list */
}

/****************************************************************************/
/* jimsrand(): return a float in range [0,1].  The routine below is used to */
/* isolate all random number generation in one location.  We assume that the*/
/* system-supplied rand() function return an int in therange [0,mmm]        */
/****************************************************************************/
float jimsrand()
{
  double mmm = (double) RAND_MAX ;//2147483647;   /* largest int  - MACHINE DEPENDENT!!!!!!!!   */
  float x;                   /* individual students may need to change mmm */
  x = rand()/mmm;            /* x should be uniform in [0,1] */
  return(x);
}

/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/

generate_next_arrival()
{
   double x,log(),ceil();
   struct event *evptr;
//   char *malloc();
   float ttime;
   int tempint;

   if (TRACE>2)
       printf("          GENERATE NEXT ARRIVAL: creating new arrival\n");

   x = lambda*jimsrand()*2;  /* x is uniform on [0,2*lambda] */
                             /* having mean of lambda        */
   evptr = (struct event *)malloc(sizeof(struct event));
   evptr->evtime =  time + x;
   evptr->evtype =  FROM_LAYER5;
   if (BIDIRECTIONAL && (jimsrand()>0.5) )
      evptr->eventity = B;
    else
      evptr->eventity = A;
   insertevent(evptr);
}


insertevent(p)
   struct event *p;
{
   struct event *q,*qold;

   if (TRACE>2) {
      printf("            INSERTEVENT: time is %lf\n",time);
      printf("            INSERTEVENT: future time will be %lf\n",p->evtime);
      }
   q = evlist;     /* q points to header of list in which p struct inserted */
   if (q==NULL) {   /* list is empty */
        evlist=p;
        p->next=NULL;
        p->prev=NULL;
        }
     else {
        for (qold = q; q !=NULL && p->evtime > q->evtime; q=q->next)
              qold=q;
        if (q==NULL) {   /* end of list */
             qold->next = p;
             p->prev = qold;
             p->next = NULL;
             }
           else if (q==evlist) { /* front of list */
             p->next=evlist;
             p->prev=NULL;
             p->next->prev=p;
             evlist = p;
             }
           else {     /* middle of list */
             p->next=q;
             p->prev=q->prev;
             q->prev->next=p;
             q->prev=p;
             }
         }
}

printevlist()
{
  struct event *q;
  int i;
  printf("--------------\nEvent List Follows:\n");
  for(q = evlist; q!=NULL; q=q->next) {
    printf("Event time: %f, type: %d entity: %d\n",q->evtime,q->evtype,q->eventity);
    }
  printf("--------------\n");
}



/********************** Student-callable ROUTINES ***********************/

/* called by students routine to cancel a previously-started timer */
stoptimer(AorB)
int AorB;  /* A or B is trying to stop timer */
{
 struct event *q,*qold;

 if (TRACE>2)
    printf("          STOP TIMER: stopping timer at %f\n",time);
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
 for (q=evlist; q!=NULL ; q = q->next)
    if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) {
       /* remove this event */
       if (q->next==NULL && q->prev==NULL)
             evlist=NULL;         /* remove first and only event on list */
          else if (q->next==NULL) /* end of list - there is one in front */
             q->prev->next = NULL;
          else if (q==evlist) { /* front of list - there must be event after */
             q->next->prev=NULL;
             evlist = q->next;
             }
           else {     /* middle of list */
             q->next->prev = q->prev;
             q->prev->next =  q->next;
             }
       free(q);
       return;
     }
  printf("Warning: unable to cancel your timer. It wasn't running.\n");
}


starttimer(AorB,increment)
int AorB;  /* A or B is trying to stop timer */
float increment;
{

 struct event *q;
 struct event *evptr;
// char *malloc();

 if (TRACE>2)
    printf("          START TIMER: starting timer at %f\n",time);
 /* be nice: check to see if timer is already started, if so, then  warn */
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
   for (q=evlist; q!=NULL ; q = q->next)
    if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) {
      printf("Warning: attempt to start a timer that is already started\n");
      return;
      }

/* create future event for when timer goes off */
   evptr = (struct event *)malloc(sizeof(struct event));
   evptr->evtime =  time + increment;
   evptr->evtype =  TIMER_INTERRUPT;
   evptr->eventity = AorB;
   insertevent(evptr);
}


/************************** TOLAYER3 ***************/
tolayer3(AorB,packet)
int AorB;  /* A or B is trying to stop timer */
struct pkt packet;
{
 struct pkt *mypktptr;
 struct event *evptr,*q;
// char *malloc();
 float lastime, x, jimsrand();
 int i;


 ntolayer3++;

 /* simulate losses: */
 if (jimsrand() < lossprob)  {
      nlost++;
      if (TRACE>0)
	printf("          TOLAYER3: packet being lost\n");
      return;
    }

/* make a copy of the packet student just gave me since he/she may decide */
/* to do something with the packet after we return back to him/her */
 mypktptr = (struct pkt *)malloc(sizeof(struct pkt));
 mypktptr->seqnum = packet.seqnum;
 mypktptr->acknum = packet.acknum;
 mypktptr->checksum = packet.checksum;
 for (i=0; i<20; i++)
    mypktptr->payload[i] = packet.payload[i];
 if (TRACE>2)  {
   printf("          TOLAYER3: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
	  mypktptr->acknum,  mypktptr->checksum);
    for (i=0; i<20; i++)
        printf("%c",mypktptr->payload[i]);
    printf("\n");
   }

/* create future event for arrival of packet at the other side */
  evptr = (struct event *)malloc(sizeof(struct event));
  evptr->evtype =  FROM_LAYER3;   /* packet will pop out from layer3 */
  evptr->eventity = (AorB+1) % 2; /* event occurs at other entity */
  evptr->pktptr = mypktptr;       /* save ptr to my copy of packet */
/* finally, compute the arrival time of packet at the other end.
   medium can not reorder, so make sure packet arrives between 1 and 10
   time units after the latest arrival time of packets
   currently in the medium on their way to the destination */
 lastime = time;
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
 for (q=evlist; q!=NULL ; q = q->next)
    if ( (q->evtype==FROM_LAYER3  && q->eventity==evptr->eventity) )
      lastime = q->evtime;
 evptr->evtime =  lastime + 1 + 9*jimsrand();



 /* simulate corruption: */
 if (jimsrand() < corruptprob)  {
    ncorrupt++;
    if ( (x = jimsrand()) < .75)
       mypktptr->payload[0]='Z';   /* corrupt payload */
      else if (x < .875)
       mypktptr->seqnum = 999999;
      else
       mypktptr->acknum = 999999;
    if (TRACE>0)
	printf("          TOLAYER3: packet being corrupted\n");
    }

  if (TRACE>2)
     printf("          TOLAYER3: scheduling arrival on other side\n");
  insertevent(evptr);
}

tolayer5(AorB,datasent)
  int AorB;
  char datasent[20];
{
  int i;
  if (TRACE>2) {
     printf("          TOLAYER5: data received: ");
     for (i=0; i<20; i++)
        printf("%c",datasent[i]);
     printf("\n");
   }

}
