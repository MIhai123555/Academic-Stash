#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lodepng.h"

typedef struct queue_node{
    unsigned int x, y;
    struct queue_node *next;
} queue_node;

typedef struct{
    queue_node *beg_p, *end_p;
} queue;

typedef struct{
    double red, green, blue, 
    brightness, deviation;
    unsigned int is_visited, is_marked;
} pixel;

void init_queue(queue *q_p){
    q_p->beg_p = q_p->end_p = NULL;
    return;
}

unsigned char* load_png(const char* filename, unsigned int* width, unsigned int* height) 
{
  unsigned char* image = NULL;
  int error = lodepng_decode32_file(&image, width, height, filename);
  if(error != 0) {
    printf("error %u: %s\n", error, lodepng_error_text(error)); 
  }
  return (image);
}

pixel** create_pixel_mat(unsigned char *picture, unsigned int width, unsigned int height, unsigned int left, unsigned int right,  unsigned int top, unsigned int bottom){
    unsigned int i, j, section_height = bottom - top, section_width = right - left;
    double red, green, blue, color_mean, deviation, brightness;
    pixel **pixel_mat = (pixel**)malloc(sizeof(pixel*)*section_height);
    for (i=0; i<section_height; i++){
        pixel_mat[i] = (pixel*)malloc(sizeof(pixel)*section_width);
    }
    for (i=0; i<section_height; i++){
        for (j=0; j<section_width; j++){
            red = ((double)picture[4*(i*width + left + j)])/255.0; //normalized red
            green = ((double)picture[4*(i*width + left + j) + 1])/255.0; //normalized green
            blue = ((double)picture[4*(i*width + left + j) + 2])/255.0; //normalized blue
            color_mean = (red + green + blue)/3.0;
            deviation = sqrt((pow(red - color_mean, 2.0) + pow(green - color_mean, 2.0) + pow(blue - color_mean, 2.0))/3.0);
            pixel_mat[i][j].is_visited = 0; pixel_mat[i][j].is_marked = 0;
            pixel_mat[i][j].red = red; pixel_mat[i][j].green = green; pixel_mat[i][j].blue = blue;
            pixel_mat[i][j].brightness = color_mean; pixel_mat[i][j].deviation = deviation;
        }
    }
    return pixel_mat;
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

double get_deviation(pixel new_px, pixel cur_px){
    return sqrt((pow(new_px.red - cur_px.red, 2.0) + pow(new_px.green - cur_px.green, 2.0) + pow(new_px.blue - cur_px.blue, 2.0))/3.0);
}

void try_to_push_pixel(pixel **pixel_mat, queue *queue_ptr, unsigned int cur_x, unsigned int cur_y, int shift_x, int shift_y, 
                       unsigned int section_width, unsigned int section_height, unsigned int *blob_size, unsigned int blob_number,
                       double brightness_threshold, double deviation_threshold){
    if ((cur_x + shift_x < 0) || (cur_x + shift_x > section_width - 1) || (cur_y + shift_y < 0) || (cur_y + shift_y > section_height - 1)) return;
    pixel new_pixel = pixel_mat[cur_y + shift_y][cur_x + shift_x],
          cur_pixel = pixel_mat[cur_y][cur_x];
    if (!new_pixel.is_visited && new_pixel.brightness > brightness_threshold && new_pixel.deviation < deviation_threshold){
        push_queue(queue_ptr, cur_x + shift_x, cur_y + shift_y);
        pixel_mat[cur_y + shift_y][cur_x + shift_x].is_marked = 1;
        blob_size[blob_number]++;
    }
    pixel_mat[cur_y + shift_y][cur_x + shift_x].is_visited = 1;
    return;
}

void scoop_the_blob(pixel **pixel_mat, double brightness_threshold, double deviation_threshold,
                    unsigned int *blob_size, unsigned int blob_number, unsigned int x, unsigned int y, unsigned int section_width, unsigned int section_height){
    queue Queue, *queue_ptr = &Queue;
    unsigned int cur_x, cur_y;
    pixel cur_pixel;
    init_queue(queue_ptr);
    push_queue(queue_ptr, x, y);
    pixel_mat[y][x].is_visited = 1;
    blob_size[blob_number]++;
    while (!is_empty_queue(queue_ptr)){
        pop_queue(queue_ptr, &cur_x, &cur_y);
        try_to_push_pixel(pixel_mat, queue_ptr, cur_x, cur_y, 0, -1, section_width, section_height, blob_size, blob_number, brightness_threshold, deviation_threshold);
        //try_to_push_pixel(pixel_mat, queue_ptr, cur_x, cur_y, 1, -1, section_width, section_height, blob_size, blob_number, brightness_threshold, deviation_threshol);
        try_to_push_pixel(pixel_mat, queue_ptr, cur_x, cur_y, 1, 0, section_width, section_height, blob_size, blob_number, brightness_threshold, deviation_threshold);
        //try_to_push_pixel(pixel_mat, queue_ptr, cur_x, cur_y, 1, 1, section_width, section_height, blob_size, blob_number, brightness_threshold, deviation_threshold);
        try_to_push_pixel(pixel_mat, queue_ptr, cur_x, cur_y, 0, 1, section_width, section_height, blob_size, blob_number, brightness_threshold, deviation_threshold);
       // try_to_push_pixel(pixel_mat, queue_ptr, cur_x, cur_y, -1, 1, section_width, section_height, blob_size, blob_number, brightness_threshold, deviation_threshold);
        try_to_push_pixel(pixel_mat, queue_ptr, cur_x, cur_y, -1, 0, section_width, section_height, blob_size, blob_number, brightness_threshold, deviation_threshold);
        //try_to_push_pixel(pixel_mat, queue_ptr, cur_x, cur_y, -1, -1, section_width, section_height, blob_size, blob_number, brightness_threshold, deviation_threshold);
    }
    return;
}

void write_png(const char* file_name, const unsigned char* image, unsigned width, unsigned height){
    unsigned char* buffer_png;
    size_t png_size;
    int error = lodepng_encode32(&buffer_png, &png_size, image, width, height);
    if(!error){
        lodepng_save_file(buffer_png, png_size, file_name);
    }
    else{
        printf("error %u: %s\n", error, lodepng_error_text(error));
    }
    free(buffer_png);
    return;
}

void create_marked_png(pixel **pixel_mat, unsigned int section_width, unsigned int section_height){
    int i, j;
    const char *marked_file_name = "marked.png";
    unsigned char *marked_png = (unsigned char*)malloc(4*section_width*section_height);
    for (i=0; i<section_height; i++){
        for (j=0; j<section_width; j++){
            if (pixel_mat[i][j].is_marked){
                marked_png[4*(i*section_width + j)] = 255;
                marked_png[4*(i*section_width + j)+1] = 255;
                marked_png[4*(i*section_width + j)+2] = 255;
                marked_png[4*(i*section_width + j)+3] = 255;
            }
            else{
                marked_png[4*(i*section_width + j)] = 0;
                marked_png[4*(i*section_width + j)+1] = 0;
                marked_png[4*(i*section_width + j)+2] = 0;
                marked_png[4*(i*section_width + j)+3] = 255;
            }
        }
    }
    write_png(marked_file_name, marked_png, section_width, section_height);
    free(marked_png);
    return;
}

int main(void){
    unsigned int width, height, i, j, section, blob_number = 0, ship_count = 0, size_threshold = 5, section_number = 1,
    *blob_size = (int*)calloc(10000, sizeof(int));
    double  brightness_thresholds[section_number], deviation_thresholds[section_number], brightness_threshold, deviation_threshold;
    const char *file_name = "ships_big.png", *border_file_name = "borders.txt";
    unsigned char *picture = load_png(file_name, &width, &height);
    int left_borders[section_number], right_borders[section_number], top_borders[section_number], bottom_borders[section_number],
        left, right, top, bottom, section_height, section_width;
    pixel **pixel_mat, cur_pixel;

    FILE *border_file_ptr = fopen(border_file_name, "r");
    for (section = 0; section<section_number; section++){
        fscanf(border_file_ptr, "%d%d%d%d%lf%lf", left_borders + section, right_borders + section, top_borders + section, bottom_borders + section, brightness_thresholds + section, deviation_thresholds + section);
        if (left_borders[section] < 0) left_borders[section] = 0;
        if (right_borders[section] < 0) right_borders[section] = width;
        if (top_borders[section] < 0) top_borders[section] = 0;
        if (bottom_borders[section] < 0) bottom_borders[section] = height;
        if (brightness_thresholds[section] < 0.0) brightness_thresholds[section] = 0.2;
        if (deviation_thresholds[section] < 0.0) deviation_thresholds[section] = 0.1;
    }
    fclose(border_file_ptr);

    for (section = 0; section < section_number; section++){
        left = left_borders[section]; right = right_borders[section]; top = top_borders[section]; bottom = bottom_borders[section];
        brightness_threshold = brightness_thresholds[section]; deviation_threshold = deviation_thresholds[section];
        section_height = bottom - top; section_width = right - left;
        pixel_mat = create_pixel_mat(picture, width, height, left, right, top, bottom);
        for (i=0; i<section_height; i++){
            for (j=0; j<section_width; j++){
                cur_pixel = pixel_mat[i][j];
                if (cur_pixel.brightness > brightness_threshold && cur_pixel.deviation < deviation_threshold && !cur_pixel.is_visited){
                    scoop_the_blob(pixel_mat, brightness_threshold, deviation_threshold,
                                blob_size, blob_number, j, i, section_width, section_height);
                    blob_number++;
                }
                pixel_mat[i][j].is_visited = 1;
            }
        }
        create_marked_png(pixel_mat, section_width, section_height);
        for (i=0; i<section_height; i++) free(pixel_mat[i]);
        free(pixel_mat);
    }
    for (i=0; i<blob_number; i++){
        printf("%d ", blob_size[i]);
    }
    printf("\n");
    free(blob_size);
    return 0;
}