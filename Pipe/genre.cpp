#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <filesystem>
#include "defs.h"

using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    string name;
    cin >> name;
    string file_id = argv[1];
    int fd;
    char message[BUFF_SIZE];

    int count = 0;
    for (int j = 1; j < stoi(file_id); j++)
    {
        string fifo_name = "/tmp/fifo_part" + to_string(j) + "_" + name;
        fd = open(fifo_name.c_str(), O_RDONLY);
        if (fd < 0)
        {
            error("Read Failed!");
        }
        memset(message, 0, BUFF_SIZE);
        read(fd, message, sizeof(message));
        close(fd);
        count += atoi(message);
    }
    cout << name << ": " << count << endl;
}