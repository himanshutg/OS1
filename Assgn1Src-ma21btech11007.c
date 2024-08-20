#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

// Checks if a number is Tetrahedral or not,
//  will be used by every process to do the calclulations/check
//  if it is a Tetrahedral number
bool isTetrahedrel(int n)
{
    for (int p = 1; p * (p + 1) * (p + 2) <= 6 * n; p++)
    {
        if (6 * n == p * (p + 1) * (p + 2))
        {
            // printf("The number %d is Thedral\n", n);
            return 1;
        }
    }
    return 0;
}

int main()
{

    FILE *inputFile;
    int N, K;

    // Open the input file
    if ((inputFile = fopen("input.txt", "r")) == NULL)
    {
        printf("Error opening input file.");
        return 0;
    }

    // Read N and K from the input file
    fscanf(inputFile, "%d %d", &N, &K);

    // closing the input file
    fclose(inputFile);

    // int N = 100;
    // int K = 10;

    //Checking for the validity of the Input
    if (N <= 0 || K <= 0)
    {
        printf("Input is Invalid");
        return 0;
    }

    //Declaring the shared memory for the child processes
    int shared_mem;
    int *ptr[K];

    //size denotes the size of memory for each child, is equal to ceil of N/K
    int size;
    if(N%K ==0) size = N/K;
    else size = N/K +1;

    //Names of shared memory of each child and allocating shared memory for each child
    char* shared_mem_name[K];
    for (int i = 0; i < K; i++)
    {
        char name[100];
        sprintf(name, "%d", i);
        shared_mem = shm_open(name, O_CREAT | O_RDWR, 0666);
        ftruncate(shared_mem, K * size * sizeof(int));
        ptr[i] = (int *)mmap(0, size * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shared_mem, 0);
        shared_mem_name[i] = (char *)malloc(100 * sizeof(char));
        sprintf(shared_mem_name[i], "%d", i);
    }

    //Creating K Child processes and making them check the required numbers and storing them to their respective shared memories.
    for (int i = 0; i < K; i++)
    {
        int pid = fork();
        int number_To_Be_Checked = i + 1;
        if (pid == 0)
        {
            int shared_mem = shm_open(shared_mem_name[i], O_RDWR, 0666);
            int *shared_mem_ptr = (int *)mmap(0, size * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shared_mem, 0);
            
            int counter = 0;//Variable that counts the number of numbers checked for each process

            while (number_To_Be_Checked <= N)
            {
                //Checkingg if the number is Tetrahedral and storing to the Shared Memory
                shared_mem_ptr[counter] = isTetrahedrel(number_To_Be_Checked);

                // printf("Process Number %d Number : %d Result: %d\n", i, number_To_Be_Checked, shared_mem_ptr[counter]);
                counter += 1;
                number_To_Be_Checked = number_To_Be_Checked + K;
            }
            exit(0);
        }
    }


    //Waiting for all Child Processes to Finish executing
    for (int i = 0; i < K; i++)
    {
        wait(NULL);
    }

    //OPening the main Output file to store the tetrahedral Numbers for all the processes.
    FILE * Output_Main = fopen("OutMain","w");
    for (int i = 0; i < K; i++)
    {

        fprintf(Output_Main, "P%d :", i+1);

        char process_Number[100];
        sprintf(process_Number,"%d",i+1);
        char opfile[] = "OutputFile";
        strcat(opfile,process_Number);
        strcat(opfile, ".txt");

        //OPening the output file to store the tetrahedral Numbers for each process.
        FILE * Output = fopen(opfile,"w");

        int number_To_Be_Checked = i + 1;
        int counter = 0;
        while (number_To_Be_Checked <= N)
        {

            if(ptr[i][counter]){
                    fprintf(Output, "%d : %s\n", number_To_Be_Checked, "a tetrahedral number");
                    fprintf(Output_Main,"%d ", number_To_Be_Checked);
                }
                else{
                    fprintf(Output, "%d : %s\n", number_To_Be_Checked, "Not a tetrahedral number");
                }

            // printf("\nProcess Number %d Number : %d Result: %d", i, number_To_Be_Checked, ptr[i][counter]);
            counter += 1;

            number_To_Be_Checked = number_To_Be_Checked + K;
        }
        fclose(Output);
        fprintf(Output_Main,"\n");
    }
    fclose(Output_Main);

    // Close shared memory and free allocated memory
    for (int i = 0; i < K; i++)
    {
        munmap(ptr[i], size * sizeof(int));
        close(shared_mem);
        free(shared_mem_name[i]);
    }

    return 0;
}