// OSp2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#define HAVE_STRUCT_TIMESPEC
#define NUM_THREADS 4
#define PTHREADM PTHREAD_MUTEX_INITIALIZER
#define PTHREADC PTHREAD_COND_INITIALIZER
#include <pthread.h>
#include <string>
#include <cstdlib>
#include <stdlib.h>
#include <ctime>
#include <random>
#include "semaphore.h"
#include <fstream>
using namespace std;

ofstream report;

void* player(void* threadid);
void* dealer(void* threadid);
void rolldice(long id);
void dice_handler(long threadid);
void compare_handler();
int toss1, toss2;
int sumA, sumB, sumC, sumD;
bool win = false;
int current = 12;
pthread_mutex_t mutext_pass_cond = PTHREADM;
bool Apass = false;
pthread_cond_t A_condition = PTHREADC;

bool Bpass = false;
pthread_cond_t B_condition = PTHREADC;

bool Cpass = false;
pthread_cond_t C_condition = PTHREADC;

bool Dpass = false; 
pthread_cond_t D_condition = PTHREADC;


static pthread_mutex_t func_mutex;
static pthread_cond_t pass;
pthread_cond_t checkit = PTHREADC;
bool checkingit = false;

pthread_t dealer_threads_handler;
static pthread_cond_t check_condition;

pthread_t player_threads_handler[NUM_THREADS];

pthread_mutex_t mutext_exit_cond = PTHREADM;
pthread_cond_t win_condition = PTHREADC;

std::default_random_engine generator;
std::uniform_int_distribution<int> distribution(1, 6);
std::uniform_int_distribution<int> dealer_choose(0, 3);


void* dealer(void* threadid) {
    pthread_mutex_lock(&mutext_exit_cond);
    dice_handler((long)threadid);
    while (!win) {
        checkingit = false;
        while (!checkingit) {
            pthread_cond_wait(&checkit, &mutext_exit_cond);
            
        }

        compare_handler();
        pthread_cond_signal(&check_condition);
    }
    pthread_mutex_unlock(&mutext_exit_cond);
    exit(0);

}


void dice_handler(long threadid) {
    int d_choice;

    if (threadid == 10)
    {
        d_choice = dealer_choose(generator);
        switch (d_choice) {
        case 0:
            Apass = true;
            current = 0;
            pthread_cond_signal(&A_condition);
            break;
        case 1:
            Bpass = true;
            current = 1;
            pthread_cond_signal(&B_condition);
            break;
        case 2:
            Cpass = true;
            current = 2;
            pthread_cond_signal(&C_condition);
            break;
        case 3:
            Dpass = true;
            current = 3;
            pthread_cond_signal(&D_condition);
            break;
        }
    }
    else
    {
        switch(threadid){
        case 0:
            Apass = false;
            Bpass = true;
            pthread_cond_signal(&B_condition);
            break;
        case 1:
            Bpass = false;
            Cpass = true;
            pthread_cond_signal(&C_condition);
            break;
        case 2:
            Cpass = false;
            Dpass = true;
            pthread_cond_signal(&D_condition);
            break;
        case 3:
            pthread_cond_signal(&A_condition);
            Dpass = false;
            Apass = true;

            break;
        }
    }

}


void* player(void *threadid){
    pthread_mutex_lock(&func_mutex);
    while (!win) {
        long id = (long)threadid;
        switch (id) {
        case 0:
            while (!Apass)
                pthread_cond_wait(&A_condition, &func_mutex);
            current = 0;
            break;
        case 1:
            while (!Bpass)
                pthread_cond_wait(&B_condition, &func_mutex);
            current = 1;
            break;
        case 2:
            while (!Cpass)
                pthread_cond_wait(&C_condition, &func_mutex);
            current = 2;
            break;
        case 3:
            while (!Dpass)
                pthread_cond_wait(&D_condition, &func_mutex);
            current = 3;
            break;
        }
        rolldice(id);
        pthread_cond_signal(&checkit);
        checkingit = true;
        while (checkingit) {
            pthread_cond_wait(&check_condition, &func_mutex);
        }
        dice_handler(id);
    }
    pthread_mutex_unlock(&func_mutex);
    return 0;
}

void compare_handler() {
    switch (current) {
    case 0:
        if (sumA == sumC) {
            
            //pthread_cond_signal(&win_condition);
            cout << "Dealer: The winning team is B and D!" << endl;
            report << "Dealer: The winning team is A and C!" << endl;
            report.close();
            exit(0);
        }
        break;
    case 1:
        if (sumB == sumD) {
            //return true;
            //pthread_cond_signal(&win_condition);
            cout << "Dealer: The winning team is B and D!" << endl;
            report << "Dealer: The winning team is B and D!" << endl;
            report.close();
            exit(0);
        }
        break;
    case 2:
        if (sumA == sumC) {
            //return true;
            //pthread_cond_signal(&win_condition);
            cout << "Dealer: The winning team is B and D!" << endl;
            report << "Dealer: The winning team is A and C!" << endl;
            report.close();
            exit(0);
        }
        break;
    case 3:
        if (sumB == sumD) {
            //return true;
            //pthread_cond_signal(&win_condition);
            cout << "Dealer: The winning team is B and D!" << endl;
            report << "Dealer: The winning team is B and D!" << endl;
            report.close();
            exit(0);
        }
        break;
    }
}

void rolldice(long id) {
    
    toss1 = distribution(generator);
    toss2 = distribution(generator);
    switch(id)
    {
    case 0:
        sumA = toss1 + toss2;
        cout << "player A: " << toss1 << " " << toss2 << endl;
        report << "Player A: gets " << toss1 << " and " << toss2 << " with a sum of " << sumA << endl;
        break;
    case 1:
        sumB = toss1 + toss2;
        cout << "player B: " << toss1 << " " << toss2 << endl;
        report << "Player B: gets " << toss1 << " and " << toss2 << " with a sum of " << sumB << endl;
        break;
    case 2:
        sumC = toss1 + toss2;
        cout << "player C: " << toss1 << " " << toss2 << endl;
        report << "Player C: gets " << toss1 << " and " << toss2 << " with a sum of " << sumC << endl;
        break;
    case 3:
        sumD = toss1 + toss2;
        cout << "player D: " << toss1 << " " << toss2 << endl;
        report << "Player D: gets " << toss1 << " and " << toss2 << " with a sum of " << sumD << endl;
        break;
    }

}
int main(int argc, char* argv[])
{
    report.open("report.txt");
    if (argv[1] != NULL)
    {
        int seedvalue = atoi(argv[1]);
        generator.seed(seedvalue);
    }
    else {
        generator.seed(67676767);
    }
    pthread_mutex_init(&func_mutex, NULL);
    pthread_cond_init(&pass, NULL);
    int t_player;
    
    int deal = pthread_create(&dealer_threads_handler, NULL, &dealer, (void*)10);
    for (long a = 0; a < NUM_THREADS; a++) {
        t_player = pthread_create(&player_threads_handler[a], NULL, &player, (void*)a);
    }
    pthread_join(dealer_threads_handler, NULL); 
    for (long b = 0; b < NUM_THREADS; b++) {
       pthread_join(player_threads_handler[b], NULL);
    }
    report.close();
    exit(EXIT_SUCCESS);
}
