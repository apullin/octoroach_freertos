/* 
 * File:   blob.h
 * Author: pullin
 *
 * Created on April 24, 2014, 1:43 PM
 */

#ifndef BLOB_H
#define	BLOB_H

typedef struct{
    unsigned int length;
    unsigned char* data;
} Blob_t;

Blob_t blobCreate(unsigned int length);
void blobDestroy(Blob_t blob);

#endif	/* BLOB_H */

