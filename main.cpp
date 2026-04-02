#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

#include <iostream>
#include <vector>
#include <climits>
#include <cstring>

static const int kMinMaxSleepMs = 7;
static const int kAverageSleepMs = 12;

struct ThreadData
{
    int* array;
    int size;
    int minValue;
    int maxValue;
    int minIndex;
    int maxIndex;
    double average;
};

void platformSleep(int milliseconds)
{
#ifdef _WIN32
    Sleep(static_cast<DWORD>(milliseconds));
#else
    usleep(static_cast<useconds_t>(milliseconds) * 1000);
#endif
}

void findMinMax(ThreadData* data)
{
    data->minValue = data->array[0];
    data->maxValue = data->array[0];
    data->minIndex = 0;
    data->maxIndex = 0;

    int i = 0;
    for (i = 1; i < data->size; ++i)
    {
        if (data->array[i] < data->minValue)
        {
            data->minValue = data->array[i];
            data->minIndex = i;
        }
        if (data->array[i] > data->maxValue)
        {
            data->maxValue = data->array[i];
            data->maxIndex = i;
        }
        platformSleep(kMinMaxSleepMs);
    }

    std::cout << "Min value: " << data->minValue
              << " (index " << data->minIndex << ")" << std::endl;
    std::cout << "Max value: " << data->maxValue
              << " (index " << data->maxIndex << ")" << std::endl;
}

void computeAverage(ThreadData* data)
{
    double sum = data->array[0];

    int i = 0;
    for (i = 1; i < data->size; ++i)
    {
        sum += data->array[i];
        platformSleep(kAverageSleepMs);
    }

    data->average = sum / data->size;
    std::cout << "Average value: " << data->average << std::endl;
}

#ifdef _WIN32

DWORD WINAPI minMaxThread(LPVOID param)
{
    ThreadData* data = static_cast<ThreadData*>(param);
    findMinMax(data);
    return 0;
}

DWORD WINAPI averageThread(LPVOID param)
{
    ThreadData* data = static_cast<ThreadData*>(param);
    computeAverage(data);
    return 0;
}

#else

void* minMaxThread(void* param)
{
    ThreadData* data = static_cast<ThreadData*>(param);
    findMinMax(data);
    return nullptr;
}

void* averageThread(void* param)
{
    ThreadData* data = static_cast<ThreadData*>(param);
    computeAverage(data);
    return nullptr;
}

#endif

void printArray(const int* array, int size)
{
    int i = 0;
    for (i = 0; i < size; ++i)
    {
        std::cout << array[i];
        if (i < size - 1)
        {
            std::cout << " ";
        }
    }
    std::cout << std::endl;
}

int main()
{
    try
    {
        int size = 0;
        std::cout << "Enter array size: ";
        std::cin >> size;

        if (size <= 0)
        {
            std::cerr << "Error: array size must be positive." << std::endl;
            return 1;
        }

        std::vector<int> array(static_cast<std::size_t>(size));

        std::cout << "Enter " << size << " integers: ";
        int i = 0;
        for (i = 0; i < size; ++i)
        {
            if (!(std::cin >> array[static_cast<std::size_t>(i)]))
            {
                std::cerr << "Error: invalid input." << std::endl;
                return 1;
            }
        }

        std::cout << "Original array: ";
        printArray(array.data(), size);

        ThreadData data;
        std::memset(&data, 0, sizeof(data));
        data.array = array.data();
        data.size = size;

#ifdef _WIN32

        HANDLE hMinMax = CreateThread(nullptr, 0, minMaxThread, &data, 0, nullptr);
        if (nullptr == hMinMax)
        {
            std::cerr << "Error: failed to create min_max thread." << std::endl;
            return 1;
        }

        HANDLE hAverage = CreateThread(nullptr, 0, averageThread, &data, 0, nullptr);
        if (nullptr == hAverage)
        {
            std::cerr << "Error: failed to create average thread." << std::endl;
            CloseHandle(hMinMax);
            return 1;
        }

        WaitForSingleObject(hMinMax, INFINITE);
        WaitForSingleObject(hAverage, INFINITE);

        // Release in reverse order of creation
        CloseHandle(hAverage);
        CloseHandle(hMinMax);

#else

        pthread_t tMinMax;
        pthread_t tAverage;

        int rc = pthread_create(&tMinMax, nullptr, minMaxThread, &data);
        if (0 != rc)
        {
            std::cerr << "Error: failed to create min_max thread. rc=" << rc << std::endl;
            return 1;
        }

        rc = pthread_create(&tAverage, nullptr, averageThread, &data);
        if (0 != rc)
        {
            std::cerr << "Error: failed to create average thread. rc=" << rc << std::endl;
            pthread_join(tMinMax, nullptr);
            return 1;
        }

        pthread_join(tMinMax, nullptr);
        pthread_join(tAverage, nullptr);

#endif

        // Replace min and max elements with average
        int avgInt = static_cast<int>(data.average);
        array[static_cast<std::size_t>(data.minIndex)] = avgInt;
        array[static_cast<std::size_t>(data.maxIndex)] = avgInt;

        std::cout << std::endl << "After replacing min and max with average (" << avgInt << "):" << std::endl;
        printArray(array.data(), size);
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Exception: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
