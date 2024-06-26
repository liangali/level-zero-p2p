#include <stdio.h>
#include <stdlib.h>

#include "lz_context.h"

int parseInput(const std::string &input)
{
    int multiplier = 1;
    int length = input.length();

    char lastChar = std::tolower(input[length - 1]);
    if (lastChar == 'k')
    {
        multiplier = 1024;
        length--;
    }
    else if (lastChar == 'm')
    {
        multiplier = 1024 * 1024;
        length--;
    }

    for (int i = 0; i < length; ++i)
    {
        if (!std::isdigit(input[i]))
        {
            std::cerr << "ERROR: Invalid input (-n requires a number or number with k or m, e.g., 256, 2k, 4m)" << std::endl;
            exit(-1);
        }
    }

    int number = std::stoi(input.substr(0, length));

    return number * multiplier;
}

void parseCommandLine(int argc, char *argv[], int &local, int &remote, int &n)
{

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "-l")
        {
            if (i + 1 < argc)
            { // check if next parameter exists
                local = std::atoi(argv[++i]);
                if (local != 0 && local != 1)
                {
                    std::cerr << "ERROR: -l must be 0 or 1." << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                std::cerr << "ERROR: -l requires a number." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else if (arg == "-r")
        {
            if (i + 1 < argc)
            { // check if next parameter exists
                remote = std::atoi(argv[++i]);
                if (remote != 0 && remote != 1)
                {
                    std::cerr << "ERROR: -r must be 0 or 1." << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                std::cerr << "ERROR: -r requires a number." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else if (arg == "-n")
        {
            if (i + 1 < argc)
            { // check if next parameter exists
                n = parseInput(argv[++i]);
            }
            else
            {
                std::cerr << "ERROR: -n requires a number." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            std::cerr << "ERROR: Invalid argument." << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char **argv)
{
    int local_gpu = 0, remote_gpu = 1, data_count = 1024;
    parseCommandLine(argc, argv, local_gpu, remote_gpu, data_count);
    printf("#### Input parameters: loca_ gpu idx = %d, remote_gpu idx = %d, data_count = %d\n", local_gpu, remote_gpu, data_count);

    lzContext ctx0, ctx1;
    ctx0.initZe(local_gpu);
    ctx1.initZe(remote_gpu);

    queryP2P(ctx0.device(), ctx1.device());
    queryP2P(ctx1.device(), ctx0.device());

    void *buf0 = ctx0.createBuffer(data_count, 0);
    void *buf1 = ctx1.createBuffer(data_count, 1);
    printf("buf0 = %p, buf1 = %p\n", buf0, buf1);

    ctx0.printBuffer(buf0);
    ctx1.printBuffer(buf1);

    ctx0.runKernel("../../lz_p2p/test_kernel_dg2.spv", "local_read_from_remote", buf1, buf0, data_count);
    ctx0.printBuffer(buf0);

    ctx0.runKernel("../../lz_p2p/test_kernel_dg2.spv", "local_write_to_remote", buf1, buf0, data_count);
    ctx1.printBuffer(buf1);

    printf("done\n");
    return 0;
}