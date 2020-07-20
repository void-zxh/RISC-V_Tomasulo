#include "exceptions.hpp"
#include "Instruction.hpp"

#ifndef PREDICTION_HPP
#define PREDICTION_HPP

using namespace std;

class Prediction
{
private:
    /*
        STRONG_NOT_TAKEN----------------0
        WEAK_NOT_TAKEN------------------1
        WEAK_TAKEN----------------------2
        STRONG_TAKEN--------------------3
    */
    uint history[200005];
    uint count, correct;

public:
    Prediction()
    {
        memset(history, 0, sizeof(history));
        count = 0;
        correct = 0;
    }

    bool getPrediction(uint resultpc,Instruction inst)
    {
        uint x = resultpc & (0xffu);
        count++;
        return (history[x >> 2] >> ((x & 3) << 1u)) & 3;
    }

    void update(ROBunit tmp)
    {
        uint x = tmp.resultpc, now = history[x >> 2] >> ((x & 3) << 1);
        if (bool(tmp.num) == tmp.pred)
            correct++;
        if (tmp.pred)
        {
            if (now < 3)
                now++;
        }
        else
        {
            if (now > 0)
                now--;
        }

        history[x >> 2] |= 3 << ((x & 3) << 1);
        history[x >> 2] &= now << ((x & 3) << 1);
    }

    void out()
    {
        cout << "Correct " << correct << " times" << endl;
        cout << "Total " << count << " times\n";
        cout << "Accuracy: " << (double)correct / count * 100 << "%\n";
    }
};

#endif