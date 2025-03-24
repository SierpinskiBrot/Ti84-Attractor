#include <graphx.h>
#include <time.h>
#include <math.h>
#include <keypadc.h>

#include "fixedpoint.h"
#include "vector.h"
#include "color.h"




Fixed24 cosLUT[628];
Fixed24 sinLUT[628];

uint16_t NUM_POINTS = 400;
uint16_t POINTS_SHOWN = 20;

Fixed24 SCR_LEFT(-40);
Fixed24 SCR_RIGHT(40);
Fixed24 SCR_TOP(40);
Fixed24 SCR_BOTTOM(-40);

Vec3 origin(0, 0, 0);
Vec3 xLine(20, 0, 0);
Vec3 yLine(0, 20, 0);
Vec3 zLine(0, 0, 20);

Vec3 cameraPos(0, 0, 0);

Fixed24 inverseHorizontalSize(320.0F / (SCR_RIGHT - SCR_LEFT).floor());
Fixed24 inverseVerticalSize(240.0F / (SCR_TOP - SCR_BOTTOM).floor());

Fixed24 rho(28);
Fixed24 sigma(10);
Fixed24 beta(8.0F / 3.0F);
Fixed24 dt(0.02F);


Vec3 rotateY(const Vec3 p, uint16_t angle) {
    return Vec3(
        p.x * cosLUT[angle] + (p.z * sinLUT[angle]), 
        p.y, 
        p.x * -sinLUT[angle] + (p.z * cosLUT[angle]));
}
Vec3 rotateX(Vec3 p, uint16_t angle) {
    return Vec3(
        p.x, 
        p.y * cosLUT[angle] - (p.z * sinLUT[angle]), 
        p.y * sinLUT[angle] + (p.z * cosLUT[angle]));
}

int getScrX(const Vec3 p) {
    return Fixed24((p.x - SCR_LEFT) * inverseHorizontalSize).floor();
}
int getScrY(const Vec3 p) {
    return Fixed24(Fixed24(240) - (p.y - SCR_BOTTOM) * inverseVerticalSize).floor();
}
Vec3 translate(const Vec3 p) {
    return p + cameraPos;
}


int main(void)
{
    
    //Create the cos lookup table
    for (int i = 0; i < 628; i++) cosLUT[i] = Fixed24(cosf(i / 100.0F));
    for (int i = 0; i < 628; i++) sinLUT[i] = Fixed24(sinf(i / 100.0F));
    
    gfx_Begin();
    gfx_SetDrawBuffer();


    //Create palette starting from red going to green
    for (int i = 0; i < 255; i++) {
        gfx_palette[i] = gfx_RGBTo1555(255 - i, i, 0);
    }
    //Black for background
    gfx_palette[0] = gfx_RGBTo1555(0, 0, 0);
    gfx_palette[254] = gfx_RGBTo1555(255, 255, 255);

    //Generate the points in their starting positions
    Vec3 points[NUM_POINTS];
    for (int i = 0; i < NUM_POINTS; i++) {
        points[i].x = Fixed24(i * 0.02F + 1);
        points[i].y = Fixed24(i * 0.02F + 1);
        points[i].z = Fixed24(i * 0.02F + 1);
    }
    gfx_SetTextFGColor(254);
    
    //Initial rotation
    int16_t yRotation = 31;
    int16_t xRotation = 31;

    bool pointMode = true;
    bool clearScreenEachFrame = true;
    bool drawAxes = true;

    bool second;
    bool prev2nd;

    bool trace;
    bool prevTrace;

    bool graph;
    bool prevGraph;

    bool del;
    bool prevDel;

    bool stat;
    bool prevStat;


    gfx_ZeroScreen();
    bool running = true;
    while (running) {
        clock_t start = clock();
        
        
        
        
        
        //Handle inputs
        kb_Scan();
        
        if (kb_Data[7] == kb_Up) cameraPos.y += Fixed24(1);
        if (kb_Data[7] == kb_Down) cameraPos.y -= Fixed24(1);
        if (kb_Data[7] == kb_Left) cameraPos.x -= Fixed24(1);
        if (kb_Data[7] == kb_Right) cameraPos.x += Fixed24(1);

        second = kb_Data[1] == kb_2nd;
        if(second && !prev2nd) pointMode = !pointMode;
        prev2nd = second;

        trace = kb_Data[1] == kb_Trace;
        if (trace && !prevTrace) clearScreenEachFrame = !clearScreenEachFrame;
        prevTrace = trace;

        graph = kb_Data[1] == kb_Graph;
        if (graph && !prevGraph) drawAxes = !drawAxes;
        prevGraph = graph;
        
        del = kb_Data[1] == kb_Del;
        if (del && POINTS_SHOWN < NUM_POINTS && !prevDel) POINTS_SHOWN += 10;
        prevDel = del;

        stat = kb_Data[4] == kb_Stat;
        if (stat && POINTS_SHOWN >= 10 && !prevStat) POINTS_SHOWN -= 10;
        prevStat = stat;

        if (POINTS_SHOWN < 0) POINTS_SHOWN = 0;
        if (POINTS_SHOWN > NUM_POINTS) POINTS_SHOWN = NUM_POINTS;

        if (kb_Data[4] == kb_8) xRotation -= 5;
        if (kb_Data[4] == kb_2) xRotation += 5;
        if (kb_Data[5] == kb_6) yRotation += 5;
        if (kb_Data[3] == kb_4) yRotation -= 5;
        //Keep rotation between 0 and 2PI
        if (yRotation > 627) yRotation = 0;
        if (yRotation < 0) yRotation = 627;
        if (xRotation > 627) xRotation = 0;
        if (xRotation < 0) xRotation = 627;

        if (clearScreenEachFrame) gfx_ZeroScreen();
        
        if (drawAxes) {
            //----------Draw the cartesian axes----------
            gfx_SetColor(255);

            //Rotate the lines
            Vec3 originRotated = translate(origin);
            Vec3 xLineRotated = translate(rotateX(rotateY(xLine, yRotation), xRotation));
            Vec3 yLineRotated = translate(rotateX(rotateY(yLine, yRotation), xRotation));
            Vec3 zLineRotated = translate(rotateX(rotateY(zLine, yRotation), xRotation));
            //Get the ends of the rotated lines in screen coordinates
            int originScreen[2]{ getScrX(originRotated),getScrY(originRotated) };
            int xLineScreen[2]{ getScrX(xLineRotated),getScrY(xLineRotated) };
            int yLineScreen[2]{ getScrX(yLineRotated),getScrY(yLineRotated) };
            int zLineScreen[2]{ getScrX(zLineRotated),getScrY(zLineRotated) };
            //Draw the labels for the axes
            gfx_SetTextXY(getScrX(xLineRotated), getScrY(xLineRotated) - 10); gfx_PrintString("X");
            gfx_SetTextXY(getScrX(yLineRotated), getScrY(yLineRotated) - 10); gfx_PrintString("Y");
            gfx_SetTextXY(getScrX(zLineRotated), getScrY(zLineRotated) - 10); gfx_PrintString("Z");
            //Draw the axes
            gfx_Line(originScreen[0], originScreen[1], xLineScreen[0], xLineScreen[1]);
            gfx_Line(originScreen[0], originScreen[1], yLineScreen[0], yLineScreen[1]);
            gfx_Line(originScreen[0], originScreen[1], zLineScreen[0], zLineScreen[1]);
        }
        
        
        for (uint16_t i = 0; i < POINTS_SHOWN; i++) {
            Vec3 old = points[i];

            points[i].x += dt * sigma * (old.y - old.x);
            points[i].y += dt * (old.x * (rho - old.z) - old.y);
            points[i].z += dt * (old.x * old.y - beta * old.z);
            
            Vec3 rotated = translate(rotateX(rotateY(points[i], yRotation),xRotation));
            int screenX = getScrX(rotated);
            int screenY = getScrY(rotated);

            
            if (screenX <= 319 && screenX >= 0 && screenY <= 239 && screenY >= 0) {

                int16_t color = Fixed24(points[i].z * Fixed24(5)).floor();
                
                if (color < 1) color = 1;
                if (color > 253) color = 253;
                
                gfx_SetColor(color);

                if(pointMode) gfx_SetPixel(screenX,screenY);

                if (!pointMode) {
                    Vec3 rotatedOld = translate(rotateX(rotateY(old, yRotation), xRotation));
                    int screenXold = getScrX(rotatedOld);
                    int screenYold = getScrY(rotatedOld);
                    gfx_Line(screenXold, screenYold, screenX, screenY);
                }

            }
            
        }
        
        
        
        gfx_SetTextFGColor(254);
        gfx_SetTextXY(0, 200);
        gfx_PrintString("Points shown: ");
        gfx_PrintInt(POINTS_SHOWN,0);

        gfx_SetTextXY(0, 210);
        gfx_PrintString("Draw axes: ");
        if (drawAxes) gfx_PrintString("On");
        else gfx_PrintString("Off");

        gfx_SetTextXY(0, 220);
        gfx_PrintString("Draw mode: ");
        if (pointMode) gfx_PrintString("Points");
        else gfx_PrintString("Lines");
        
        
       
        clock_t end = clock();
        int elapsed_time = (int)((end - start));

        gfx_SetTextXY(0, 230);
        gfx_PrintString("Frame time: ");
        gfx_PrintInt(elapsed_time, 0);

        

        

        
        

        gfx_SwapDraw();

        if (kb_Data[6] == kb_Enter) running = false;

    }

    //End graphics drawing
    gfx_End();

    
}
