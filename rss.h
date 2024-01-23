#ifndef RSS_H
#define RSS_H

#include <stdio.h>
#include "arith.h"

#define N 5
#define N_SHARES 10
#define N_SHARES_P_SERVER 6

#define T 2

#define N_BITS 128
#define N_TRANSMIT 128

unsigned long long factorial(int num);

unsigned long long binomialCoefficient(int n, int k);

void generateTuples(int set[], int tuple[], int tuples[][T], int n, int t, int index, int tupleIndex, int *totalTuples);

void printTuples(int tuples[][2], int t, int totalTuples);

void shareDistribution(int shareDistribution[][N_SHARES], int tuples[][T]);

void printShareDistribution(int shareDistr[][N_SHARES]);

void shareCounting(int shareCount[][N_SHARES], int shareDistribution[][N_SHARES]);

void printShareCounting(int shareCount[][N_SHARES]);

void shareMapping(int shareMap[][N_SHARES_P_SERVER], int shareDistribution[][N_SHARES]);

void printShareMapping(int shareMap[][N_SHARES_P_SERVER]);

void generateFractions(f_elm_t fractionMatrix[][N_SHARES], int shareCount[][N_SHARES]);

void secretSharing(f_elm_t shares[], f_elm_t secret, int n_shares);

void printSecretSharing(f_elm_t secretShares, int n_shares);

void secretOpening(f_elm_t share[], f_elm_t secret, int n_shares);

void secretSharingServers(f_elm_t sharesServer[][N_SHARES_P_SERVER], f_elm_t shares[],
                          int shareMap[][N_SHARES_P_SERVER]);

void printSecretSharingServers(f_elm_t sharesServer[][N_SHARES_P_SERVER]);

void keySharingServersN(f_elm_t sharesServer[][N_BITS][N_SHARES_P_SERVER], f_elm_t keys[],
                        int shareMap[][N_SHARES_P_SERVER]);

void shareMultMaskN(f_elm_t multMaskServ[][N_BITS][N_SHARES_P_SERVER * N_SHARES_P_SERVER],
                    int shareDistr[][N_SHARES]);

void shareElmMaskN(f_elm_t elmMaskServ[][N_BITS][N_SHARES_P_SERVER * N_SHARES_P_SERVER],
                   int shareDistr[][N_SHARES], int shareCount[][N_SHARES]);

void aggregateResN(f_elm_t resServ[][N_BITS][N_SHARES_P_SERVER * N_SHARES_P_SERVER],
                   f_elm_t expAggrRes[][N_SHARES * N_SHARES], int shareDistr[][N_SHARES]);

void aggregateVectorN(f_elm_t expAggrRes[][N_SHARES * N_SHARES], f_elm_t res[]);

void aggregateRowsN(f_elm_t resSharesServ[][N_BITS], f_elm_t res[]);

void mergeHashesN(f_elm_t hashes[][N_BITS][N_SHARES_P_SERVER * N_SHARES_P_SERVER],
                  f_elm_t mergedHashes[][N_SHARES * N_SHARES], int shareDistr[][N_SHARES]);

void compareHashesN(f_elm_t expAggrRes[][N_SHARES * N_SHARES], f_elm_t mergedHashes[][N_SHARES * N_SHARES],
                    int checkHash[][N_SHARES * N_SHARES]);

void checkHashesN(int checkHash[][N_SHARES * N_SHARES]); 

void printResN(f_elm_t res[]);

void calculateOPRF(f_elm_t resElm[], unsigned char *resOPRF);

#endif