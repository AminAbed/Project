#ifndef COMMENTTRACER_H
#define COMMENTTRACER_H
#include <QString>



#define TRACER_INDEX     "index"
#define PLOT_NAME        "name"
#define PLOT_POSITION    "position"
#define PLOT_COMMENT     "comment"

struct CommentTracer {
    double  position;
    QString name;
    int     index;
    QString comment;
    };


#endif // COMMENTTRACER_H





