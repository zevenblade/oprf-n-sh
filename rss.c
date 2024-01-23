#include <string.h>
#include <stdlib.h>

#include "rss.h"
#include "arith.h"
#include "fips202.h"

unsigned long long factorial(int num)
{
    if (num == 0 || num == 1)
    {
        return 1;
    }
    else
    {
        return num * factorial(num - 1);
    }
}

unsigned long long binomialCoefficient(int n, int k)
{
    if (k < 0 || k > n)
    {
        return 0; // Invalid input
    }

    return factorial(n) / (factorial(k) * factorial(n - k));
}

void generateTuples(int set[], int tuple[], int tuples[][T], int n, int t, int index, int tupleIndex, int *totalTuples)
{
    if (tupleIndex == t)
    {
        for (int i = 0; i < t; i++)
        {
            tuples[*totalTuples][i] = tuple[i];
        }
        (*totalTuples)++;
        return;
    }

    for (int i = index; i < n; i++)
    {
        tuple[tupleIndex] = set[i];
        generateTuples(set, tuple, tuples, n, t, i + 1, tupleIndex + 1, totalTuples);
    }
}

void printTuples(int tuples[][2], int t, int totalTuples)
{
    for (int i = 0; i < totalTuples; i++)
    {
        printf("(");
        for (int j = 0; j < t; j++)
        {
            printf("%d", tuples[i][j]);
            if (j < t - 1)
            {
                printf(", ");
            }
        }
        printf(")\n");
    }
}

void shareDistribution(int shareDistribution[][N_SHARES], int tuples[][T])
{
    int not;

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N_SHARES; j++)
        {
            not = 0;
            for (int k = 0; k < T; k++)
            {
                if((i + 1) == tuples[j][k])
                {
                    not = 1;
                }
            }

            if (!not)
            {
                shareDistribution[i][j] = 1;
            }
            else
            {
                shareDistribution[i][j] = 0;
            }
        }
    }
}

void printShareDistribution(int shareDistr[][N_SHARES])
{
    for (int i = 0; i < N_SHARES; i++)
    {
        printf("(");
        for (int j = 0; j < N; j++)
        {
            printf("%d ", shareDistr[j][i]);
        }
        printf(")\n");
    }
}

void shareCounting(int shareCount[][N_SHARES], int shareDistribution[][N_SHARES])
{

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N_SHARES; j++)
        {
            if (shareDistribution[i][j] == 1)
            {
                shareCount[j][j] += 1;

                for (int k = j + 1; k < N_SHARES; k++)
                {
                    if (shareDistribution[i][k] == 1)
                    {
                        shareCount[j][k] += 1;
                    }
                }
            }
        }
    }

    for (int i = 1; i < N_SHARES; i++)
    {
        for (int j = 0; j < i; j++)
        {
            shareCount[i][j] = shareCount[j][i];
        }
    }
}

void printShareCounting(int shareCount[][N_SHARES])
{
    for (int i = 0; i < N_SHARES; i++)
    {
        printf("(");
        for (int j = 0; j < N_SHARES; j++)
        {
            printf("%d ", shareCount[i][j]);
        }
        printf(")\n");
    }
}

void shareMapping(int shareMap[][N_SHARES_P_SERVER], int shareDistribution[][N_SHARES])
{
    int ind;

    for (int i = 0; i < N; i++)
    {
        ind = 0;
        for (int j = 0; j < N_SHARES; j++)
        {
            if (shareDistribution[i][j] == 1)
            {
                shareMap[i][ind] = j + 1;
                ind++;
            }
        }
    }
}

void printShareMapping(int shareMap[][N_SHARES_P_SERVER])
{
    for (int i = 0; i < N; i++)
    {
        printf("(");
        for (int j = 0; j < N_SHARES_P_SERVER; j++)
        {
            printf("%d ", shareMap[i][j]);
        }
        printf(")\n");
    }
}

void generateFractions(f_elm_t fractionMatrix[][N_SHARES], int shareCount[][N_SHARES])
{
    uint64_t one = 1;
    f_elm_t tmp_mtrx[N_SHARES][N_SHARES];

    for (int i = 0; i < N_SHARES; i++)
    {
        for (int j = 0; j < N_SHARES; j++)
        {
            f_from_ui(tmp_mtrx[i][j], shareCount[i][j]);
            to_mont(tmp_mtrx[i][j], fractionMatrix[i][j]);
            f_inv(fractionMatrix[i][j]);
        }
    }
}

void secretSharing(f_elm_t shares[], f_elm_t secret, int n_shares)
{
    f_elm_t partial_res[n_shares];

    for (int i = 0; i < (n_shares - 1); i++)
    {
        f_rand(shares[i]);
    }

    f_copy(secret, partial_res[0]);

    for (int i = 0; i < (n_shares - 1); i++)
    {
        f_sub(partial_res[i], shares[i], partial_res[i + 1]);
    }

    f_copy(partial_res[n_shares - 1], shares[n_shares - 1]);
}

void printSecretSharing(f_elm_t secretShares, int n_shares)
{
    for (int i = 0; i < n_shares; i++)
    {
        printf("Share %02d ; ", i + 1);
        print_f_elm_l(secretShares[i]);
        printf("\n");
    }
    printf("\n");
}

void secretOpening(f_elm_t share[], f_elm_t secret, int n_shares)
{
    f_elm_t partial_res[n_shares - 1];

    f_add(share[0], share[1], partial_res[0]);

    for (int i = 2; i < n_shares; i++)
    {
        f_add(share[i], partial_res[i - 2], partial_res[i - 1]);
    }

    f_copy(partial_res[n_shares - 2], secret);
}

void secretSharingServers(f_elm_t sharesServer[][N_SHARES_P_SERVER], f_elm_t shares[],
                          int shareMap[][N_SHARES_P_SERVER])
{
    int ind;

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N_SHARES_P_SERVER; j++)
        {
            ind = shareMap[i][j];
            f_copy(shares[ind - 1], sharesServer[i][j]);
        }
    }
}

void printSecretSharingServers(f_elm_t sharesServer[][N_SHARES_P_SERVER])
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N_SHARES_P_SERVER; j++)
        {
            print_f_elm_l(sharesServer[i][j]);
        }
        printf("\n");
    }
}

void keySharingServersN(f_elm_t sharesServer[][N_BITS][N_SHARES_P_SERVER], f_elm_t keys[],
                        int shareMap[][N_SHARES_P_SERVER])
{
    int ind;
    f_elm_t shares[N_SHARES];

    for (int r = 0; r < N_BITS; r++)
    {
        secretSharing(shares, keys[r], N_SHARES);
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N_SHARES_P_SERVER; j++)
            {
                ind = shareMap[i][j];
                f_copy(shares[ind - 1], sharesServer[i][r][j]);
            }
        }
    }
}

void shareMultMaskN(f_elm_t multMaskServ[][N_BITS][N_SHARES_P_SERVER * N_SHARES_P_SERVER],
                    int shareDistr[][N_SHARES])
{
    int servInd[N];

    uint64_t zero_ui = 0;
    f_elm_t zero;
    f_from_ui(zero, zero_ui);

    f_elm_t multMask[N_SHARES * N_SHARES];

    for (int r = 0; r < N_BITS; r++)
    {
        memset(servInd, 0, sizeof(servInd));

        secretSharing(multMask, zero, N_SHARES * N_SHARES);

        for (int i = 0; i < N_SHARES; i++)
        {
            for (int j = 0; j < N_SHARES; j++)
            {
                for (int k = 0; k < N; k++)
                {
                    if ((shareDistr[k][i] == 1) && (shareDistr[k][j] == 1))
                    {
                        f_copy(multMask[i * N_SHARES + j], multMaskServ[k][r][servInd[k]]);
                        servInd[k] += 1;
                    }
                }
            }
        }
    }
}

void shareElmMaskN(f_elm_t elmMaskServ[][N_BITS][N_SHARES_P_SERVER * N_SHARES_P_SERVER],
                   int shareDistr[][N_SHARES], int shareCount[][N_SHARES])
{
    int servInd[N];

    uint64_t zero_ui = 0;
    f_elm_t zero;
    f_from_ui(zero, zero_ui);

    int n_row_shares;
    int indRowShare;

    for (int r = 0; r < N_BITS; r++)
    {
        memset(servInd, 0, sizeof(servInd));

        for (int i = 0; i < N_SHARES; i++)
        {
            for (int j = 0; j < N_SHARES; j++)
            {
                n_row_shares = shareCount[i][j];
                f_elm_t rowShares[n_row_shares];
                secretSharing(rowShares, zero, n_row_shares);

                indRowShare = 0;
                for (int k = 0; k < N; k++)
                {
                    if ((shareDistr[k][i] == 1) && (shareDistr[k][j] == 1))
                    {
                        f_copy(rowShares[indRowShare], elmMaskServ[k][r][servInd[k]]);
                        servInd[k] += 1;
                        indRowShare += 1;
                    }
                }
            }
        }
    }
}

void aggregateResN(f_elm_t resServ[][N_BITS][N_SHARES_P_SERVER * N_SHARES_P_SERVER],
                   f_elm_t expAggrRes[][N_SHARES * N_SHARES], int shareDistr[][N_SHARES])
{
    int servInd[N];
    memset(servInd, 0, sizeof(servInd));

    uint64_t zero_ui = 0;
    f_elm_t zero;
    f_from_ui(zero, zero_ui);

    f_elm_t sum;

    for (int r = 0; r < N_BITS; r++)
    {
        for (int i = 0; i < (N_SHARES * N_SHARES); i++)
        {
            f_copy(zero, expAggrRes[r][i]);
        }
    }

    for (int i = 0; i < N_SHARES; i++)
    {
        for (int j = 0; j < N_SHARES; j++)
        {
            for (int k = 0; k < N; k++)
            {
                if ((shareDistr[k][i] == 1) && (shareDistr[k][j] == 1))
                {
                    for (int r = 0; r < N_BITS; r++)
                    {
                        f_add(resServ[k][r][servInd[k]], expAggrRes[r][i * N_SHARES + j], sum);
                        f_copy(sum, expAggrRes[r][i * N_SHARES + j]);
                    }
                    servInd[k] += 1;
                }
            }
        }
    }
}

void aggregateVectorN(f_elm_t expAggrRes[][N_SHARES * N_SHARES], f_elm_t res[])
{
    f_elm_t sum_1, sum_2;

    uint64_t zero_ui = 0;
    f_elm_t zero;
    f_from_ui(zero, zero_ui);

    for (int r = 0; r < N_BITS; r++)
    {
        f_copy(zero, sum_1);
        for (int i = 0; i < (N_SHARES * N_SHARES); i++)
        {
            f_add(expAggrRes[r][i], sum_1, sum_2);
            f_copy(sum_2, sum_1);
        }
        f_copy(sum_2, res[r]);
    }
}

void aggregateRowsN(f_elm_t resSharesServ[][N_BITS], f_elm_t res[])
{
    f_elm_t sum_1, sum_2;

    uint64_t zero_ui = 0;
    f_elm_t zero;
    f_from_ui(zero, zero_ui);

    for (int r = 0; r < N_BITS; r++)
    {
        f_copy(zero, sum_1);
        for (int i = 0; i < N; i++)
        {
            f_add(resSharesServ[i][r], sum_1, sum_2);
            f_copy(sum_2, sum_1);
        }
        f_copy(sum_2, res[r]);
    }
}

void mergeHashesN(f_elm_t hashes[][N_BITS][N_SHARES_P_SERVER * N_SHARES_P_SERVER],
                  f_elm_t mergedHashes[][N_SHARES * N_SHARES], int shareDistr[][N_SHARES])
{
    int servInd[N];
    memset(servInd, 0, sizeof(servInd));

    for (int i = 0; i < N_SHARES; i++)
    {
        for (int j = 0; j < N_SHARES; j++)
        {
            for (int k = 0; k < N; k++)
            {
                if ((shareDistr[k][i] == 1) && (shareDistr[k][j] == 1))
                {
                    for (int r = 0; r < N_BITS; r++)
                    {
                        f_copy(hashes[k][r][servInd[k]], mergedHashes[r][i * N_SHARES + j]);
                    }
                    servInd[k] += 1;
                }
            }
        }
    }
}

void compareHashesN(f_elm_t expAggrRes[][N_SHARES * N_SHARES], f_elm_t mergedHashes[][N_SHARES * N_SHARES],
                    int checkHash[][N_SHARES * N_SHARES])
{
    f_elm_t curr_hash;
    char *ptr1, *ptr2;
    int equal;

    for (int r = 0; r < N_BITS; r++)
    {
        for (int i = 0; i < (N_SHARES * N_SHARES); i++)
        {
            shake128((unsigned char *)curr_hash, WORDS_FIELD * 8,
                     (unsigned char *)expAggrRes[r][i], WORDS_FIELD * 8);

            equal = 1;
            ptr1 = (unsigned char *)curr_hash;
            ptr2 = (unsigned char *)mergedHashes[r][i];
            for (int j = 0; j < 8; ++j)
            {
                if (ptr1[j] != ptr2[j])
                {
                    equal = 0; // Set to 0 if elements are not equal
                    break;     // No need to continue checking
                }
            }
            checkHash[r][i] = equal;
        }
    }
}

void checkHashesN(int checkHash[][N_SHARES * N_SHARES])
{
    int hashesPass = 1;

    for (int r = 0; r < N_BITS; r++)
    {
        for (int i = 0; i < (N_SHARES * N_SHARES); i++)
        {
            if (checkHash[r][i] == 0)
            {
                hashesPass = 0;
            }
        }
    }

    if (hashesPass == 1)
    {
        printf("All hashes pass!\n");
    }
    else
    {
        printf("Error in hashes\n");
    }
}

void printResN(f_elm_t res[])
{
    for (int i = 0; i < N_BITS; i++)
    {
        printf("%03d: ", i + 1);
        print_f_elm_l(res[i]);
        printf("\n");
    }
}

void calculateOPRF(f_elm_t resElm[], unsigned char *resOPRF)
{
    f_elm_t resElm_mont;

    for (int i = 0; i < N_BITS; i++)
    {
        to_mont(resElm[i], resElm_mont);
        f_leg(resElm_mont, resOPRF + i);
    }
}

