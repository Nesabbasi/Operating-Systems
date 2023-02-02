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
#include <algorithm>
#include "defs.h"

using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void get_data(const string file_path, vector<string> &genre_data)
{
    ifstream genre_file(file_path);
    string line;
    while (getline(genre_file, line, ','))
    {
        genre_data.push_back(line);
    }
}

int main(int argc, char *argv[])
{
    string library = argv[1];
    const filesystem::path directory{library};
    vector<string> file_path;
    vector<string> genre_data;
    for (auto const &dir_entry : filesystem::directory_iterator{directory})
    {
        file_path.push_back(dir_entry.path());
    }

    for (auto file : file_path)
    {
        if (file == "library/genres.csv")
        {
            get_data(file, genre_data);
        }
    }

    for (int i = 0; i < genre_data.size(); i++)
    {
        for (int j = 0; j < file_path.size() - 1; j++)
        {
            string fifo_name = "/tmp/fifo_part" + to_string(j + 1) + "_" + genre_data[i];
            if (mkfifo(fifo_name.c_str(), 0666) == -1)
            {
                if (errno != EEXIST)
                    error("ERROR on mkfifo");
            }
        }
    }

    string genre_data_send = genre_data[0];
    for (int j = 1; j < genre_data.size(); j++)
    {
        genre_data_send += "," + genre_data[j];
    }

    for (auto file : file_path)
    {
        if (file == "library/genres.csv")
        {
            for (int i = 0; i < genre_data.size(); i++)
            {
                int fd[2];
                if (pipe(fd) == -1)
                    error("ERROR on pipe()");

                int pid = fork();

                if (pid < 0)
                    error("ERROR on fork()");

                if (pid == 0) //child process
                {
                    close(fd[WRITE]);
                    dup2(fd[READ], STDIN_FILENO);
                    close(fd[READ]);
                    if (execl("./genre.out", "./genre.out", to_string(file_path.size()).c_str(), NULL) == -1)
                    {
                        error("execl() Error!");
                    }
                    close(fd[READ]);
                }
                else
                {
                    write(fd[WRITE], genre_data[i].c_str(), genre_data[i].size() + 1);
                    close(fd[WRITE]);
                }
            }
        }
        else
        {
            int fd[2];
            if (pipe(fd) == -1)
                error("ERROR on pipe()");

            int pid = fork();

            if (pid < 0)
                error("ERROR on fork()");

            if (pid == 0) //child process
            {
                close(fd[WRITE]);
                dup2(fd[READ], STDIN_FILENO);
                close(fd[READ]);
                if (execl("./part.out", "./part.out", genre_data_send.c_str(), NULL) == -1)
                {
                    error("execl() Error!");
                }
                close(fd[READ]);
            }
            else
            {
                write(fd[WRITE], file.c_str(), file.size() + 1);
                close(fd[WRITE]);
            }
        }
    }

    for (int i = 0; i < genre_data.size(); i++)
    {
        for (int j = 0; j < file_path.size() - 1; j++)
        {
            wait(NULL);
        }
    }
}