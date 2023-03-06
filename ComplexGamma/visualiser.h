typedef struct CoordSet
{
    double coord[3];        // XYZ of coord
    int    steps;           // Terms added so far in series
    double zeta[2];         // Zeta acccumulated so far (as 2 doubles)
    double residual;        // Residual from last term of series, expressed as abs(term) / abs(total)
} CoordSet;

void init_visualiser(const char * title, int wWidth, int wHeight, void _stdcall idle_func(void));
void display_visualiser(int how, int w, int h, struct CoordSet* coords);

