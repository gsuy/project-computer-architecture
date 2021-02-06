#include <iostream>
#include <string>
#include <cstdio>
#include <string.h>
#include <cmath>
using namespace std;

string twoComplement(string str, int bit)
{
    int n = str.length();
    char tmp[bit], onesComp[bit], twosComp[bit];

    strcpy(tmp, str.c_str());
    int i, carry = 1;
    /* Find ones complement of the binary number */
    for (i = 0; i < bit; i++)
    {
        if (tmp[i] == '1')
        {
            onesComp[i] = '0';
        }
        else if (tmp[i] == '0')
        {
            onesComp[i] = '1';
        }
    }
    onesComp[bit] = '\0';

    /*
     * Add 1 to the ones complement
     */
    for (i = bit - 1; i >= 0; i--)
    {
        if (onesComp[i] == '1' && carry == 1)
        {
            twosComp[i] = '0';
        }
        else if (onesComp[i] == '0' && carry == 1)
        {
            twosComp[i] = '1';
            carry = 0;
        }
        else
        {
            twosComp[i] = onesComp[i];
        }
    }
    twosComp[bit] = '\0';
    return twosComp;
}

string offSet(string str)
{
    int num = atoi(str.c_str());
    char tmp[16];
    if (num > 0)
    {
        string s = "";
        for (int i = 0; i < 16; i++)
        {
            tmp[i] = '0' + (num % 2);
            num /= 2;
            s = tmp[i] + s;
        }
        return s;
    }
    else
    {
        string t;
        t = str.substr(1, 2);
        t = offSet(t);
        return twoComplement(t, 16);
    }
}
