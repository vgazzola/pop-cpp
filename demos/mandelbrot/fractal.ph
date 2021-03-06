#ifndef FRACTAL_H
#define FRACTAL_H

#include "screen.ph"

parclass Fractal {
    classuid(1002);
public:
    Fractal();
    Fractal(POPString machine) @{ od.search(10, 3, 0); };//added lwk
    ~Fractal();
    async void generate([in] int w, [in] int h,
                        [in] float rm, [in] float rs, [in] float im, [in] float is,
                        [in] int tile, [in] Screen &s);
};

#endif
