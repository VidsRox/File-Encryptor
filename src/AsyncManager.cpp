#include "AsyncManager.hpp"
#include <cstring>
#include <iostream>

void AsyncManager::add_file(std::string path){
    file_path.push_back(path);
}

void AsyncManager::process(){
    io_uring_queue_init(queue_depth, &ring, 0);//Initialize io_uring with queue depth 

    int files_submitted = 0;
    int completions_received = 0;
    int total_files = file_path.size();

    struct stat st;

    while(completions_received< total_files*2){// *2 because read + write per file

        //FIRST- fill queue with reads
        while(in_flight < queue_depth && files_submitted < total_files){
            IOContext* ctx = new IOContext();
            
        //open file
            ctx->fd = open(file_path[files_submitted].c_str(), O_RDWR);

            ctx->is_read = true;
            ctx->action = action;
            ctx->path = file_path[files_submitted];
            
            //Find file size so we know how big a buffer to allocate
            fstat(ctx->fd, &st);
            ctx->size = st.st_size;

            //allocate buffer
            ctx->buffer = new char[ctx->size + 1];
            ctx->buffer[ctx->size] = '\0';

            //Get a submission slot
            struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);

            //Fill in read operation
            io_uring_prep_read(sqe, ctx->fd, ctx->buffer, ctx->size, 0);

            // Tagged it to know which completion belongs to what
            io_uring_sqe_set_data(sqe, ctx);

            in_flight++;
            files_submitted++;
        }
        //submit to kernel outside inner loop
        io_uring_submit(&ring);

        //THEN: collect one completion
        struct io_uring_cqe* cqe;
        io_uring_wait_cqe(&ring, &cqe);

        IOContext* ctx = (IOContext*)io_uring_cqe_get_data(cqe);

        if(ctx->is_read){
            std::string transformed = cryptor.transform(std::string(ctx->buffer, ctx->size));
            memcpy(ctx->buffer, transformed.c_str(), ctx->size);
            
            //for the write we need to get a fresh SQE slot:
            struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
            io_uring_prep_write(sqe, ctx->fd, ctx->buffer, ctx->size, 0);
            io_uring_sqe_set_data(sqe, ctx);

            ctx->is_read = false;
            io_uring_submit(&ring);
            completions_received++;
        } else{
            //write done, now cleanup
            close(ctx->fd);

            delete[] ctx->buffer;
            delete ctx;
            in_flight--;
            completions_received++;
        }
        io_uring_cqe_seen(&ring, cqe);
        
    }   
    io_uring_queue_exit(&ring);
}