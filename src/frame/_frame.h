
/** array offsets for tuples */
#define LED_X 0
#define LED_Y 1


/** type to define a polygon (using frame cords) */
typedef struct _LedFramePolygon
{
    /* amount of points */
    int points;
    /* array of x,y coordinate tuples */
    LedFrameCord cords[LED_FRAME_POLYPOINTS_MAX][2];
}LedFramePolygon;




void		_frame_polygon_print(LedFramePolygon *p, NftLoglevel l);
void		_frame_polygon_copy(LedFramePolygon *src, LedFramePolygon *dst);
void 		_frame_polygon_get_minimum(LedFramePolygon *p, LedFrameCord *x, LedFrameCord *y);
void 		_frame_polygon_get_maximum(LedFramePolygon *p, LedFrameCord *x, LedFrameCord *y);
