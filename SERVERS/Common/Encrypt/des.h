/* DES.H - header file for DES.C
 */

#ifndef _DES_H_
#define _DES_H_

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

/* UINT4 defines a four byte word */
typedef unsigned long int UINT4;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  UINT4 subkeys[32];                                             /* subkeys */
  UINT4 iv[2];                                       /* initializing vector */
  UINT4 originalIV[2];                        /* for restarting the context */
  int encrypt;                                               /* encrypt flag */
} DES_CBC_CTX;

typedef struct {
  UINT4 subkeys[32];                                             /* subkeys */
  UINT4 iv[2];                                       /* initializing vector */
  UINT4 inputWhitener[2];                                 /* input whitener */
  UINT4 outputWhitener[2];                               /* output whitener */
  UINT4 originalIV[2];                        /* for restarting the context */
  int encrypt;                                              /* encrypt flag */
} DESX_CBC_CTX;

typedef struct {
  UINT4 subkeys[3][32];                     /* subkeys for three operations */
  UINT4 iv[2];                                       /* initializing vector */
  UINT4 originalIV[2];                        /* for restarting the context */
  int encrypt;                                              /* encrypt flag */
} DES3_CBC_CTX;

void DES_CBCInit(DES_CBC_CTX *, unsigned char *, unsigned char *, int);
int DES_CBCUpdate(DES_CBC_CTX *, unsigned char *, unsigned char *, unsigned int);
void DES_CBCRestart(DES_CBC_CTX *);

void DESX_CBCInit(DESX_CBC_CTX *, unsigned char *, unsigned char *, int);
int DESX_CBCUpdate(DESX_CBC_CTX *, unsigned char *, unsigned char *, unsigned int);
void DESX_CBCRestart(DESX_CBC_CTX *);

void DES3_CBCInit(DES3_CBC_CTX *, unsigned char *, unsigned char *, int);
int DES3_CBCUpdate(DES3_CBC_CTX *, unsigned char *, unsigned char *, unsigned int);
void DES3_CBCRestart(DES3_CBC_CTX *);

#ifdef __cplusplus
}
#endif

#endif
