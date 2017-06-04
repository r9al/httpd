#include <threadmgr.h>
#include <httphandlers.h>
#include <prerror.h>
#include <tools.h>
#include <pass.h>
#include <defines.h>
#include <session.h>


int MAX_THREADS = 3;

CmdPipe::CmdPipe()
{
	cmdQueue = new CmdQueue();
#ifndef USE_PTHREAD
	lock     = PR_NewLock();
	if( !lock)
		fprintf(stderr, "ERRR: Unable to create Locks \n");
#else
    pthread_mutex_init( &lock, NULL );
#endif
	usage    = 0;
}

CmdPipe::~CmdPipe()
{
	delete cmdQueue;
#ifndef USE_PTHREAD
	PR_DestroyLock( lock );
#else
    pthread_mutex_destroy( &lock);
#endif
}

void CmdPipe::pushCmd( Connection *conn)
{
	if( !conn )
		return;
#ifndef USE_PTHREAD
	PR_Lock  (lock);
#else
    pthread_mutex_lock( &lock );
#endif
	//printf("INFO: Pushing Connection Data\n");
	cmdQueue->push( conn );
	usage++;

#ifndef USE_PTHREAD
	PR_Unlock(lock);
#else
    pthread_mutex_unlock( &lock );
#endif
}

Connection* CmdPipe::popCmd ()
{
	Connection* conn = NULL;
#ifndef USE_PTHREAD
	PR_Lock  (lock);
#else
    pthread_mutex_lock( &lock );
#endif
	if( cmdQueue->size() > 0 && ((conn = cmdQueue->front()) != NULL) )
	{
        //printf("INFO: Poping Connection Data Address:0x%x, %d\n",conn, cmdQueue->size());
		cmdQueue->pop();
	}
#ifndef USE_PTHREAD
	PR_Unlock(lock);
#else
    pthread_mutex_unlock( &lock );
#endif
	return conn;
}

void CmdPipe::reduceUsage()
{
#ifndef USE_PTHREAD
	PR_Lock  (lock);
#else
    pthread_mutex_lock( &lock );
#endif
    usage--;
#ifndef USE_PTHREAD
	PR_Unlock(lock);
#else
    pthread_mutex_unlock( &lock );
#endif
}

int CmdPipe::getUsage ()
{
#ifndef USE_PTHREAD
	PR_Lock  (lock);
#else
    pthread_mutex_lock( &lock );
#endif
	int temp = usage;
#ifndef USE_PTHREAD
	PR_Unlock(lock);
#else
    pthread_mutex_unlock( &lock );
#endif
	return temp;
}

ThreadMgr *ThreadMgr::tMgr=0;

ThreadMgr::ThreadMgr()
{
	int i ;
    cIndex = 0;
	for( i=0; i<MAX_THREADS; i++ )
	{
		cmdPipe[i] = new CmdPipe();
		index[i]   = i;
		db[i] = NULL;
		//int rc = sqlite3_open(INFO_STORE, &db[i], getEncryptionStructure(ENC_KEY_DATABASE) );
		int rc = sqlite3_open(INFO_STORE, &db[i] );
		if( rc )
		{
			fprintf(stderr, "ERRR: Unable to open db, Corrupted ? \n");
       		sqlite3_close(db[i]);
			db[i] = NULL;
		}
	}
}

ThreadMgr::~ThreadMgr()
{
}

ThreadMgr* ThreadMgr::createInstance()
{
	if(! tMgr )
	{
		tMgr = new ThreadMgr();
	}
	return tMgr;
}

void ThreadMgr::startThreads()
{
	int i;
	for( i=0; i<MAX_THREADS; i++ )
	{
#ifndef USE_PTHREAD
		tdata[i]   = PR_CreateThread(PR_SYSTEM_THREAD, ThreadMgr::thread, &index[i],
			PR_PRIORITY_NORMAL, PR_GLOBAL_THREAD, PR_JOINABLE_THREAD, 8388608 );
		if( !tdata[i] )
		{
			fprintf(stderr, "ERRR: Unable to create thread \n");
		}
		else
		{
			fprintf(stderr, "INFO: Created Thread with Index=%d \n",i);
		}
#else
        int rc = pthread_create(&tdata[i], NULL, ThreadMgr::thread, &index[i] );
        if ( rc != 0 ){
            fprintf(stderr, "ERRR: Unable to create thread \n");
        } else {
            fprintf(stderr, "INFO: Created Thread with Index=%d \n",i);
        }
#endif
	}
}

void ThreadMgr::stopThreads()
{
	fprintf( stderr, "INFO: Stopping all threads \n");
	int i;
	for(i=0; i<MAX_THREADS; i++ )
	{
		Connection *conn = new Connection();
		conn->cmd = THREAD_CMD_EXIT;
		cmdPipe[i]->pushCmd(conn);
#ifndef USE_PTHREAD
		PR_JoinThread( tdata[i] );
#else
		pthread_join( tdata[i], NULL);
#endif
		fprintf( stderr, "DBUG: Stopped thread %d \n", i);
#ifndef USE_PTHREAD
		tdata[i] = NULL;
#else
        tdata[i] = 0;
#endif
	}
}

#ifndef USE_PTHREAD
void ThreadMgr::thread(void *data)
#else
void* ThreadMgr::thread(void *data)
#endif
{
	int* index = (int*)data;
	//ThreadMgr   *mgr  = ThreadMgr::createInstance();
	ThreadMgr   *mgr  = ThreadMgr::tMgr;
	Connection  *conn = NULL;
	HttpHandler *hHdlr = HttpHandler::createInstance();
	int cmd   = THREAD_CMD_SLEEP;
    int aLvl  = 1;
	//printf("In thread %d \n",*index);
	sqlite3 *db  = NULL;
	do
	{
        conn = NULL;
		if ( (conn = mgr->cmdPipe[*index]->popCmd()) == NULL || conn == (Connection *)0xFFFFFFFF)
		{
#ifndef USE_PTHREAD		
			PR_Sleep(10);
#else

        struct timespec req, resp;
        req.tv_sec = 0;
        req.tv_nsec = 1000000;
        nanosleep( &req, &resp );
#endif
			//printf("Thread %d Sleeping \n",*index);
			continue;
		}
		else
		{
			//Process the request here
  	        fprintf(stderr, "DBUG: Thread %d received task %llu\n", *index, (unsigned long long int)conn);
			cmd = conn->cmd;
			if( cmd == THREAD_CMD_PTASK )
			{
                aLvl = atoi(conn->sess->getVariable("auth"));
                if( aLvl == 0 )
                    aLvl = 1;
				PluginHandler *temp = (PluginHandler *)hHdlr->getHandler(conn->req.getReqFile());
				HttpResp *tempResp   = &conn->resp;
				tempResp->setAddOn (1);
				if( temp && ( aLvl >= (int)temp->authLvlReq) )
				{
					db   = mgr->db[*index];
					if( !db )
   		 			{
						tempResp->setContentLen( 0 );
						sendConnRespHdr ( conn, HTTP_RESP_INTERNALERR );
						PR_Close(conn->socket);
						conn->socket = NULL;
						delete  conn;
						continue;
			  		}
					else
					{
						tempResp->setContentLen( -1 );
						tempResp->setStatus(HTTP_RESP_OK);
						conn->db    = db;
						conn->udata = NULL;
						temp->processReq(conn);
						PR_Close(conn->socket);
						conn->socket = NULL;
						delete (conn);
					}
				}
				else
				{
					fprintf(stderr, "ERRR: Unable to find handler: %s \n",conn->req.getReqFile());
					tempResp->setContentLen( 0 );
					sendConnRespHdr ( conn, HTTP_RESP_NOTFOUND );
					PR_Close(conn->socket);
					conn->socket = NULL;
					delete (conn);
				}
			}
            mgr->cmdPipe[*index]->reduceUsage();
		}
	}
	while ( cmd != THREAD_CMD_EXIT );

	if( cmd == THREAD_CMD_EXIT && conn)
	{
		delete conn;
	}
#ifdef USE_PTHREAD
    return 0;
#endif // USE_PTHREAD
}

void ThreadMgr::assignTask (Connection *conn)
{
	int temp, i;
	int least = 32768;
	int leastIndex = 0;
	if(!conn)
		return;
	for(i=0; i<MAX_THREADS; i++)
	{
		temp = cmdPipe[i]->getUsage();
        //printf("Thread %d Usage=%d \n",i,temp);
		if( temp < least )
		{
			least = temp;
			leastIndex = i;
		}
	}
    //leastIndex = cIndex;
    //cIndex=(cIndex+1)%MAX_THREADS;
    //printf("Assigning Task to Thread=%d \n",leastIndex);
	cmdPipe[leastIndex]->pushCmd(conn);
}
