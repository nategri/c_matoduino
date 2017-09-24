// Compile with:
// gcc -I. -o c_matoduino main.c muscles.c neural_rom.c

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

#include "defines.h"
#include "neural_rom.h"
#include "muscles.h"

//
// Globals
//

uint16_t N_MAX;

int32_t LeftTotal;
int32_t RightTotal;

uint8_t MotorNeuronASum;
uint8_t MotorNeuronBSum;

uint8_t MotorBState[N_MOTORB];
uint8_t MotorAState[N_MOTORA];

//
// Structs
//

// Struct for representing a neuron connection
struct NeuralConnection {
  uint16_t id;
  int8_t weight;
};

//
// Three sets of neural state arrays
//

// One set for neurons that are connected to others
int8_t CurrConnectedState[N_NTOTAL];
int8_t NextConnectedState[N_NTOTAL];

// Another set for muscles that aren't connected to other cells
int16_t* CurrMuscleState;
int16_t* NextMuscleState;

// Final set to track how many cycles a neuron has been idle
uint8_t* IdleCycles;

//
// Functions for getting and setting these states
//

void StatesInit() {
  // Total number of connected neurons (first word in ROM)
  N_MAX = (uint16_t)NeuralROM[0];

  CurrMuscleState = malloc((N_NTOTAL-N_MAX)*sizeof(int16_t));
  NextMuscleState = malloc((N_NTOTAL-N_MAX)*sizeof(int16_t));
  IdleCycles = malloc(N_MAX*sizeof(uint8_t));

  memset(CurrConnectedState, 0, sizeof(CurrConnectedState));
  memset(NextConnectedState, 0, sizeof(NextConnectedState));
  memset(CurrMuscleState, 0, (N_NTOTAL-N_MAX)*sizeof(CurrMuscleState[0]));
  memset(NextMuscleState, 0, (N_NTOTAL-N_MAX)*sizeof(NextMuscleState[0]));

  memset(IdleCycles, 0, N_MAX*sizeof(NextConnectedState[0]));
}

void SetCurrState(uint16_t N_ID, int8_t val) {
  if(N_ID < N_MAX) {
    CurrConnectedState[N_ID] = val;
  }
  else {
    CurrMuscleState[N_ID-N_MAX] = val;
  }
}

int16_t GetCurrState(uint16_t N_ID) {
  if(N_ID < N_MAX) {
    return CurrConnectedState[N_ID];
  }
  else {
    return CurrMuscleState[N_ID-N_MAX];
  }
}

void SetNextState(uint16_t N_ID, int16_t val) {
  if(N_ID < N_MAX) {
    if(val > N_THRESHOLD) {
      NextConnectedState[N_ID] = N_THRESHOLD+1;
    }
    else if(val < (-1*N_THRESHOLD)) {
      NextConnectedState[N_ID] = -1*N_THRESHOLD-1;
    }
    else {
      NextConnectedState[N_ID] = val;
    }
    
  }
  else {
    NextMuscleState[N_ID-N_MAX] = val;
  }
}

int16_t GetNextState(uint16_t N_ID) {
  if(N_ID < N_MAX) {
    return NextConnectedState[N_ID];
  }
  else {
    return NextMuscleState[N_ID-N_MAX];
  }
}

void AddToNextState(uint16_t N_ID, int8_t val) {
  int16_t currVal = GetNextState(N_ID);
  SetNextState(N_ID, currVal + val);
}

// Copy 'next' state into 'current' state
void CopyStates() {
  memcpy(CurrConnectedState, NextConnectedState, sizeof(NextConnectedState));
  memcpy(CurrMuscleState, NextMuscleState, (N_NTOTAL-N_MAX)*sizeof(NextMuscleState[0]));
}

//
// Functions for handling connectome simulation
//

// Parse a word of the ROM into a neuron id and connection weight
struct NeuralConnection ParseROM(uint16_t romWord) {
  uint8_t* romByte;
  romByte = (uint8_t*)&romWord;

  uint16_t id = romByte[1] + ((romByte[0] & 0b10000000) << 1);
  
  uint8_t weightBits = romByte[0] & 0b01111111;
  weightBits = weightBits + ((weightBits & 0b01000000) << 1);
  int8_t weight = (int8_t)weightBits;

  struct NeuralConnection neuralConn = {id, weight};

  return neuralConn;
}

// Propagate each neuron connection weight into the next state
void PingNeuron(uint16_t N_ID) {
  uint16_t address = NeuralROM[N_ID+1];
  uint16_t len = NeuralROM[N_ID+1+1] - NeuralROM[N_ID+1];
  
  for(int i = 0; i<len; i++) {
    struct NeuralConnection neuralConn = ParseROM(NeuralROM[address+i]);

    AddToNextState(neuralConn.id, neuralConn.weight);
  }
}

void DischargeNeuron(uint16_t N_ID) {
  PingNeuron(N_ID);
  SetNextState(N_ID, 0);
}

// Flush neurons that have been idle for a while
void HandleIdleNeurons() {
  for(int i = 0; i < N_MAX; i++) {
    if(GetNextState(i) == GetCurrState(i)) {
      IdleCycles[i] += 1;
    }
    if(IdleCycles[i] > 10) {
      SetNextState(i, 0);
      IdleCycles[i] = 0;
    }
  }
}

//
// Function for determinining how muscle weights map to motors
//

void ActivateMuscles() {
  LeftTotal = 0;
  RightTotal = 0;

  // Gather totals on left and right side muscles
  for(int i = 0; i < N_NMUSCLES; i++) {
    uint16_t leftId = LeftBodyMuscles[i];
    uint16_t rightId = RightBodyMuscles[i]; 

    LeftTotal += GetNextState(leftId);
    RightTotal += GetNextState(rightId);

    // Temoporarily moved to print muscles func
    //SetNextState(leftId, 0.0);
    //SetNextState(rightId, 0.0);
  }

  MotorNeuronASum = 0;
  MotorNeuronBSum = 0;

  for(int i = 0; i < N_MOTORB; i++) {
    uint8_t motorBId = MotorNeuronsB[i];
    if(GetCurrState(motorBId) > N_THRESHOLD) {
      MotorNeuronBSum += 1;
      MotorBState[i] = 1;
    }
    else {
      MotorBState[i] = 0;
    }
  }

  for(int i = 0; i < N_MOTORA; i++) {
    uint8_t motorAId = MotorNeuronsA[i];
    if(GetCurrState(motorAId) > N_THRESHOLD) {
      MotorNeuronASum += 1;
      MotorAState[i] = 1;
    }
    else {
      MotorAState[i] = 0;
    }
  }
}

// Complete one cycle ('tick') of the nematode neural system
void NeuralCycle() {
  for(int i = 0; i < N_MAX; i++) {
    if (GetCurrState(i) > N_THRESHOLD) {
      DischargeNeuron(i);
    }
  }

  ActivateMuscles();
  HandleIdleNeurons();
  CopyStates();
}

//
// Functions for running chemotaxis and nose touch responses
//

void RunChemotaxis() {
  // Chemotaxis neurons
  PingNeuron(N_ADFL);
  PingNeuron(N_ADFR);
  PingNeuron(N_ASGR);
  PingNeuron(N_ASGL);
  PingNeuron(N_ASIL);
  PingNeuron(N_ASIR);
  PingNeuron(N_ASJR);
  PingNeuron(N_ASJL);
  NeuralCycle();
}

void RunNoseTouch() {
  // Nose touch neurons
  PingNeuron(N_FLPR);
  PingNeuron(N_FLPL);
  PingNeuron(N_ASHL);
  PingNeuron(N_ASHR);
  //PingNeuron(N_IL1VL);
  //PingNeuron(N_IL1VR);
  PingNeuron(N_OLQDL);
  PingNeuron(N_OLQDR);
  PingNeuron(N_OLQVR);
  PingNeuron(N_OLQVL);
  NeuralCycle();
}

void PrintMuscles(FILE *fptr) {
  uint32_t sum = 0;

  for(int i = 0; i < N_NMUSCLES; i++) {
    //uint16_t leftId = LeftBodyMuscles[i];
    //uint16_t rightId = RightBodyMuscles[i]; 

    uint16_t leftId = LeftBodyMuscles[i];
    uint16_t rightId = RightBodyMuscles[i]; 

    int32_t leftVal = GetNextState(leftId);
    int32_t rightVal = GetNextState(rightId);

    sum += abs(leftVal) + abs(rightVal);

    // Only here temporarily---should be in muscles function
    SetNextState(leftId, 0.0);
    SetNextState(rightId, 0.0);
  }

  //printf("%f\n", (float)sum*255/600.0);

  // Get the weight on each muscle
  for(int i = 0; i < N_NNECKMUSCLES; i++) {
    //uint16_t leftId = LeftBodyMuscles[i];
    //uint16_t rightId = RightBodyMuscles[i]; 

    uint16_t leftId = LeftNeckMuscles[i];
    uint16_t rightId = RightNeckMuscles[i]; 

    int32_t leftVal = GetNextState(leftId);
    int32_t rightVal = GetNextState(rightId);

    fprintf(fptr, "%d %d\n", leftVal, rightVal);

    // Only here temporarily---should be in muscles function
    SetNextState(leftId, 0.0);
    SetNextState(rightId, 0.0);
  }
}

// Utility function to flush weights from muscles
void FlushMuscles() {
  for(int i = 0; i < N_NMUSCLES; i++) {
    uint16_t leftId = LeftBodyMuscles[i];
    uint16_t rightId = RightBodyMuscles[i]; 

    // Only here temporarily---should be in muscles function
    SetNextState(leftId, 0.0);
    SetNextState(rightId, 0.0);
  }

  for(int i = 0; i < N_NNECKMUSCLES; i++) {
    uint16_t leftId = LeftNeckMuscles[i];
    uint16_t rightId = RightNeckMuscles[i]; 

    // Only here temporarily---should be in muscles function
    SetNextState(leftId, 0.0);
    SetNextState(rightId, 0.0);
  }
}

int main() {
  // initialize state arrays
  StatesInit();

  // Ready a file to write to
  FILE *filePtr;
  char fileStr[100];

  FILE *filePtrCirc;
  char fileStrCirc[100];
  sprintf(fileStrCirc, "./motor_circuits/simulations.out");
  filePtrCirc = fopen(fileStrCirc, "w");

  srand(time(NULL));
  uint16_t randInt;

  
  for(int j=0; j<100; j++) {
    sprintf(fileStr, "./sim_data/sim%d.out", j);
    filePtr = fopen(fileStr, "w");


    randInt = (rand() % 10000) + 500;
  	fprintf(filePtr, "START SIM\n");
  	fprintf(filePtrCirc, "START\n");

  	printf("START\n");


    // Burn in chemotaxis
    for(int i=0; i<randInt; i++) {
      RunChemotaxis();
      if(randInt - i <= 100) {
        //fprintf(filePtr, "%d %d\n", LeftTotal, RightTotal);
        fprintf(filePtr, "TICK %d\n", i);
        PrintMuscles(filePtr);
      	fprintf(filePtrCirc, "%d %d\n", MotorNeuronBSum, MotorNeuronASum);

        // For motor a/b state
        printf("TICK\n");
        for(int k = 0; k < N_MOTORB; k++) {
          if(MotorBState[k] == 1) {
            printf("B%d\n", k+1);
          }
        }
        for(int k = 0; k < N_MOTORA; k++) {
          if(MotorAState[k] == 1) {
            printf("A%d\n", k+1);
          }
        }
      }
      else {
        FlushMuscles();
      }
    }

    fprintf(filePtr, "NOSE TOUCH\n");
    // Check nose touch response
    for(int i=0; i<100; i++) {
      fprintf(filePtr, "TICK %d\n", i);
      RunNoseTouch();
      //fprintf(filePtr, "%d %d\n", LeftTotal, RightTotal);
      PrintMuscles(filePtr);
      fprintf(filePtrCirc, "%d %d\n", MotorNeuronBSum, MotorNeuronASum);

      // For motor a/b state
      printf("TICK\n");
      for(int k = 0; k < N_MOTORB; k++) {
        if(MotorBState[k] == 1) {
          printf("B%d\n", k+1);
        }
      }
      for(int k = 0; k < N_MOTORA; k++) {
        if(MotorAState[k] == 1) {
          printf("A%d\n", k+1);
        }
      }
    }

    // Close file
    fclose(filePtr);
  }
  // Close file
  fclose(filePtrCirc);
}
