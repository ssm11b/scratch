
#include <iostream>
#include <stdint.h>
#include "rdrand.h"

using namespace std;

int main()
{
        for (int i = 0; i < 10; i++)
        {
                cout << hex << rand16() << " " << rand32() << " " << rand64() << endl;
        }
        return 0;
}

