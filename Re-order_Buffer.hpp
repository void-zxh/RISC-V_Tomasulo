#include "exceptions.hpp"

#ifndef RE_ORDER_BUFFER_HPP
#define RE_ORDER_BUFFER_HPP

class Reorder_Buffer
{
    friend class RISCV_Tomasulo;

private:
    ROBunit q[8];
    Reservationtype type[8];
    uint head;
    uint tail;

public:
    Reorder_Buffer()
    {
        type[0] = R0, type[1] = R1, type[2] = R2, type[3] = R3, type[4] = R4, type[5] = R5, type[6] = R6, type[7] = R7;
        head = 0;
        tail = 1;
    }

    bool full() { return (tail + 1) % 8 == head; }
    bool empty() { return tail == (head + 1) % 8; }

    void set(ROBunit &x)
    {
        q[tail] = x;
        if (x.rs)
            reg[x.rs].Qi = type[tail];
        tail = (tail + 1) % 8;
    }

    ROBunit front(Reservationtype &cur_name)
    {
        cur_name = type[(head + 1) % 8];
        return q[(head + 1) % 8];
    }

    void pop()
    {
        q[head = (head + 1) % 8].used = false;
    }

    void clear()
    {
        for (int i = 0; i < 8; ++i)
            q[i].used = false;
        head = 0;
        tail = 1;
    }

    void update(Reservationtype pt, uint x)
    {
        for (int i = (head + 1) % 8; i != tail; i = (i + 1) % 8)
        {
            if (q[i].used)
            {
                if ((q[i].Op >= 25 && q[i].Op <= 27) || q[i].Op == 10)
                {
                    if (q[i].Qs == pt)
                    {
                        q[i].Vs += x;
                        q[i].Qs = N0;
                    }
                }
                if (q[i].Qi == pt)
                {
                    q[i].Qi = N0;
                    q[i].num = x;
                }
            }
        }
    }

    bool check(Reservationtype pt, uint x)
    {
        for (int i = (head + 1) % 8; q[i].Qi != pt; i = (i + 1) % 8)
            if (q[i].Op >= 25 && q[i].Op <= 27)
                if (q[i].Qs != N0 || q[i].Vs == x)
                    return false;
        return true;
    }
};

#endif