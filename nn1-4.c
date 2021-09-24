//*****************************
/*
    Neural Network
    2 layers 
*/
//*****************************

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include "printing.h"

// *******************************************************************
// #pragma GCC optimize("O3","unroll-loops","omit-frame-pointer","inline", "unsafe-math-optimizations")
// #pragma GCC option("arch=native","tune=native","no-zero-upper")
//************************************************************

#define NL1 100//hidden Layer
#define NL2 10//Output
#define N 12 //Input
#define M 70000//Number of inputs-outputs
#define TEST 10000//test batch
#define EPOCH 1000

#define a -0.2//learning rate

//---------------------
double X[M][N];
double Y[M][NL2]={0};
double WL1[NL1][N+1];
double WL2[NL2][NL1+1];
double DL1[NL1];
double OL1[NL1];
double DL2[NL2];
double OL2[NL2];

void Initialise_X();
void Initialise_W();
void SaveWeightsToFile();


double sigmoid(double x);
void forward(const int n,const int m,double w[n][m+1],double x[m],double y[n]);

void activateNN(double *Vector);
void trainNN(double *input,double *desired);
double MSE(double *desired);

int max_index(int n,double *arr);

int main(){
    printf("Lab4 - NN\n");
    double t1, t2, ttot = 0, t ;
    printf("\nX[%d][%d], L1: %d, L2: %d\n",M,N,NL1,NL2);
	
    t1 = omp_get_wtime() ;
    Initialise_X();//Random values X[M][N] and Y[M][NL2]
    Initialise_W();//Random values [-0.1,0.1] for Weights
    t2 = omp_get_wtime() ;
    ttot += t = t2-t1 ;
    printf("Initialise X,Y,W :  %lf\n",t);

    SaveWeightsToFile("Weights_t0.csv");
    
//*
    for(int epoch=0;epoch<EPOCH;epoch++){
      double error=0;
      t1 = omp_get_wtime() ;
      for(int d=0;d<M-TEST;d++){
            activateNN((double *)X[d]);
            trainNN((double *)X[d],(double *)Y[d]);
            error+=MSE((double*)Y[d]);
        }
        t2 = omp_get_wtime() ;
        ttot += t = t2-t1 ;
        if(epoch%100==0){
            printf("%4.0d MSE: %lf , ",epoch,error/(M-TEST));
            printf("time : %lf\n",t);
        }
    }
    //*/
//Test
//* 
    t1 = omp_get_wtime() ;
    printf("\nTest\n");
    int count =0;
    int estimated=0;
    int category =0;
    double err=0;
    for(int d=M-TEST;d<M;d++){
        activateNN((double *)X[d]);
        estimated=max_index(NL2,OL2);
        category=max_index(NL2,(double*)Y[d]);
        if(estimated==category) count++;
        err+=MSE((double*)Y[d]);
        // printf("%lf,%lf - > %d\n",X[d][0],X[d][1],category);
    }
    printf("Test MSE: %lf\n",err/TEST);
    printf("Test Correct: %d of %d \n",count,TEST);
    t2 = omp_get_wtime() ;
    ttot += t = t2-t1 ;
    printf("    time : %lf\n",t);

    printf("Total time : %lf\n",ttot);
//  */   
    SaveWeightsToFile("Weights_t1.csv");

    return 0;
}

//*********************************************************
void Initialise_X(){

    for(int j=0;j<M;j++){
        for (int i=0;i<N;i++)
            //random values [-1,1]
            X[j][i]=((rand()/(double)RAND_MAX)*2.0-1.0);
    }

    for(int j=0;j<M;j++){
        for (int i=0;i<NL2;i++)
            //random values [-1,1]
            Y[j][i]=((rand()/(double)RAND_MAX)*2.0-1.0);
    }        
}

//*********************************************************

void Initialise_W(){

    for (int j=0;j<=N;j++)
        for (int i=0;i<NL1;i++)
    //random values [-0.1,0.1]
            WL1[i][j]=((rand()/(double)RAND_MAX)*0.2-0.1);
            
    
    for (int j=0;j<=NL1;j++)
        for (int i=0;i<NL2;i++)
    //random values [-0.1,0.1]
            WL2[i][j]=((rand()/(double)RAND_MAX)*0.2-0.1);

}
//*********************************************************

double sigmoid(double x){return (1.0/(1+exp(-x)));}
double dsigmoid(double y){return y*(1-y);}

//*********************************************************

void forward(const int n,const int m,double w[n][m+1],double x[m],double y[n]){

    double sum;

    for (int i=0;i<n;i++){
        sum=w[i][m];//bias
        for(int j=0;j<m;j++){
            sum += w[i][j]*x[j];
        }
        y[i]=sigmoid(sum);
    }   
}

//*********************************************************

double sum_delta(int j,const int nl1,const int nl2,double DL[],double W[nl2][nl1+1]){
    double sum=0;
    for(int l=0;l<nl2;l++){
        sum+=DL[l]*W[l][j];
    }
    return sum;
}

//*********************************************************


void updateweight(const int n,const int k,double W[n][k+1],double input[k],double delta[n]){
    // !!! W[n][k+1] !!!
    //n : number of neurons output layer
    //k : input layer
    for(int i=0;i<n;i++){
        double temp=delta[i];
    // #pragma omp parallel for firstprivate(temp) shared
        for(int j=0;j<k;j++){
            W[i][j] += a*temp*input[j]; 
        }        
        W[i][k] += a*temp;//bias update
    }

}

//*********************************************************

void activateNN(double *Vector){
    //Layer 1
    forward(NL1,N,WL1,Vector,OL1);
    
    //Layer 2
    forward(NL2,NL1,WL2,OL1,OL2);
    
}

//*********************************************************
void trainNN(double *input,double *desired){

    //Output Layer 2

    for(int i=0;i<NL2;i++){
        double o2=OL2[i];
        DL2[i]=dsigmoid(o2)*(o2-desired[i]);

    }

    //Hidden Layer 1

    for(int i=0;i<NL1;i++){
        double sum = sum_delta(i,NL1,NL2,DL2,WL2);
        double o1= OL1[i];
        DL1[i]=sum*dsigmoid(o1);
    }
    
    // New W
    updateweight(NL2,NL1,WL2,OL1,DL2);
    updateweight(NL1,N,WL1,input,DL1);
}

//*********************************************************

double MSE(double *desired){

    double sum = 0;
    for(int n=0;n<NL2;n++){
        double dif=desired[n]-OL2[n];
        sum+=(dif*dif);
    }
    return sum/NL2;
    

}
//*********************************************************
int max_index(int n,double *arr){

    double max=arr[0];
    int index = 0;
    for(int i=0;i<n;i++){
        if(max<arr[i]){
            max=arr[i];
            index = i;
        }  
    }
    return index;
}
//*********************************************************

void SaveWeightsToFile(char *fileName){

FILE *filePointer ; 

filePointer = fopen(fileName, "w");

fprintf(filePointer, "WL1[%d][%d]\n",NL1,N+1);
for(int i=0;i<NL1;i++){
    for(int j=0;j<N+1;j++){
        fprintf(filePointer,"%lf, ",WL1[i][j]);
    }
    fprintf(filePointer,"\n");
}

fprintf(filePointer, "\nWL2[%d][%d]\n",NL2,NL1+1);
for(int i=0;i<NL2;i++){
    for(int j=0;j<NL1+1;j++){
        fprintf(filePointer,"%lf, ",WL2[i][j]);
    }
    fprintf(filePointer,"\n");
}


fclose(filePointer);
    
}