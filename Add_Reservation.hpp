#include "exceptions.hpp"

#ifndef ADD_RESERVATION_HPP
#define ADD_RESERVATION_HPP

class Add_Reservation
{
    friend class RISCV_Tomasulo;

private:
    ReservationUnit q[4];
    Reservationtype type[4];
    uint count_work;
    uint count_spare;

public:
    Add_Reservation()
    {
        type[0] = A0, type[1] = A1, type[2] = A2, type[3] = A3;
        count_work = 0;
        count_spare = 0;
    }

    Reservationtype push(ReservationUnit &x)
    {
        int i;
        for (i = 0; i < 4; ++i)
        {
            if (!q[i].used)
            {
                count_work++;
                q[i] = x;
                if (!x.Qj && !q[i].Qk)
                {
                    count_spare++;
                    q[i].spare = true;
                }
                break;
            }
        }
        return type[i];
    }

    ReservationUnit pop(Reservationtype &ty)
    {
        ReservationUnit tmp;
        for (int i = 0; i < 4; ++i)
        {
            if (q[i].used && q[i].spare && !q[i].Qj && !q[i].Qk)
            {
                count_spare--;
                q[i].spare = false;
                q[i].work = true;
                ty = type[i];
                tmp = q[i];
                break;
            }
        }
        return tmp;
    }

    void update(Reservationtype ty, uint v)
    {
        for (int i = 0; i < 4; ++i)
        {
            if (q[i].used)
            {
                if (q[i].Qj == ty)
                {
                    q[i].Vj = v;
                    q[i].Qj = N0;
                }
                if (q[i].Qk == ty)
                {
                    q[i].Vk = v;
                    q[i].Qk = N0;
                }
                if (!q[i].Qj && !q[i].Qk && !q[i].spare && !q[i].work)
                {
                    count_spare++;
                    q[i].spare = true;
                }
            }
        }
        if (ty >= A0 && ty <= A3)
        {
            count_work--;
            q[ty - A0].used = false;
        }
    }

    void clear()
    {
        for (int i = 0; i < 4; ++i)
            q[i].used = false;
        count_work = 0;
        count_spare = 0;
    }
    
    bool empty() { return count_work == 0; }
    bool full() { return count_work == 4; }
    bool idle() { return count_spare != 0; }
};

#endif