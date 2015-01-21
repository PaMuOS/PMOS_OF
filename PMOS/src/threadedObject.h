#pragma once


#include "ofThread.h"


/// This is a simple example of a ThreadedObject created by extending ofThread.
/// It contains data (count) that will be accessed from within and outside the
/// thread and demonstrates several of the data protection mechanisms (aka
/// mutexes).
class ThreadedObject: public ofThread
{
public:
    /// Create a ThreadedObject and initialize the member
    /// variable in an initialization list.
    ThreadedObject(): count(0)
    {
    }
    bool bServerConnected;
    /// Start the thread.
    void start()
    {
        // Mutex blocking is set to true by default
        // It is rare that one would want to use startThread(false).
        startThread();
    }

    /// Signal the thread to stop.  After calling this method,
    /// isThreadRunning() will return false and the while loop will stop
    /// next time it has the chance to.
    void stop()
    {
        stopThread();
    }

    
    /// Our implementation of threadedFunction.
    void threadedFunction(){

            while(isThreadRunning()) {
                if(lock()){
               
                    string pingStr = (string)"ping -c 1 -t 1 " + "pmos.fi" ;
                    int flag = system( pingStr.c_str());
                    
                    if(flag == 0){
                        bServerConnected = true;
                    }else{
                        bServerConnected = false;
                        
                    }
                    //cout << bServerConnected << endl;
                
                    unlock();

                    sleep(100);
                }else
                {
                    // If we reach this else statement, it means that we could not
                    // lock our mutex, and so we do not need to call unlock().
                    // Calling unlock without locking will lead to problems.
                    ofLogWarning("threadedFunction()") << "Unable to lock mutex.";
                }
                
            }
            // done
    }
    
    int getCount()
    {
        ofScopedLock lock(mutex);
        return count;
    }

protected:
    // This is a simple variable that we aim to always access from both the
    // main thread AND this threaded object.  Therefore, we need to protect it
    // with the mutex.  In the case of simple numerical variables, some
    // garuntee thread safety for small integral types, but for the sake of
    // illustration, we use an int.  This int could represent ANY complex data
    // type that needs to be protected.
    int count;

};
