/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Main Test program.

#pragma ident "@(#)Main.cc	1.53 97/04/15 SMI"

// Include Files.
#include <iostream.h>
#include <stdlib.h>
#include "Main.hh"

#include <unistd.h>
#include <stropts.h>

//========================GLOBAL ADMIN SPACE===================================


static void Usage();
static void ParseCommands(int pargc, char **pargv);


static int printInfo = 0;
static int runAsync = 0;
static int runConnection = 0;
static int runDataTypes = 0;
static int runSession = 0;
static int runMessgStore = 0;
static int runMessg = 0;
static int runMessgEnv = 0;
static int runMessgBody = 0;
static int runBasic = 0;
static int runUtils = 0;
static int runSearch = 0;
static int runCompose = 0;
static int runOutgoingMessg = 0;
static int runDisconnect = 0;
static int runUnsolicitEvent = 0;


int
main(int argc, char** argv)
{
  cout << endl << "************SdmMain Invoked.*************" << endl << endl;

  ParseCommands(argc, argv);

  if (printInfo) {
    PrintSizeOfSelectedObjects();
  }

  if (runAsync) {
    if (RunAsyncTest() != Sdm_EC_Success) {
        cout << endl << "** AsyncTest failed *** \n";
    } else {
        cout << endl << "** AsyncTest passed *** \n";
    }
  }
  
  
  if (runConnection) {
    if (RunConnectionTest() != Sdm_EC_Success) {
        cout << endl << "** ConnectionTest failed *** \n";
     } else {
        cout << endl << "** ConnectionTest passed *** \n";
   }
  }
  if (runDataTypes) {
    if (RunDataTypeTest() != Sdm_EC_Success) {
        cout << endl << "** DataTypeTest failed *** \n";
     } else {
        cout << endl << "** DataTypeTest passed *** \n";
   }
  }
  if (runDisconnect) {
    if (RunDisconnectTest() != Sdm_EC_Success) {
        cout << endl << "** DisconnectTest failed *** \n";
     } else {
        cout << endl << "** DisconnectTest passed *** \n";
   }
  }
  if (runUnsolicitEvent) {
    if (RunUnsolicitEventTest() != Sdm_EC_Success) {
        cout << endl << "** UnsolicitEventTest failed *** \n";
     } else {
        cout << endl << "** UnsolicitEventTest passed *** \n";
   }
  }
  if (runSession) {
    if (RunSessionTest() != Sdm_EC_Success) {
        cout << endl << "** SessionTest failed *** \n";
    } else {
        cout << endl << "** SessionTest passed *** \n";
    }
  }
  if (runMessgStore) {
    if (RunMessageStoreTest() != Sdm_EC_Success) {
        cout << endl << "** MessgStoreTest failed *** \n";
    } else {
        cout << endl << "** MessgStoreTest passed *** \n";
    }
  }
  if (runMessg) {
    if (RunMessageTest() != Sdm_EC_Success) {
        cout << endl << "** MessgTest failed *** \n";
    } else {
        cout << endl << "** MessgTest passed *** \n";
    }
  }
  if (runMessgEnv) {
    if (RunMessageEnvelopeTest() != Sdm_EC_Success) {
        cout << endl << "** MessageEnvelopeTest failed *** \n";
    } else {
        cout << endl << "** MessageEnvelopeTest passed *** \n";
    }
  }
  if (runMessgBody) {
    if (RunMessageBodyTest() != Sdm_EC_Success) {
        cout << endl << "** MessgBodyTest failed *** \n";
    } else {
        cout << endl << "** MessgBodyTest passed *** \n";
    }
  }
  if (runBasic) {
    if (RunBasicTest() != Sdm_EC_Success) {
        cout << endl << "** BasicTest failed *** \n";
    } else {
        cout << endl << "** BasicTest passed *** \n";
    }
  }
  if (runCompose) {
    if (RunComposeTest() != Sdm_EC_Success) {
        cout << endl << "** ComposeTest failed *** \n";
    } else {
        cout << endl << "** ComposeTest passed *** \n";
    }
  }
  if (runOutgoingMessg) {
    if (RunOutgoingMessageTest() != Sdm_EC_Success) {
        cout << endl << "** OutgoingMessageTest failed *** \n";
    } else {
        cout << endl << "** OutgoingMessageTest passed *** \n";
    }
  }
  if (runUtils) {
    if (RunUtilityTest() != Sdm_EC_Success) {
        cout << endl << "** UtilityTest failed *** \n";
    } else {
        cout << endl << "** UtilityTest passed *** \n";
    }
  }
  if (runSearch) {
    if (RunSearchTest() != Sdm_EC_Success) {
        cout << endl << "** SearchTest failed *** \n";
    } else {
        cout << endl << "** SearchTest passed *** \n";
    }
  }
}

static
void ParseCommands(int pargc, char **pargv)
{
  int i, j;
  if (pargc > 1)
    for(i = 1; i < pargc; i++) {
    
        if ( pargv[i][0] != '-' )   {                    // Command switch?
              continue;
        } else  {         // Process command line switch option
            int i1 = i;
 
            for (j = 1; pargv[i1][j] != 0; j++)  {
            
                 switch(pargv[i1][1])
                  {
                    /*
                     * HANDLE SWITCHES WE RECOGNIZE
                     */
                  case 'a':     // -a; run asynchronous test
                    runAsync = 1;
                    break;
                  case 'p':     // -p; print out object info
                    printInfo = 1;
                    break;
                  case 'c':     // -c; run connection test.
                    runConnection = 1;
                    continue;
                  case 'd':     // -d; run datatypes test.
                    runDataTypes = 1;
                    continue;
#ifdef INCLUDE_DISCONNECT
                  case 'D':     // -D; run disconnect test.
                    runDisconnect = 1;
                    continue;
#endif
                  case 'U':     // -U; run unsolicit event test.
                    runUnsolicitEvent = 1;
                    continue;
                  case 's':     // -s; run session test.
                    runSession = 1;
                    continue;
                  case 't':     // -t; run message store test.
                    runMessgStore = 1;
                    continue;
                  case 'm':     // -m; run message test.
                    runMessg = 1;
                    continue;
                  case 'e':     // -e; run message envelope test.
                    runMessgEnv = 1;
                    continue;
                  case 'b':     // -b; run message body test.
                    runMessgBody = 1;
                    continue;
                  case 'B':     // -B; run basic test.
                    runBasic = 1;
                    continue;
                  case 'u':     // -u; run utility test.
                    runUtils = 1;
                    continue;
                  case 'S':     // -S; run session test.
                    runSearch = 1;
                    continue;
                  case 'h':     // -h; print Usage.
                    Usage();
                    exit(0);
                  case 'C':     // -C; run compose test.
                    runCompose = 1;
                    continue;
                  case 'o':     // -C; run compose test.
                    runOutgoingMessg = 1;
                    continue;
                 default:
                    fprintf(stderr, "%s: Unknown option -%c\n", "sdmtest", pargv[i1][j]);
                    Usage();
                    exit(-1);
                  }     // switch
 
              } // for j
          } // else
      } // of FOR
}


static 
void Usage()
{
  fprintf(stderr,"Usage:\tsdmtest -[hpacdDstmebBCuUSo]\n");
  fprintf(stderr,"\t\t -h \t print usage information\n");
  fprintf(stderr,"\t\t -p \t print object sizes\n");
  fprintf(stderr,"\t\t -a \t run asynchrounous calls test\n");
  fprintf(stderr,"\t\t -c \t run connection test\n");
  fprintf(stderr,"\t\t -d \t run datatypes test\n");
#ifdef INCLUDE_DISCONNECT
  fprintf(stderr,"\t\t -D \t run disconnect test\n");
#endif
  fprintf(stderr,"\t\t -s \t run session test\n");
  fprintf(stderr,"\t\t -t \t run message store test\n");
  fprintf(stderr,"\t\t -m \t run message test\n");
  fprintf(stderr,"\t\t -e \t run message envelope test\n");
  fprintf(stderr,"\t\t -b \t run message body test\n");
  fprintf(stderr,"\t\t -B \t run basic test\n");
  fprintf(stderr,"\t\t -C \t run compose test\n");
  fprintf(stderr,"\t\t -u \t run utility test\n");
  fprintf(stderr,"\t\t -U \t run unsolicit event test\n");
  fprintf(stderr,"\t\t -S \t run search test\n");
  fprintf(stderr,"\t\t -o \t run outgoing message test\n");
}





