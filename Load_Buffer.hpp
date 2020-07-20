#include "exceptions.hpp"

#ifndef LOAD_BUFFER_HPP
#define LOAD_BUFFER_HPP

class Load_Buffer
{
    friend class RISCV_Tomasulo;

private:
    ReservationUnit q[5];
    Reservationtype type[5];
    uint count_work;
    uint count_spare;

public:
    Load_Buffer()
    {
        type[0] = L0, type[1] = L1, type[2] = L2, type[3] = L3, type[4] = L4;
        count_work = 0;
        count_spare = 0;
    }

    Reservationtype push(ReservationUnit &x)
    {
        int i;
        for (i = 0; i < 5; ++i)
        {
            if (!q[i].used)
            {
                count_work++;
                q[i] = x;
                if (!x.Qj)
                {
                    count_spare++;
                    q[i].spare = true;
                }
                break;
            }
        }
        return type[i];
    }

    ReservationUnit find(Reservationtype &ty)
    {
        ReservationUnit tmp;
        for (int i = 0; i < 5; ++i)
        {
            if (q[i].used && q[i].spare && !q[i].Qj)
            {
                ty = type[i];
                tmp = q[i];
                break;
            }
        }
        return tmp;
    }

    ReservationUnit find_next(Reservationtype &ty)
    {
        ReservationUnit tmp;
        int i;
        for (i = ty - L0 + 1; i < 5; ++i)
        {
            if (q[i].used && q[i].spare && !q[i].Qj)
            {
                ty = type[i];
                tmp = q[i];
                break;
            }
        }
        if (i == 5)
            ty = N0;
        return tmp;
    }

    void pop(Reservationtype ty)
    {
        count_spare--;
        q[ty - L0].work = true;
        q[ty - L0].spare = false;
    }

    void update(Reservationtype ty, uint val)
    {
        for (int i = 0; i < 5; ++i)
        {
            if (q[i].used)
            {
                if (q[i].Qj == ty)
                {
                    q[i].Vj += val;
                    q[i].Qj = N0;
                    if (!q[i].work && !q[i].spare)
                    {
                        count_spare++;
                        q[i].spare = true; 
                    }
                }
            }
        }
        q[ty - L0].used = false;
        count_work--;
    }

    void clear()
    {
        for (int i = 0; i < 5; ++i)
            q[i].used = false;
        count_work = 0;
        count_spare = 0;
    }

    bool empty() { return count_work == 0; }
    bool full() { return count_work == 5; }
    bool idle() { return count_spare != 0; }
};

#endif