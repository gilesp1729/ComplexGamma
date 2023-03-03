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
complex<double> series_zeta(complex<double> s, int *result)
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
            *result = 1;
            return zeta;
        }
    }
    *result = 0;
    return zeta;
}


// Dirichlet zeta function. Supposed to converge for real(s) > 0.
complex<double> dirichlet_zeta(complex<double> s, int* result)
{
    complex<double> zeta(0, 0);

    // cout << "Dirichlet zeta(" << s << ")" << endl;
    for (int i = 1; i < 100; i++)  // limit of 100 terms, arbitrarily
    {
        double di = (double)i;
        complex<double> term = (di / pow(di + 1, s)) - ((di - s) / pow(di, s));

        // cout << i << term << endl;
        zeta += term;
        if (abs(term) < 1.0e-7 * abs(zeta))
        {
            // cout << "Converged in " << i << " iterations" << endl;
            *result = 1;
            return zeta / (s - 1.0);
        }
    }
    *result = 0;
    return zeta / (s - 1.0);
}


// Riemann zeta function using functional equation to compute zeta for real(s) <= 0.0.
// Note: blows up for s = 0 (since it tries to calculate series_zeta(1.0) )
// For 0.0 < real(s) <= 1.0, we use the Dirichlet zeta function.
// Result output: 1 if converged, 0 otherwise.
complex<double> riemann_zeta(complex<double> s, int *result)
{
    complex<double> s1;

    if (real(s) >= 1.0)   // need to do something at exactly s = 1.0 + 0.0i
        return series_zeta(s, result); // -dirichlet_zeta(s, result);  // test if they agree

    if (real(s) > 0.0)
        return dirichlet_zeta(s, result);   // should agree with series_zeta if real(s) > 1
    
    s1 = riemann_zeta(1.0 - s, result);
    return pow(2.0, s) * pow(pi, s - 1.0) * sin(pi * s / 2.0) * cgamma(s) * s1;
}



int main()
{
    double step = 0.05;
    double imin = -5.0;
    double rmin = -5.0;
    double imax = 5.0;
    double rmax = 5.0;
    
    int w = (rmax - rmin) / step;
    int h = (imax - imin) / step;
    int m, n;
    double i, r;
    CoordSet* coords;
    int result;

    coords = (CoordSet *)malloc(w * h * sizeof(CoordSet));

    init_visualiser("Visualiser", 800, 800);

    // Accumulate points. 
    for (i = imin, n = 0; n < h; i += step, n++)
    {
        for (r = rmin, m = 0; m < w; r += step, m++)
        {
            complex<double> s(r, i);
            complex<double> z = riemann_zeta(s, &result);
            int indx = n * w + m;

            coords[indx].coord[0] = real(s);
            coords[indx].coord[1] = imag(s);
            coords[indx].coord[2] = abs(z);
            coords[indx].color = result;

            // cout << "Zeta of " << s << " = " << zeta << endl;
        }
    }

    display_visualiser(0, w, h, coords);
}

