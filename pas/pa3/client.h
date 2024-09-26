#ifndef _CLIENT_H_
#define _CLIENT_H_


#include <thread>

#include "BoundedBuffer.h"
#include "HistogramCollection.h"
#include "FIFORequestChannel.h"


/**
 * @todo    Test get new channel Function
 * 
 * @brief   Function that creates new communication channels
 * @param   p_channel   Pointer to a communication channel
 * @param   __bufcap    Maximum buffer capacity, specified at run time
 * 
 * @return  Pointer to a new FIFORequestChannel
 */
FIFORequestChannel* get_new_channel ( FIFORequestChannel* p_channel, int __bufcap )
{
    MESSAGE_TYPE newChannelMSG = NEWCHANNEL_MSG;
    p_channel->cwrite(&newChannelMSG, sizeof(MESSAGE_TYPE));
    char* buffer = new char[__bufcap];
    p_channel->cread(buffer, __bufcap);
    p_channel = new FIFORequestChannel{buffer, FIFORequestChannel::CLIENT_SIDE};
    delete[] buffer;
    return p_channel;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// Patient Thread //////////////////////////////////////////////////

/**
 * @todo    Test Patient Thread Function
 * 
 * @brief   Functionality of Patient threads
 * @param   p_reqbuf    Bounded buffer containing requests
 * @param   __patient   Unique ID of the patient to pull data for
 * @param   __n_reqs    Number of data points to request
 */
void patient_thread_function ( BoundedBuffer* p_reqbuf, int __patient, int __n_reqs );


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////// File Thread /////////////////////////////////////////////////////

/**
 * @todo    Test File Thread Helper
 * 
 * @brief   Helper function to deal with File Requests
 * 
 * @note    Default behavior treats p_dest as a BoundedBuffer pointer, but if both the offset and the length are zero, it treats p_dest as a FIFORequestChannel pointer and cwrites the data
 * 
 * @param   p_dest      Pointer the the desired destination of the data
 * @param   __fname     Name of file to request data from
 * @param   __offset    Where in the file to start reading data
 * @param   __length    Amount of data to read (bytes)
 * 
 * @note    Create filemsg objects
 *          Copy the message to a char* `buffer` and append the filename to that buffer 
 *          Push them to the desired location (BoundedBuffer OR FIFORequestChannel)
 */
void file_thread_helper ( void* p_dest, std::string& __fname, int64_t __offset = 0, int __length = 0 );


/**
 * @todo    Test File Thread Function
 * 
 * @see     file_thread_helper
 * 
 * @brief   Functionality of File threads
 * @param   p_reqbuf    Bounded buffer containing requests
 * @param   __fname     Name of file to request data from
 * @param   __fsize     File size of the file to get data from
 * @param   __bufcap    Maximum buffer capacity, specified at run time
 * 
 * @note    Open output file, allocate the memory of the file with fseek
 *          close the file
 *          for each section of the file, call the helper function
 *          if there is extra data left in the file, call the helper 1 last time
 */
void file_thread_function ( BoundedBuffer* p_reqbuf, std::string __fname, int64_t __fsize, int __bufcap );

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////// Worker Thread ///////////////////////////////////////////////////

/**
 * @todo    Test Worker Thread Data Helper
 * 
 * @brief   Worker thread Helper that creates data responses
 * 
 * @param   __d_msg     Data message to process
 * @param   p_resbuf    Pointer to a BoundedBuffer containing responses
 * @param   p_channel   Pointer to a communication channel
 * 
 * @note    send the request accross the FIFORequestChannel
 *          collect respose from channel
 *          create std::pair of {patient_no, server's response}
 *          push that pair to the response
 */
void worker_thread_data_helper ( char* __d_msg, BoundedBuffer* p_resbuf, FIFORequestChannel* p_channel );

/**
 * @todo    Test Worker Thread File Helper
 * 
 * @brief   Worker thread Helper that creates file responses
 * 
 * @param   __f_msg     File message to process
 * @param   p_channel   Pointer to a communication channel
 * @param   __bufcap    Maximum buffer capacity, specified at run time
 */
void worker_thread_file_helper ( char* __f_msg, FIFORequestChannel* p_channel, int __bufcap );

/**
 * @todo    Test Worker Thread Function
 * 
 * @brief   Functionality of Worker threads
 * 
 * @see     worker_thread_data_helper
 * @see     worker_thread_file_helper
 * 
 * @param   p_reqbuf    Pointer to a BoundedBuffer containing requests
 * @param   p_resbuf    Pointer to a BoundedBuffer containing responses
 * @param   p_channel   Pointer to a communication channel
 * @param   __bufcap    Maximum buffer capacity, specified at run time
 * 
 * @note    infinite loop, pop message from request buffer
 *          view line 120 in the `server.cpp` for how to decide the current message
 */
void worker_thread_function ( BoundedBuffer* p_reqbuf, BoundedBuffer* p_resbuf, FIFORequestChannel* p_channel, int __bufcap );

class worker
{
private:
    std::thread* __t;
public:
    worker( BoundedBuffer* p_reqbuf, BoundedBuffer* p_resbuf, FIFORequestChannel* p_ctrl_chan, int __bufcap )
    {
        FIFORequestChannel* p_private_chan = get_new_channel(p_ctrl_chan, __bufcap);
        __t = new std::thread{worker_thread_function, p_reqbuf, p_resbuf, p_private_chan, __bufcap};
    }
    ~worker()
    {
        __t->join();
        delete __t;
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////// Histogram Thread /////////////////////////////////////////////////

/**
 * @todo    Test Histogram Thread Function
 * 
 * @brief   Functionality of Histogram threads
 * 
 * 
 * @param   p_resbuf    Pointer to a BoundedBuffer containing responses
 * @param   p_histo_coll Pointer to a HistogramCollection containing responses
 * @param   __bufcap    Maximum buffer capacity, specified at run time
 * 
 * @note    infinite loop, pop from response buffer
 *          call histogram::update(response->p_no, response->double)
 */
void histogram_thread_function ( BoundedBuffer* p_resbuf, HistogramCollection* p_histo_coll, int __bufcap );

#endif