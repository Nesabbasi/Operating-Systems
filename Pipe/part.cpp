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

void get_data(const string file_path, vector<string> &part_data)
{
    ifstream part_file(file_path);
    string line;
    while (getline(part_file, line))
    {
        part_data.push_back(line);
    }
}

vector<string> split(const string &s, char delim)
{
    vector<string> result;
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim))
    {
        result.push_back(item);
    }
    return result;
}

void clean_data(vector<string> part_data, vector<pair<string, int>> &genre_number, const char file_id)
{
    vector<string> new_part_data;
    for (int i = 0; i < part_data.size(); i++)
    {
        new_part_data = split(part_data[i], ',');
        for (int j = 0; j < genre_number.size(); j++)
        {
            for (int k = 1; k < new_part_data.size(); k++)
            {
                if (new_part_data[k] == genre_number[j].first)
                {
                    genre_number[j].second += 1;
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    string file_path;
    cin >> file_path;
    char file_id = file_path[12];
    vector<string> part_data;
    vector<string> genre_data = split(argv[1], ',');
    vector<pair<string, int>> genre_number;

    for (auto i : genre_data)
    {
        genre_number.push_back(make_pair(i, 0));
    }

    get_data(file_path, part_data);
    clean_data(part_data, genre_number, file_id);

    int fd;
    for (int i = 0; i < genre_number.size(); i++)
    {
        string fifo_name = "/tmp/fifo_part" + string(1, file_id) + "_" + genre_number[i].first;
        fd = open(fifo_name.c_str(), O_WRONLY);
        string message = to_string(genre_number[i].second);
        write(fd, message.c_str(), message.size() + 1);
        close(fd);
    }
}