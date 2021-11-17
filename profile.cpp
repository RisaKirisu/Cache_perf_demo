#include <iostream>
#include <chrono>
#include <random>
#include <vector>

class Timer
{
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimePoint;
    bool _donothing = false;

public:
    static double *record;
    static size_t index;

    Timer() 
    {
        m_StartTimePoint = std::chrono::high_resolution_clock::now();
    }
    Timer(int a)
    {
        _donothing = true;
    }
    ~Timer() 
    {
        if (!_donothing)
        {
            stop();
        }
    }
    
    void stop()
    {
        std::chrono::time_point<std::chrono::high_resolution_clock> endTimePoint = std::chrono::high_resolution_clock::now();
        auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimePoint).time_since_epoch().count();
        auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint).time_since_epoch().count();

        auto duration = end - start;
        double ms = duration * 0.001;

        record[index] = duration;
        index ++;
        std::cout << duration << "us (" << ms << "ms)\n";
    }
};

void multiply_test(int n)
{
    using mat = std::vector<std::vector<float>>;
    std::mt19937 gen(114514);
    std::uniform_real_distribution<float> dis(0, 1.0);
    
    mat* ogA = new mat(n, std::vector<float>(n));
    mat* ogB = new mat(n, std::vector<float>(n));
    mat* ogC = new mat(n, std::vector<float>(n));
    mat* A;
    mat* B;
    mat* C;
    
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            (*ogA)[i][j] = dis(gen);
            (*ogB)[i][j] = dis(gen);
            (*ogC)[i][j] = dis(gen);
        }
    }


    std::cout << "ijk: ";
    A = new mat(*ogA);
    B = new mat(*ogB);
    C = new mat(*ogC);
    // 2n^3 flop
    {
        Timer timer;
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                float sum = 0.0;
                for (int k = 0; k < n; k++)
                {
                    sum += (*A)[i][k] * (*B)[k][j];
                }
                (*C)[i][j] = sum;
            }
        }
    }
    delete A;
    delete B;
    delete C;

    std::cout << "kij: ";
    A = new mat(*ogA);
    B = new mat(*ogB);
    C = new mat(*ogC);
    // 2n^3 flop
    {
        Timer timer;
        for (int k = 0; k < n; k++)
        {
            for (int i = 0; i < n; i++)
            {
                float r = (*A)[i][k];
                for (int j = 0; j < n; j++)
                {
                    (*C)[i][j] += r * (*B)[k][j];   
                }
            }
        }
    }
    delete A;
    delete B;
    delete C;

    std::cout << "jki: ";
    A = new mat(*ogA);
    B = new mat(*ogB);
    C = new mat(*ogC);
    // 2n^3 flop
    {
        Timer timer;
        for (int j = 0; j < n; j++)
        {
            for (int k = 0; k < n; k++)
            {
                float r = (*B)[k][j];
                for (int i = 0; i < n; i++)
                {
                (*C)[i][j] += (*A)[i][k] * r;
                }
            }
        }
    }
    delete A;
    delete B;
    delete C;

    delete ogA;
    delete ogB;
    delete ogC;
}

double* Timer::record;
size_t Timer::index = 0;

# define N 13
int main()
{       
    double record[N][3][3] = {};
    int sizes[] = {10, 50, 100, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000};
    for (int i = 0; i < N; i++)
    {
        int n = sizes[i];
        if (i <= 5)
        {
            for (int j = 0; j < 3; j++)
            {
                Timer::record = &record[i][j][0];
                Timer::index = 0;
                multiply_test(n);
            }
        }
        else
        {   
            Timer::record = &record[i][0][0];
            Timer::index = 0;            
            multiply_test(n);
            for (int j = 1; j < 3; j++)
            {
                record[i][j][0] = record[i][0][0];
                record[i][j][1] = record[i][0][1];
                record[i][j][2] = record[i][0][2];
            }
        }
    

    }

    double record_avg[N][3] = {};
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            double sum = 0;
            for (int k = 0; k < 3; k++)
            {
                sum += record[i][k][j];
                std::cout << record[i][k][j] << std::endl;
            }
            record_avg[i][j] = sum / 3;
        }
    }
    
    std::cout << "                     ijk      kij      jki" << std::endl;
    for (int i = 0; i < N; i++)
    {
        std::cout << "matrix length: " << sizes[i] << ": ";
        for (int j = 0; j < 3; j++)
        {
            std::cout << record_avg[i][j] << "  ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    std::cout << "------For copy------" << std::endl;
    for (int j = 0; j < 3; j++)
    {
        for (int i = 0; i < N; i++)
        {
            std::cout << record_avg[i][j] << std::endl;
        }
        std::cout << "----------" << std::endl;
    }

    return 0;
}