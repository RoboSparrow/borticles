#include <stdlib.h>

#include "log.h"

#include "quadtree/qnode.h"
#include "quadtree/qlist.h"

void qlist_create(QNode *root, struct QList *qa) {
    if (qa->v_count >= QTREE_RENDER_MAX - 1) {
        LOG_INFO_F("reached quad_array limits: %d\n", QTREE_RENDER_MAX);
        return;
    }

    vec2 nw = {
        root->area.x,
        root->area.y,
    };

    vec2 ne = {
        root->area.x + root->area.width,
        root->area.y,
    };

    vec2 se = {
        root->area.x + root->area.width,
        root->area.y + root->area.height,
    };

    vec2 sw = {
        root->area.x,
        root->area.y + root->area.height,
    };

    // nw -> ne
    qa->vertexes[qa->v_count] = nw;
    qa->v_count++;
    qa->vertexes[qa->v_count] = ne;
    qa->v_count++;

    // ne -> se
    qa->vertexes[qa->v_count] = ne;
    qa->v_count++;
    qa->vertexes[qa->v_count] = se;
    qa->v_count++;

    /*
    // se -> sw
    qa->vertexes[qa->v_count] = se;
    qa->v_count++;
    qa->vertexes[qa->v_count] = sw;
    qa->v_count++;

    // sw -> ne
    qa->vertexes[qa->v_count] = sw;
    qa->v_count++;
    qa->vertexes[qa->v_count] = nw;
    qa->v_count++;
    */

    // printf("++++ (%ld) %d, %f, %f, %f, %f\n", qa->count, root->depth, root->area.x, root->area.y, root->area.width, root->area.height);

    if(root->nw != NULL) qlist_create(root->nw, qa);
    if(root->ne != NULL) qlist_create(root->ne, qa);
    if(root->sw != NULL) qlist_create(root->sw, qa);
    if(root->se != NULL) qlist_create(root->se, qa);
}

void qlist_print(FILE *fp, struct QList *quads) {
    if (!fp) {
        return;
    }
    if (!quads) {
        fprintf(fp, "[]\n");
        return;
    }
    fprintf(fp, "[");
    for (size_t i = 0; i < quads->v_count; i++){
        if (i % 4 == 0){
            fprintf(fp, "\n    ");
        }
        fprintf(fp, "{#:%ld, x:%f, y:%f}%s", i, quads->vertexes[i].x, quads->vertexes[i].y, (i < quads->v_count -1) ? ", " : "");
    }
    fprintf(fp, "\n]\n");
}
