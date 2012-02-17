#include <sstream>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void createAnsiFile(const std::string& input);

int callFork(unsigned int min, unsigned int max, const std::string& input, const std::string& output)
{
     pid_t pid_courant = getpid();
     pid_t pid_pere;
     //printf("I'm the first process, number %d\n", pid_courant);

     int nbFils = 0;
     unsigned int minFils, maxFils;

     while (min < max && nbFils < 2) {
          for (int i = 1; i <= 2; i++) {
               pid_pere = pid_courant;

               // Determining intervals for each son
               if (i == 1) {
                    minFils = min;
                    maxFils = (min + max) / 2;
               } else {
                    minFils = ((min + max) / 2) + 1;
                    maxFils = max;
               }

               // Son's creation
               switch(fork()) {
               case 0: // Son's code
                    // Value's update
                    pid_courant = getpid();
                    min = minFils;
                    max = maxFils;
                    nbFils = 0;

                    //Write the value of his interval to a temp file
                    if(min >= max) {
                         std::ifstream f(input.c_str());
                         std::stringstream name;
                         name << getpid();
                         std::ofstream o(name.str().c_str());
                         int value;
                         f.seekg(min * sizeof(int));
                         f.read((char*)&value, sizeof(int));
                         o.write((char*)&value, sizeof(int));
                         o.close();
                    }
                    //printf("%d Created %d (%d/2); Interval : %d - %d\n", pid_pere, pid_courant, i, min, max);
                    break;
               default: // Father's code
                    nbFils++;
                    break;
               }
               if (getpid() != pid_pere) // If I'm the first son
                    break;
          }
     }

     // If I'm a father
     if (pid_pere == pid_courant) {
          //printf("%d : Waiting\n", pid_courant);

          // Wait until sons finished
          pid_t process[2];
          for (int j = 0; j < 2; j++) {
               process[j] = wait(NULL);
          }

          //Convert pid integer value into string (the purpose is to open file ;))
          std::stringstream fileName[3];
          fileName[0] << getpid();
          std::ofstream own(fileName[0].str().c_str());

          fileName[1] << process[0];
          std::ifstream left(fileName[1].str().c_str());

          fileName[2] << process[1];
          std::ifstream right(fileName[2].str().c_str());

          //int tmp;
          std::cout << pid_courant << ", Interval: [" << min <<  "," << max << "]" << std::endl;
          bool leftEnd = false;
          bool rightEnd = false;

          int pos = 0;

          int n1, n2;
          left.read((char*)&n1, sizeof(int));
          right.read((char*)&n2, sizeof(int));
          while(!leftEnd && !rightEnd) {
               if(n1 <= n2) {
                    own.write((char*)&n1, sizeof(int));
                    own.write((char*)&n2, sizeof(int));
               } else {
                    own.write((char*)&n2, sizeof(int));
                    own.write((char*)&n1, sizeof(int));
               }

               left.read((char*)&n1, sizeof(int));
               right.read((char*)&n2, sizeof(int));

               leftEnd = left.eof();
               rightEnd = right.eof();
          }

          own.close();
          left.close();
          right.close();

          std::remove(fileName[1].str().c_str());
          std::remove(fileName[2].str().c_str());
     }
     return 0;
}
