typedef struct CoordSet
{
    double coord[3];        // XYZ of coord
    int    color;           // Color or other metadata
} CoordSet;

void init_visualiser(const char * title, int wWidth, int wHeight);
void display_visualiser(int how, int w, int h, struct CoordSet* coords);

