// Program to play with the Riemann zeta function and its analytic continuation.

#include <iostream>
#include <complex>
extern "C"
{
#include "visualiser.h"
}

using namespace std;

static double pi = 3.14159265359; 
int how_calcd;

// Approximation to gamma function 
// (not sure whose this is, maybe Lanczos? nor how accurate it is)
static int g = 7;
static double p[] = { 0.99999999999980993, 676.5203681218851, -1259.1392167224028,
         771.32342877765313, -176.61502916214059, 12.507343278686905,
         -0.13857109526572012, 9.9843695780195716e-6, 1.5056327351493116e-7 };

complex<double> cgamma(complex<double> z)
{
    // Reflection formula
    if (real(z) < 0.5)
    {
        return pi / (sin(pi * z) * cgamma(1.0 - z));
    }
    else
    {
        z -= 1;
        complex<double> x = p[0];
        for (int i = 1; i < g + 2; i++)
        {
            x += p[i] / (z + (double)i);
        }
        complex<double> t = z + (double)g + 0.5;
        return sqrt(2.0 * pi) * (pow(t, z + 0.5)) * exp(-t) * x;
    }
}

// From here on it's all Wikipedia on the Riemann zeta function...
 
// Series zeta function. Not valid for real(s) < 1.0 or s = 1.0.
complex<double> series_zeta(complex<double> s)
{
    complex<double> zeta(0, 0);

   // cout << "Series zeta(" << s << ")" << endl;
    for (int i = 1; i < 100; i++)  // limit of 100 terms, arbitrarily
    {
        complex<double> term = 1.0 / pow((double)i, s);

        // cout << i << term << endl;
        zeta += term;
        if (abs(term) < 1.0e-7 * abs(zeta))
        {
           // cout << "Converged in " << i << " iterations" << endl;
            break;
        }
    }
    return zeta;
}

// Alternating sign zeta function for zeta of 0.0 < real(s) < 1.0
complex<double> alternating_zeta(complex<double> s)
{
    complex<double> zeta(0, 0);

   // cout << "Alternating zeta(" << s << ")" << endl;
    for (int i = 1; i < 100; i++)  // limit of 100 terms, arbitrarily
    {
        complex<double> term = ((i & 1) ? 1.0 : -1.0) / pow((double)i, s);

        // cout << i << term << endl;
        zeta += term;
        if (abs(term) < 1.0e-7 * abs(zeta))
        {
           // cout << "Converged in " << i << " iterations" << endl;
            break;
        }
    }
    return zeta * (1.0 / (1.0 - pow(2.0, 1.0 - s)));
}


// Riemann zeta function using functional equation to compute zeta for real(s) <= 0.0.
// Note: blows up for s = 0 (since it tries to calculate series_zeta(1.0) )
// For 0.0 < real(s) <= 1.0, we use the alternating zeta function.
// Auxiliary output: how_calcd = 0 series, 1 alternating, 2 functional eq.
complex<double> riemann_zeta(complex<double> s)
{
    complex<double> s1;

    if (real(s) > 1.0)
    {
        how_calcd = 0;
        return series_zeta(s);
    }

    if (real(s) > 0.0)
    {
        how_calcd = 1;
        return alternating_zeta(s);
    }
    
   // cout << "Functional equation zeta(" << s << ")" << endl;
    s1 = riemann_zeta(1.0 - s);
    how_calcd = 2;
    return pow(2.0, s) * pow(pi, s - 1.0) * sin(pi * s / 2.0) * cgamma(s) * s1;
}



int main()
{
    //complex<double> arg(0.999, 0);
    int w = 100;
    int h = 100;
    double step = 0.1;
    double imin = -5.0;
    double rmin = -5.0;
    int m, n;
    double i, r;
    CoordSet* coords;

    //cout << arg << endl;
    //cout << "Gamma: " << cgamma(arg) << endl;
    //cout << "Zeta terms:" << endl;
    //cout << "Result: " << riemann_zeta(arg) << endl;
    //return 0;

    coords = (CoordSet *)malloc(w * h * sizeof(CoordSet));


    init_visualiser("Visualiser", 800, 800);

    // Accumulate points. 
    for (i = imin, n = 0; n < h; i += step, n++)
    {
        for (r = rmin, m = 0; m < w; r += step, m++)
        {
            complex<double> s(r, i);
            complex<double> z = riemann_zeta(s);
            int indx = n * w + m;

            coords[indx].coord[0] = real(s);
            coords[indx].coord[1] = imag(s);
            coords[indx].coord[2] = abs(z);
            coords[indx].color = how_calcd;

            // cout << "Zeta of " << s << " = " << zeta << endl;
        }
    }


    display_visualiser(0, w * h, coords);
}

