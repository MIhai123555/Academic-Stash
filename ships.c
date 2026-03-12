#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <lodepng.h>

typedef struct queue_node{
    int x, y;
    struct queue_node *next;
} queue_node;

typedef struct{
    queue_node *beg_p, *end_p;
} queue;

void init_queue(queue *q_p){
    q_p->beg_p = q_p->end_p = NULL;
    return;
}

void push_queue(queue *q_p, int x, int y){
    queue_node *new = (queue_node*)malloc(sizeof(queue_node));
    new->x = x;
    new->y = y;
    new->next = NULL;
    if (q_p->end_p) q_p->end_p->next = new;
    q_p->end_p = new;
    if (!q_p->beg_p) q_p->beg_p = new;
    return;
}

int is_empty_queue(queue *q_p){
    return (q_p->beg_p == NULL);
}

void pop_queue(queue *q_p, int *x_out, int *y_out){
    queue_node *tmp = q_p->beg_p;
    *x_out = tmp->x;
    *y_out = tmp->y;
    q_p->beg_p = tmp->next;
    if (!q_p->beg_p) q_p->end_p = NULL;
    free(tmp);
    return;
}

void clear_queue(queue *q_p){
    queue_node *tmp = q_p->beg_p;
    while (q_p->beg_p){
        q_p->beg_p = q_p->beg_p->next;
        free(tmp);
        tmp = q_p->beg_p;
    }
    q_p->end_p = NULL;
    return;
}

void print_queue(queue *q_p){
    queue_node *tmp = q_p->beg_p;
    while (tmp){
        printf("(%d %d) ", tmp->x, tmp->y);
        tmp = tmp->next;
    }
    printf("\n");
    return;
}

void scoop_the_blob(double **mat_brightness, int **mat_is_visited, double brightness_threshold, int *blob_size, int blob_number, int x, int y, int width, int height){
    queue Queue, *queue_ptr = &Queue;
    int cur_x, cur_y;
    init_queue(queue_ptr);
    push_queue(queue_ptr, x, y);
    mat_is_visited[x][y] = 1;
    blob_size[blob_number]++;
    while (!is_empty_queue(queue_ptr)){
        pop_queue(queue_ptr, &cur_x, &cur_y);
        //printf("(%d %d)\n", cur_x, cur_y);
        if (cur_y > 0 && !mat_is_visited[cur_x][cur_y-1] && mat_brightness[cur_x][cur_y-1] > brightness_threshold){
            push_queue(queue_ptr, cur_x, cur_y-1);
            blob_size[blob_number]++;
            mat_is_visited[cur_x][cur_y-1] = 1;
        }
        if (cur_y > 0 && cur_x < width-1 && !mat_is_visited[cur_x+1][cur_y-1] && mat_brightness[cur_x+1][cur_y-1] > brightness_threshold){
            push_queue(queue_ptr, cur_x+1, cur_y-1);
            blob_size[blob_number]++;
            mat_is_visited[cur_x+1][cur_y-1] = 1;
        }
        if (cur_x < width-1 && !mat_is_visited[cur_x+1][cur_y] && mat_brightness[cur_x+1][cur_y] > brightness_threshold){
            push_queue(queue_ptr, cur_x+1, cur_y);
            blob_size[blob_number]++;
            mat_is_visited[cur_x+1][cur_y] = 1;
        }
        if (cur_x < width-1 && cur_y < height-1 && !mat_is_visited[cur_x+1][cur_y+1] && mat_brightness[cur_x+1][cur_y+1] > brightness_threshold){
            push_queue(queue_ptr, cur_x+1, cur_y+1);
            blob_size[blob_number]++;
            mat_is_visited[cur_x+1][cur_y+1] = 1;
        }
        if (cur_y < height-1 && !mat_is_visited[cur_x][cur_y+1] && mat_brightness[cur_x][cur_y+1] > brightness_threshold){
            push_queue(queue_ptr, cur_x, cur_y+1);
            blob_size[blob_number]++;
            mat_is_visited[cur_x][cur_y+1] = 1;
        }
        if (cur_x > 0 && cur_y < height-1 && !mat_is_visited[cur_x-1][cur_y+1] && mat_brightness[cur_x-1][cur_y+1] > brightness_threshold){
            push_queue(queue_ptr, cur_x-1, cur_y+1);
            blob_size[blob_number]++;
            mat_is_visited[cur_x-1][cur_y+1] = 1;
        }
        if (cur_x > 0 && !mat_is_visited[cur_x-1][cur_y] && mat_brightness[cur_x-1][cur_y] > brightness_threshold){
            push_queue(queue_ptr, cur_x-1, cur_y);
            blob_size[blob_number]++;
            mat_is_visited[cur_x-1][cur_y] = 1;
        }
    }
    return;
}

int main(void){
    int width, height, i, j, blob_number = 0,
    *blob_size = (int*)calloc(10000, sizeof(int));
    scanf("%d%d", &width, &height);
    double *mat_brightness[height], brightness_threshold = 0.5;
    int *mat_is_visited[height];
    for (i=0; i<height; i++){
        mat_brightness[i] = (double*)malloc(sizeof(double)*width);
        mat_is_visited[i] = (int*)calloc(width, sizeof(int));
    }
    for (i=0; i<height; i++){
        for (j=0; j<width; j++){
            scanf("%lf", mat_brightness[i]+j);
        }
    }
    for (i=0; i<width; i++){
        for (j=0; j<height; j++){
            if (!mat_is_visited[i][j]){
                if (mat_brightness[i][j] > brightness_threshold){
                    scoop_the_blob(mat_brightness, mat_is_visited, brightness_threshold, blob_size, blob_number, i, j, width, height);
                    blob_number++;
                }
                else{
                    mat_is_visited[i][j] = 1;
                }
            }
        }
    }
    for (i=0; i<height; i++){
        for (j=0; j<width; j++){
            printf("%d ", mat_is_visited[i][j]);
        }
        printf("\n");
    }
    for (i=0; i<blob_number; i++){
        printf("%d ", blob_size[i]);
    }
    printf("\n");
    for (i=0; i<width; i++){
        free(mat_brightness[i]);
        free(mat_is_visited[i]);
    }
    free(blob_size);
    return 0;
}