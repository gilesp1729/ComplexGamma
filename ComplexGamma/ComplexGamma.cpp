// Program to play with the Riemann zeta function and its analytic continuation.

#include <iostream>
#include <complex>
extern "C"
{
#include "visualiser.h"
    void __stdcall generate(void);
}

#define STEPS 50

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
complex<double> series_zeta(complex<double> s, int * steps, double* residual)
{
    complex<double> zeta(0, 0);

   // cout << "Series zeta(" << s << ")" << endl;
    for (int i = 1; i < STEPS; i++) 
    {
        double di = (double)i;
        complex<double> term = 1.0 / pow(di, s);

        // cout << di << term << endl;
        zeta += term;
        *residual = abs(term) / abs(zeta);
        if (*residual < 1.0e-7)
        {
           // cout << "Converged in " << i << " iterations" << endl;
            *steps = i;
            return zeta;
        }
    }
    *steps = STEPS;
    return zeta;
}


// Dirichlet zeta function. Supposed to converge for real(s) > 0.
complex<double> dirichlet_zeta(complex<double> s, int* steps, double* residual)
{
    complex<double> zeta(0, 0);

    // cout << "Dirichlet zeta(" << s << ")" << endl;
    for (int i = 1; i < STEPS; i++)
    {
        double di = (double)i;
        complex<double> term = (di / pow(di + 1, s)) - ((di - s) / pow(di, s));

        // cout << di << term << endl;
        zeta += term / (s - 1.0);
        *residual = abs(term) / abs(zeta);
        if (*residual < 1.0e-7)
        {
            // cout << "Converged in " << i << " iterations" << endl;
            *steps = i;
            return zeta;
        }
    }
    *steps = STEPS;
    return zeta;
}


// Riemann zeta function using functional equation to compute zeta for real(s) <= 0.0.
// Note: blows up for s = 0 (since it tries to calculate series_zeta(1.0) )
// For 0.0 < real(s) <= 1.0, we use the Dirichlet zeta function.
// Also output: the number of steps required to converge to 10^-7, or STEPS if it
// did not reach convergence. Residual is also output (as abs(term) / abs(zeta))
complex<double> riemann_zeta(complex<double> s, int *steps, double *residual)
{
    complex<double> z1;

    if (real(s) >= 1.0)   // need to do something at exactly s = 1.0 + 0.0i
        return series_zeta(s, steps, residual); // -dirichlet_zeta(s, result);  // test if they agree

    if (real(s) > 0.0)
        return dirichlet_zeta(s, steps, residual);   // should agree with series_zeta if real(s) > 1
    
    // functional equation
    z1 = riemann_zeta(1.0 - s, steps, residual);
    return pow(2.0, s) * pow(pi, s - 1.0) * sin(pi * s / 2.0) * cgamma(1.0 - s) * z1;

   // z1 *= pow(2.0, s);
   // z1 *= pow(pi, s - 1.0);
   // z1 *= sin(pi * s / 2.0);
   // z1 *= cgamma(1.0 - s);
   // return z1;
}

// Globals for control.
double step = 0.1;
double imin = -10.0;
double rmin = -30.0;
double imax = 10.0;
double rmax = 5.0;
int w, h;
CoordSet* coords;


int main()
{
    
    w = (rmax - rmin) / step;
    h = (imax - imin) / step;

    coords = (CoordSet *)malloc(w * h * sizeof(CoordSet));
    memset(coords, 0, w * h * sizeof(CoordSet));

    init_visualiser("Visualiser", 800, 800, NULL);
    generate();
    display_visualiser(0, w, h, coords);
}

void __stdcall generate(void)
{
    int m, n;
    double i, r;

    // Accumulate points. 
    for (i = imin, n = 0; n < h; i += step, n++)
    {
        for (r = rmin, m = 0; m < w; r += step, m++)
        {
            complex<double> s(r, i);
            int indx = n * w + m;

            complex<double> z = riemann_zeta(s, &coords[indx].steps, &coords[indx].residual);

            coords[indx].coord[0] = real(s);
            coords[indx].coord[1] = imag(s);
            coords[indx].coord[2] = abs(z);
        }
    }
}


