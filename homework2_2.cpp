#include <stdio.h>
#include <limits.h>
#include "MyAssert.h"

const int cntTeams = 10;
const int MaxLen = 10000;
const int ErrorReadValue = INT_MIN;

void swap(int* op1, int* op2);
void SetScore(int* ptrMatrix, int teamId1, int teamId2, int score);
int GetScore(int* ptrMatrix, int teamId1, int teamId2);  // Проверяй индексы перед обращением
int SumAriphmProgression(int x);

int main() {
    int scoreMatrix[MaxLen] = {};

    SetScore(scoreMatrix, 0, 2, 13);
    GetScore(scoreMatrix, 100, 0);

    
    printf("%i\n", GetScore(scoreMatrix, 0, 1));
    
    return 0;
}

void swap(int* op1, int* op2) {
    int temp = *op2;
    
    *op2 = *op1;
    *op1 = temp;
}

void SetScore(int* ptrMatrix, int teamId1, int teamId2, int score) {
    // 0 index function
    //  teamId1 = x, teamId2 = y, y > x
    if (teamId1 == teamId2) {
        printf("The team cannot play with itself!\n");
        return;
    }

    if (teamId2 < teamId1) {
        swap(&teamId1, &teamId2);
    }

    int offset = SumAriphmProgression(teamId2 - 1) + teamId1;
    
    if (offset >= MaxLen) {
        printf("%s:%i %s(): No enough memory (%zu bytes)!\n", __FILE__, __LINE__, __FUNCTION__, offset);
        return;
    }
    
    *(ptrMatrix + offset) = score;
}

int GetScore(int* ptrMatrix, int teamId1, int teamId2) {
 
    ASSERT (INDEX_OK (teamId1, cntTeams), " Incorrect team number!", ErrorReadValue);
    ASSERT (INDEX_OK (teamId2, cntTeams), " Incorrect team number!", ErrorReadValue);

    if (teamId1 == teamId2) {
        printf("The team cannot play with itself!\n");
        return ErrorReadValue;
    }

    if (teamId2 < teamId1) {
        swap(&teamId1, &teamId2);
    }

    int offset = SumAriphmProgression(teamId2 - 1) + teamId1;
    
    if (offset >= MaxLen) {
        printf("No enough memory!\n");
        return ErrorReadValue;
    }

    return *(ptrMatrix + offset);
}

int SumAriphmProgression(int n) {
    return (n + 1) * n / 2;
}
