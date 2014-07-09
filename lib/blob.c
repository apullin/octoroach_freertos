#include <xc.h>
#include <stdlib.h>
#include "blob.h"


Blob_t blobCreate(unsigned int length){
    Blob_t newBlob;
    newBlob.data = (unsigned char*) malloc(length);
    if(newBlob.data == NULL){
        newBlob.length = 0;
    }
    else{
        newBlob.length = length;
    }

    return newBlob;
}

void blobDestroy(Blob_t blob){
    if(blob.data != NULL){
        free(blob.data);
    }
}