#include <Windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "glaux/glaux.h"
#include "Trackbal.h"
#include "visualiser.h"
#include <math.h>

// Basic 3D visualiser. Takes all the GL gubbins out of the calling program.

#define INITIAL_HALFSIZE 100.0f

// Perspective zTrans adjustment
#define ZOOM_Z_SCALE 0.25f

// Initial values of translation components
float xTrans = 0;
float yTrans = 0;
float zTrans = -2.0f * INITIAL_HALFSIZE;
int zoom_delta = 0;
float half_size = INITIAL_HALFSIZE;
int	right_mouseX, right_mouseY;
BOOL	right_mouse = FALSE;

// Stuff to be displayed
int coord_w = 0;
int coord_h = 0;
CoordSet *coord_data;
CoordSet *coord_data;
int coord_how_displayed = 0;
double z_cutoff = 20.0;

// Callback to C++ for generating data
AUXIDLEPROC generate_data = NULL;

static float ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
static float diffuse[] = { 0.5f, 1.0f, 1.0f, 1.0f };
static float position[] = { 90.0f, 90.0f, 150.0f, 0.0f };
static float lmodel_ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static float lmodel_twoside[] = { GL_TRUE };

void
Init(void)
{

    glClearColor(1.0, 1.0, 1.0, 1.0);

    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);


    // Enable alpha blending, so we can have transparency
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);      // alpha blending
    glBlendFunc(GL_ZERO, GL_SRC_COLOR);     // multiply blending.
    glEnable(GL_BLEND);

    // For text annotations, horizontal characters start at 1000, outlines at 2000
    wglUseFontBitmaps(auxGetHDC(), 0, 256, 1000);
#if 0
    hFont = CreateFont(48, 0, 0, 0, FW_DONTCARE, FALSE, TRUE, FALSE, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
    SelectObject(auxGetHDC(), hFont);
    wglUseFontOutlines(auxGetHDC(), 0, 256, 2000, 0, 0, WGL_FONT_POLYGONS, NULL);
#endif

    //glEnable(GL_CULL_FACE);    // don't show back facing faces
}

// Set up frustum and possibly picking matrix. If picking, pass the centre of the
// picking region and its width and height.
void CALLBACK
Position(void)
{
    GLint viewport[4], width, height;
    float h, w, znear, zfar, zoom_factor;
#ifdef DEBUG_POSITION_ZOOM
    char buf[64];
#endif

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glGetIntegerv(GL_VIEWPORT, viewport);
    width = viewport[2];
    height = viewport[3];

    znear = 0.5f * half_size;
    zfar = 50 * half_size;
    if (0 /*view_ortho*/)
    {
        zoom_factor = -0.5f * zTrans / half_size;
#ifdef DEBUG_POSITION_ZOOM
        sprintf_s(buf, 64, "Ortho Ztrans %f zoomf %f\r\n", zTrans, zoom_factor);
        Log(buf);
#endif
        if (width > height)
        {
            w = half_size * zoom_factor * (float)width / height;
            h = half_size * zoom_factor;
            glOrtho(-w, w, -h, h, znear, zfar);
        }
        else
        {
            w = half_size * zoom_factor;
            h = half_size * zoom_factor * (float)height / width;
            glOrtho(-w, w, -h, h, znear, zfar);
        }
        glTranslated(xTrans, yTrans, -2.0f * half_size);
    }
    else
    {
        // In perspective mode, zooming is done more by narrowing the frustum.
        // Don't back off to zTrans, as you always hit the near clipping plane
        zoom_factor = (-0.5f * zTrans / half_size) * ZOOM_Z_SCALE;
#ifdef DEBUG_POSITION_ZOOM
        sprintf_s(buf, 64, "Persp Ztrans %f zoomf %f\r\n", zTrans, zoom_factor);
        Log(buf);
#endif
        if (width > height)
        {
            w = half_size * zoom_factor * (float)width / height;
            h = half_size * zoom_factor;
            glFrustum(-w, w, -h, h, znear, zfar);
        }
        else
        {
            w = half_size * zoom_factor;
            h = half_size * zoom_factor * (float)height / width;
            glFrustum(-w, w, -h, h, znear, zfar);
        }
        glTranslated(xTrans, yTrans, -2.0f * half_size);
    }
}

// Change the proportions of the viewport when window is resized
void CALLBACK
Reshape(int width, int height)
{
    trackball_Resize(width, height);
    glViewport(0, 0, (GLint)width, (GLint)height);
    Position();
}

void CALLBACK
mouse_wheel(AUX_EVENTREC* event)
{
    zoom_delta = event->data[AUX_MOUSESTATUS];
}

void CALLBACK
right_down(AUX_EVENTREC* event)
{
    SetCapture(auxGetHWND());
    right_mouseX = event->data[AUX_MOUSEX];
    right_mouseY = event->data[AUX_MOUSEY];
    right_mouse = TRUE;
}

void CALLBACK
right_up(AUX_EVENTREC* event)
{
    ReleaseCapture();
    right_mouse = FALSE;
}

void
init_visualiser(const char * title, int wWidth, int wHeight, AUXIDLEPROC idle_func)
{
    auxInitPosition(0, 0, wWidth, wHeight);
    auxInitDisplayMode(AUX_DEPTH16 | AUX_RGB | AUX_DOUBLE);
    auxInitWindow("Visualiser", FALSE, FALSE, FALSE);


    auxMouseFunc(AUX_LEFTBUTTON, AUX_MOUSEDOWN, trackball_MouseDown);
    auxMouseFunc(AUX_LEFTBUTTON, AUX_MOUSEUP, trackball_MouseUp);
    auxMouseFunc(AUX_RIGHTBUTTON, AUX_MOUSEDOWN, right_down);
    auxMouseFunc(AUX_RIGHTBUTTON, AUX_MOUSEUP, right_up);
    auxMouseFunc(AUX_MOUSEWHEEL, AUX_MOUSEWHEEL, mouse_wheel);
    auxReshapeFunc(Reshape);
    generate_data = idle_func;

    Init();
    trackball_Init(wWidth, wHeight);
    Position();
}

// Color to GL. Also optionally set material colors for lighting.
// rgb in [0, 1.0]
void color(double r, double g, double b, BOOL lighting)
{
    if (lighting)
    {
        float col[4];

        col[3] = 1.0f;

        // Multiply material color by amb/diff separately
        col[0] = r * ambient[0] * 2.0f;
        col[1] = g * ambient[1] * 2.0f;
        col[2] = b * ambient[2] * 2.0f;
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, col);

        col[0] = r * diffuse[0] * 2.0f;
        col[1] = g * diffuse[1] * 2.0f;
        col[2] = b * diffuse[2] * 2.0f;
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);

      //  glMaterialf(GL_FRONT, GL_SHININESS, materials[mat].shiny);
    }
    else
    {
        glColor3d(r, g, b);
    }
}

#define ABS(n) (((n) < 0) ? -(n) : (n))

void color_by_residual(double resid, BOOL lighting)
{
    // Color by residual. From ~1 to 1.0e-7 --> red to green to blue

    double red, green, blue;

    resid = -log10(resid);
    red = 7 - resid;
    blue = resid;
    green = 7 - 2 * fabs(resid - 3.5);

    color(red / 7, green / 7, blue / 7, lighting);
}

void color_by_height(double ht, BOOL lighting)
{
    // Color by absolute mag (height in z of display) in 0-5

    double red, green, blue;

    // TODO go to black when near zero
    if (ht > 5.0)
        ht = 5.0;
    red = 5 - ht;
    blue = ht;
    green = 5 - 2 * fabs(ht - 2.5);

    color(red / 5, green / 5, blue / 5, lighting);
}

void
cross(double x0, double y0, double z0, double x1, double y1, double z1, double* xc, double* yc, double* zc)
{
    *xc = y0 * z1 - z0 * y1;
    *yc = z0 * x1 - x0 * z1;
    *zc = x0 * y1 - y0 * x1;
}

// Set normal fom triangle. Return FALSE if any points are not valid or too high.
BOOL normal(double p0[3], double p1[3], double p2[3])
{
    double A, B, C, length;

    if (isnan(p0[2]) || p0[2] > z_cutoff)
        return FALSE;
    if (isnan(p1[2]) || p1[2] > z_cutoff)
        return FALSE;
    if (isnan(p2[2]) || p2[2] > z_cutoff)
        return FALSE;

    cross(p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2], p2[0] - p0[0], p2[1] - p0[1], p2[2] - p0[2], &A, &B, &C);
    length = sqrt(A * A + B * B + C * C);
    if (length < 1.0e-7)
        return FALSE;

    A /= length;
    B /= length;
    C /= length;
    glNormal3d(A, B, C);
    return TRUE;
}

void _stdcall Draw(void)
{
    double axis = 100;
    float matRot[4][4];

    // Callback to gneerate data, if required
    if (generate_data)
        generate_data();

    // handle panning with right mouse drag. 
    if (right_mouse)
    {
        POINT pt;

        auxGetMouseLoc(&pt.x, &pt.y);
        if (pt.y != right_mouseY)
        {
            GLint viewport[4], width, height;

            glGetIntegerv(GL_VIEWPORT, viewport);
            width = viewport[2];
            height = viewport[3];
            if (width > height)
            {
                // Y window coords need inverting for GL
                xTrans += 2 * zTrans * (float)(right_mouseX - pt.x) / height;
                yTrans += 2 * zTrans * (float)(pt.y - right_mouseY) / height;
            }
            else
            {
                xTrans += 2 * zTrans * (float)(right_mouseX - pt.x) / width;
                yTrans += 2 * zTrans * (float)(pt.y - right_mouseY) / width;
            }

            Position();
            right_mouseX = pt.x;
            right_mouseY = pt.y;
        }
    }

    // handle zooming
    if (zoom_delta != 0)
    {
        zTrans += 0.001f * half_size * zoom_delta;
        // Don't go too far forward, or we'll hit the near clipping plane
        if (zTrans > -0.1f * half_size)
            zTrans = -0.1f * half_size;
        Position();
        zoom_delta = 0;
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    Position();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    trackball_CalcRotMatrix(matRot);
    glMultMatrixf(&(matRot[0][0]));

    // Draw axes
    glBegin(GL_LINES);
    glColor3d(1.0, 0.4, 0.4);
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(axis, 0.0, 0.0);
    glEnd();
    glBegin(GL_LINES);
    glColor3d(0.4, 1.0, 0.4);
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(0.0, axis, 0.0);
    glEnd();
    glBegin(GL_LINES);
    glColor3d(0.4, 0.4, 1.0);
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(0.0, 0.0, axis);
    glEnd();

    // Draw 3D content
    switch (coord_how_displayed)
    {
    case 0: //as points
    default:
        glBegin(GL_POINTS);
        for (int i = 0; i < coord_w * coord_h; i++)
        {
           // color_by_residual(coord_data[i].residual, FALSE);
            color_by_height(coord_data[i].coord[2], FALSE);
            glVertex3dv(coord_data[i].coord);
        }
        glEnd();
        break;

    case 1: // as triangles
        glEnable(GL_LIGHTING);
        glBegin(GL_TRIANGLES);
        glColor3d(1.0, 0.0, 0.0);
        for (int j = 0; j < coord_h - 1; j++)
        {
            for (int i = 0; i < coord_w - 1; i++)
            {
                int indx = j * coord_w + i;

                // color_by_residual(coord_data[indx].residual, TRUE);
                color_by_height(coord_data[indx].coord[2], TRUE);

                if (normal(coord_data[indx + coord_w].coord, coord_data[indx].coord, coord_data[indx + 1].coord))
                {
                    glVertex3dv(coord_data[indx + coord_w].coord);
                    glVertex3dv(coord_data[indx].coord);
                    glVertex3dv(coord_data[indx + 1].coord);
                }
                if (normal(coord_data[indx + coord_w].coord, coord_data[indx + 1].coord, coord_data[indx + coord_w + 1].coord))
                {
                    glVertex3dv(coord_data[indx + coord_w].coord);
                    glVertex3dv(coord_data[indx + 1].coord);
                    glVertex3dv(coord_data[indx + coord_w + 1].coord);
                }
            }
        }

        glEnd();
        break;
    }

    glFlush();
    auxSwapBuffers();
}


void display_visualiser(int how, int w, int h, CoordSet *coords)
{
    // Store away the coords for visualisation.
    // coords = a [w*h] array of coordinate triplets.
    // how = 0 points, =1 triangles, and so on (TBD)
    coord_w = w;
    coord_h = h;
    coord_data = coords;
    coord_how_displayed = how;

    // Display the data and handle navigation (note: never returns until window destroyed)
    auxIdleFunc(Draw);
    auxMainLoop(Draw);
}