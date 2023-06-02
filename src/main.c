#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <getopt.h>

#define NO_ERROR 0
#define ARG_ERROR 1
#define OPEN_IMAGE_ERROR 2
#define KEY_ERROR 3
#define COLOR_ERROR 4
#define CIRCLE_RADIUS_ERROR 5
#define CIRCLE_SQUARE_ERROR 6
#define RGB_FILTER_ERROR 7
#define INFORMATION_ERROR 8
#define FLAG_BEFORE_ERROR 9
#define DIVIDE_ERROR 10
#define FIND_ALL_RECTANGLE_ERROR 11
#define BMP_ERROR 12

char* error_msg[] =
{
    "Ошибок нет",
    "Неверно введены данные",
    "Изображение не найдено",
    "Неверно введен ключ или его аргументы отсутствуют",
    "Неизвестный цвет",
    "По вашим данным невозможно построить круг. Перепроверьте входные параметры",
    "Неверно введены координаты квадрата",
    "Неверные данные для Rgb-фильтра",
    "Файл отсутствует или указан позже флага взаимодействия с ним",
    "Флаг, который вы указали для работы с изображением, необходимо указать после нужного для него ключа",
    "Неверные данные для разделения изображения",
    "Неверные данные для обводки прямоугольников или их поиска",
    "Неподдерживаемый формат BMP-файла"
};


#pragma pack (push, 1) //
typedef struct BitmapFileHeader
{
    unsigned short signature; // позволяет определить тип файла
    unsigned int filesize; // размер файла
    unsigned short reserved1; // должен быть 0
    unsigned short reserved2; // должен быть 0
    unsigned int pixelArrOffset; // показывает, где начинается сам битовый массив относительно начала файла, который и описывает картинку
} BitmapFileHeader;

typedef struct BitmapInfoHeader
{
    unsigned int headerSize; // размер структуры
    unsigned int width; // ширина картинки в пикселях
    unsigned int height; // высота картинки в пикселях
    unsigned short planes; // задает количество плоскостей, пока всегда устанавливается в 1
    unsigned short bitsPerPixel; // количество бит на один пиксель
    unsigned int compression; // тип сжатия, обычно несжатые картинки
    unsigned int imageSize; // размер картинки в байтах; если изображение несжато, то здесь должен быть записан 0
    unsigned int xPixelsPerMeter; // горизонатальное разрешение (пиксель на метр)
    unsigned int yPixelsPerMeter; // вертикальное разрешение (пиксель на метр)
    unsigned int colorsInColorTable; // количество используемых цветов из таблицы, если записан 0, то в растре используется максимально возможное количество цветов
    unsigned int importantColorCount; // количество важных цветов, если это значение 0, то все цвета считаются важными
} BitmapInfoHeader;

typedef struct Rgb
{
    unsigned char b;
    unsigned char g;
    unsigned char r;
} Rgb;
#pragma pack(pop) //

void printFileHeader (BitmapFileHeader header){ // выводит соответсвующую информацию структуры BitmapFileHeader
    printf("signature:\t%x (%hu)\n", header.signature, header.signature);
    printf("filesize:\t%x (%u)\n", header.filesize, header.filesize);
	printf("reserved1:\t%x (%hu)\n", header.reserved1, header.reserved1);
	printf("reserved2:\t%x (%hu)\n", header.reserved2, header.reserved2);
	printf("pixelArrOffset:\t%x (%u)\n", header.pixelArrOffset, header.pixelArrOffset);
}

void printInfoHeader(BitmapInfoHeader header){ // выводит соответствующую инфформацию по файлу bmp (структура BitmapInfoHeader)
	printf("headerSize:\t%x (%u)\n", header.headerSize, header.headerSize);
	printf("width:     \t%x (%u)\n", header.width, header.width);
	printf("height:    \t%x (%u)\n", header.height, header.height);
	printf("planes:    \t%x (%hu)\n", header.planes, header.planes);
	printf("bitsPerPixel:\t%x (%hu)\n", header.bitsPerPixel, header.bitsPerPixel);
	printf("compression:\t%x (%u)\n", header.compression, header.compression);
	printf("imageSize:\t%x (%u)\n", header.imageSize, header.imageSize);
	printf("xPixelsPerMeter:\t%x (%u)\n", header.xPixelsPerMeter, header.xPixelsPerMeter);
	printf("yPixelsPerMeter:\t%x (%u)\n", header.yPixelsPerMeter, header.yPixelsPerMeter);
	printf("colorsInColorTable:\t%x (%u)\n", header.colorsInColorTable, header.colorsInColorTable);
	printf("importantColorCount:\t%x (%u)\n", header.importantColorCount, header.importantColorCount);
}


int CheckRgbStruct(Rgb color_1, Rgb color_2){
    if (color_1.r == color_2.r && color_1.g == color_2.g && color_1.b == color_2.b){
        return 1;
    }
    else
        return 0;
}

void createArrPosition(Rgb** arr, Rgb find_color, int H, int W, int** position){
    for (int i = 0; i<H; i++){
        for (int j = 0; j<W; j++){
            if (CheckRgbStruct(find_color, arr[i][j])){
                position[i][j] = 1;
            }
            else
                position[i][j] = 0;
        }
    }
}

typedef struct Point{
	int X; // координата вдоль ширины
	int Y; // координата вдоль высоты
} Point;

typedef struct Figure{
	Point top1; // верхняя левая вершина
	Point top2; // верхняя правая вершина
	Point top3; // нижняя правая вершина
	Point top4; // нижняя левая вершина
    Point maxTop; //координаты наибольшего x и y
    Point minTop; // координаты наименьшего x и y
} Figure;

void fill(int y, int x, Figure* border, int** position, int H, int W)
{
    Point el = {x, y};

    Point* value = malloc(sizeof(Point)*H*W*2);
    int count = 0;
    int top_index = -1;
    value[top_index+1] = el; top_index++; count++; //push
    position[el.Y][el.X] = 2;
    while(count != 0)
    {
        el = value[top_index]; top_index--; count--; //pop
        position[el.Y][el.X] = 2; //закрасить пиксел новым цветом

    if((el.Y >= border->top1.Y) && (el.X <= border->top1.X)) { //поиск ЛВ
        border->top1.X = el.X;
        border->top1.Y = el.Y;
    }

    if((el.Y >= border->top2.Y) && (el.X >= border->top2.X)) { //поиск ПВ
        border->top2.X = el.X;
        border->top2.Y = el.Y;
    }

    if((el.Y <= border->top3.Y) && (el.X >= border->top3.X)) { //поиск ПН
        border->top3.X = el.X;
        border->top3.Y = el.Y;
    }

    if((el.Y <= border->top4.Y) && (el.X <= border->top4.X)) { //поиск ЛН
        border->top4.X = el.X;
        border->top4.Y = el.Y;
    }

    if ((el.Y <= border->minTop.Y)){ //поиск самой низкой высоты
        border->minTop.Y = el.Y;
    }

    if ((el.X <= border->minTop.X)){ //поиск самой левой границы
        border->minTop.X = el.X;
    }

    if ((el.Y >= border->maxTop.Y)){ //поиск самой высокой высоты
        border->maxTop.Y = el.Y;
    }

    if ((el.X >= border->maxTop.X)){ //поиск самой правой границы
        border->maxTop.X = el.X;
    }

        if(el.X>0 && position[el.Y][el.X-1]==1){
            el.X = el.X-1;
            el.Y = el.Y;
            value[top_index+1] = el; top_index++; count++; //push
        }
        if(el.X<(W-1) && position[el.Y][el.X+1]==1){
            el.X = el.X+1;
            el.Y = el.Y;
            value[top_index+1] = el; top_index++; count++; //push
        }
        if(el.Y>0 && position[el.Y-1][el.X]==1){
            el.X = el.X;
            el.Y = el.Y-1;
            value[top_index+1] = el; top_index++; count++; //push
        }

        if(el.Y<(H-1) && position[el.Y+1][el.X]==1){
            el.X = el.X;
            el.Y = el.Y+1;
            value[top_index+1] = el; top_index++; count++; //push
        }
    }
    free(value);
    return;
}

void search_tops(int y, int x, Figure* border, int** position, int H, int W){
    position[y][x] = 2; //пройденную точку 1 помечаем 2
    if((y >= border->top1.Y) && (x <= border->top1.X)) { //поиск ЛВ
        border->top1.X = x;
        border->top1.Y = y;
    }
    if((y >= border->top2.Y) && (x >= border->top2.X)) { //поиск ПВ
        border->top2.X = x;
        border->top2.Y = y;
    }
    if((y <= border->top3.Y) && (x >= border->top3.X)) { //поиск ПН
        border->top3.X = x;
        border->top3.Y = y;
    }
    if((y <= border->top4.Y) && (x <= border->top4.X)) { //поиск ЛН
        border->top4.X = x;
        border->top4.Y = y;
    }

    if ((y <= border->minTop.Y)){ //поиск самой низкой высоты
        border->minTop.Y = y;
    }
    if ((x <= border->minTop.X)){ //поиск самой левой границы
        border->minTop.X = x;
    }
    if ((y >= border->maxTop.Y)){ //поиск самой высокой высоты
        border->maxTop.Y = y;
    }
    if ((x >= border->maxTop.X)){ //поиск самой правой границы
        border->maxTop.X = x;
    }

    //рекурсивно проходимся по соседним точкам, проверяя, что они входят в границы изображения
    if((x < (W-1)) && (position[y][x+1] == 1)){
        search_tops(y,x+1,border,position, H, W);
    }
    if((y < (H-1)) && (position[y+1][x] == 1)){
        search_tops(y+1,x,border,position, H, W);
    }
    if((x > 0) && (position[y][x-1] == 1)){
        search_tops(y,x-1,border,position, H, W);
    }
    if((y > 0) && (position[y-1][x] == 1)){
        search_tops(y-1,x,border,position, H, W);
    }

    return;
}

long long int count_area(Figure border, int** position){
    long long int S = 0; //инициализация площади
    for (int i = border.minTop.Y; i <  border.maxTop.Y; i++){ //проходим по описанному прямоугольнику (вершины определены макс и мин границами фигуры)
        for (int j = border.minTop.X; j < border.maxTop.X; j++){
            if (position[i][j] == 2){ //если точка была пройдена в search_tops
                S++; // то учитываем ее как точку внутри фигуры
            }
        }
    }
    return S;
}

int check_rectangle(Figure border, int S){
    if (border.top1.X != border.minTop.X || border.top1.Y != border.maxTop.Y){ //вершина ЛВ не соответствует крайним границам фигуры
        return 0;
    }
    else if (border.top2.X != border.maxTop.X || border.top2.Y != border.maxTop.Y){ // -//-
        return 0;
    }
    else if (border.top3.X != border.maxTop.X || border.top3.Y != border.minTop.Y){ // -//-
        return 0;
    }
    else if (border.top4.X != border.minTop.X || border.top4.Y != border.minTop.Y){ // -//-
        return 0;
    }
    else if (S != ((border.maxTop.X - border.minTop.X) * (border.maxTop.Y - border.minTop.Y))){ //площадь, полученная исходя из вершин прямоугольника сверяется с площадью пройденной фигуры
        return 0;
    }
    else if (S == 0){
        return 0;
    }
    else
        return 1; //все Ок. Прямоугольник

}

void hightlightRectangle(Rgb** arr, Rgb color_line, int thickness, Figure border, int H, int W){
    int xl = border.minTop.X - thickness; // левая граница описанного прямоугольника
    int xr = border.maxTop.X + thickness; // правая -//-
    int yu = border.maxTop.Y + thickness; //верхняя граница описанного прямоугольника
    int yl = border.minTop.Y - thickness; // нижняя граница описанного прямоугольника
    for (int i = yl; i<yu+1; i++){ //пробег по описанному прямоугольнику
        for (int j = xl; j<xr+1; j++){
            if (i < 0 || j < 0 || i >= H || j >= W) //на случай выхода за границу обводим ту часть, которая входит в рамки изображения
                continue;
            if (((i > border.maxTop.Y) || (i < border.minTop.Y)) || ((j < border.minTop.X) || (j > border.maxTop.X))){ //закрашивание только той области, которая лежит между описанным и исходным прямоугольником
                arr[i][j] = color_line;
            }
        }
    }
}

//Complete
void findAllRectangle(int** position, int H, int W, Rgb** arr, Rgb color_line, int thickness){
    long long int S = 0; //площадь фигуры
    Figure border; //границы фигуры
    int flag_rectangle = 0; //флажок для обозначения прямоугольника
    int counter = 0; //для учета случая, что ни один прямоугольник не найден
    printf("\n");
    printf("Координаты левого-верхнего и правого-нижнего угла найденных прямоугольников:\n");
    for (int i = 0; i < H; i++) {
		for (int j = 0; j < W; j++) {
			if(position[i][j] == 1) {	//если нужная заливка
                border.top1.X = j; border.top1.Y = i;
                border.top2.X = j; border.top2.Y = i;
                border.top3.X = j; border.top3.Y = i;
                border.top4.X = j; border.top4.Y = i;
                border.minTop.X = j; border.minTop.Y = i;
                border.maxTop.X = j; border.maxTop.Y = i; //инициализация всех границ (для точки)
                fill(i, j, &border, position, H, W); //поиск всех границ и вершин
                S = count_area(border, position); //подсчет площади
                flag_rectangle = check_rectangle(border, S); //проверка фигуры на то, что он прямоугольник
                if (flag_rectangle == 1){
                    counter++;
                    printf("Левый-верхний:%d %d, Правый-нижний:%d %d\n", border.top1.X, border.top1.Y, border.top3.X, border.top3.Y); //вывод левого верхнего, правого нижнего угла, площади прямоугольника
                    hightlightRectangle(arr, color_line, thickness, border, H, W); //обводка прямоугольника
                }
                S = 0; //зануление площади для след фигуры
			}
		}
	}
    if (counter == 0){
        printf("Прямоугольников не найдено\n");
    }
}


// Complete
void rgb_filter(char* component, int value, int H, int W, Rgb** arr){ // устанавливает заданную компоненту в значение 0 или 255
    char c = component[0];
    switch(c){
        case 'r':
            for (int i = 0; i<H; i++){
                for (int j = 0; j<W; j++){
                    arr[i][j].r = value;
                }
            }
            break;

        case 'g':
            for (int i = 0; i<H; i++){
                for (int j = 0; j<W; j++){
                    arr[i][j].g = value;
                }
            }
            break;

        case 'b':
            for (int i = 0; i<H; i++){
                for (int j = 0; j<W; j++){
                    arr[i][j].b = value;
                }
            }
            break;
    }
}


// Complete
void DrawCircle(int x0, int y0, int radius, Rgb** arr, int H, int W, int thickness, int fill, Rgb line_color, Rgb fill_color){ //Рисование окружности с заполнением и толщиной
    int xl = x0 + radius; int yl = y0 - radius; // координаты левого верхнего угла квадрата, описанного около искомой окружности
    int xr = x0 - radius; int yr = y0 + radius; // -//- правый нижний угол
    for(int i=xr+1; i<xl; i++) {
        for(int j=yl+1; j<yr; j++) {
            if (i < 0 || j < 0 || i>=H || j>=W)
                continue;
            int point_1 = (y0-j)*(y0-j);
            int point_2 = (x0-i)*(x0-i);
            int Circle_in = (radius-thickness) > 0 ? radius-thickness: 0;
            if ((point_1 + point_2 <= radius*radius) && (point_1 + point_2 >= (Circle_in*Circle_in))) {
                arr[i][j] = line_color;
            }
            else if ((point_1 + point_2 <= radius*radius) && fill){
                arr[i][j] = fill_color;
            }
        }
    }
}

// Complete
void pre_DrawCircle(int xl, int yl, int xr, int yr, Rgb** arr, int H, int W, int thickness, int fill, Rgb line_color, Rgb fill_color){
    int x0 = (xl + xr) / 2;
    int y0 = (yl + yr) / 2;
    int radius = yr - y0;
    DrawCircle(x0, y0, radius, arr, H, W, thickness, fill, line_color, fill_color);
}

typedef struct ChangedFile{
    Rgb** arr;
    BitmapFileHeader* bmfh;
    BitmapInfoHeader* bmif;
} ChangedFile;

//Complete
ChangedFile* Divide(Rgb** arr, BitmapFileHeader* bmfh, BitmapInfoHeader* bmif, int thickness, int N, int M, Rgb color_line){
    int oldHeight =  bmif->height;
    int oldWidth =  bmif->width;

    BitmapFileHeader* Newbmfh =  (BitmapFileHeader*) malloc(sizeof(BitmapFileHeader));
    BitmapInfoHeader* Newbmif =  (BitmapInfoHeader*) malloc(sizeof(BitmapInfoHeader));
    Newbmfh = bmfh;
    Newbmif = bmif;

    Newbmif->height += thickness*(N - 1);
    Newbmif->width += thickness*(M - 1);
    int newHeight = Newbmif->height;
    int newWidth = Newbmif->width;

    Rgb** newArr = malloc(newHeight*sizeof(Rgb*) + (4 - (newHeight * sizeof(Rgb)) % 4) % 4);
    for (int i = 0; i < newHeight; i++){
        newArr[i] = malloc(newWidth*sizeof(Rgb) + (4 - (newWidth*sizeof(Rgb)) % 4) % 4);
    }

    for (int i = 0; i < oldHeight; i++){
        for (int j = 0; j < oldWidth; j++){
            newArr[i][j] = arr[i][j];
        }
    }

    for (int partH = 1; partH < N; partH++){
        for (int i = oldHeight - 1 + thickness*partH; i > (partH*oldHeight)/N + thickness*(partH - 1); i--){
            for (int j = 0; j < oldWidth; j++){
                if (i+thickness < newHeight){
                    newArr[i+thickness][j] = newArr[i][j];
                }
            }
        }

        for (int i = 0; i<thickness; i++){
            for (int j = 0; j<newWidth; j++){
                newArr[(partH*oldHeight)/N + thickness*(partH - 1) + 1 + i][j] = color_line;
            }
        }
    }

    for (int partW = 1; partW < M; partW++){
        for (int j = oldWidth - 1 + thickness*partW; j > (partW*oldWidth)/M + thickness*(partW-1); j--){
            for (int i = 0; i<newHeight; i++){
                if (j + thickness < newWidth){
                    newArr[i][j + thickness] = newArr[i][j];
                }
            }
        }

        for (int j = 0; j < thickness; j++){
            for (int i = 0; i < newHeight; i++){
                newArr[i][(partW*oldWidth)/M + thickness*(partW - 1) + 1 + j] = color_line;
            }
        }
    }

    ChangedFile* newPicture = (ChangedFile*) malloc(sizeof(ChangedFile));
    newPicture->arr = newArr;
    newPicture->bmfh = Newbmfh;
    newPicture->bmif = Newbmif;
    return newPicture;
}

//Считывание файла
Rgb** ReadFile(char* name_file, BitmapFileHeader* bmfh, BitmapInfoHeader* bmif, int* err){
    int r = NO_ERROR;
    FILE* f = fopen(name_file, "rb");
    Rgb** arr;

    if (f != NULL){
        fread(bmfh, 1, sizeof(BitmapFileHeader), f);
        fread(bmif, 1, sizeof(BitmapInfoHeader), f);

        unsigned int H = bmif->height; // высота изображения в пикселях
        unsigned int W = bmif->width; // -//- ширина


        if (bmif->bitsPerPixel != 24 || bmif->colorsInColorTable){
            r = BMP_ERROR;
        }

        if (!r){
            int trashSize = bmfh->pixelArrOffset - sizeof(BitmapFileHeader) - sizeof(BitmapInfoHeader);
            char* trash = malloc(sizeof(char) * trashSize);
            fread(trash, 1, trashSize, f);
            free(trash);

            arr = malloc(H*sizeof(Rgb*));
            for (int i = 0; i < H; i++){ // считывание картинки построчно и ее сохранение в двумерный массив, выделенный выше (для каждой строчки пикселей картинки)
                arr[i] = malloc(W * sizeof(Rgb) + (4 - (W*sizeof(Rgb))%4)%4);
                fread(arr[i], 1, W * sizeof(Rgb) + (4 - (W*sizeof(Rgb))%4)%4, f);
            }
        }
    }
    else
    {
        r = OPEN_IMAGE_ERROR;
    }

    *err = r;

    return arr;
}

//Вывод файла
void WriteFile(BitmapFileHeader* bmfh, BitmapInfoHeader* bmif, Rgb** arr){
    FILE *ff = fopen("out.bmp", "wb"); // открытие файла на запись
    unsigned int H = bmif->height;
    unsigned int W = bmif->width;
    fwrite(bmfh, 1, sizeof(BitmapFileHeader), ff);
    fwrite(bmif, 1, sizeof(BitmapInfoHeader), ff);

    int trashSize = bmfh->pixelArrOffset - sizeof(BitmapFileHeader) - sizeof(BitmapInfoHeader);
    char* addition = calloc(trashSize, sizeof(char));
    fwrite(&addition, 1, trashSize, ff);
    free(addition);

    unsigned int w = W * sizeof(Rgb) + (4 - (W*sizeof(Rgb))%4)%4;
    for (int i = 0; i<H; i++){ // вывод картинки в out.bmp
        fwrite(arr[i], 1, w, ff);
        free(arr[i]);
    }
    free(arr);
    fclose(ff);
}

//Выбор цвета
int ChooseColor(Rgb* color, char* s_color){
    int err = NO_ERROR;
    Rgb black = {0, 0, 0};
    Rgb pirple = {255, 0, 128};
    Rgb light_blue = {250, 206, 135};
    Rgb blue = {255,0,0};
    Rgb green = {0, 255, 0};
    Rgb yellow = {0,255,255};
    Rgb orange = {0,102,255};
    Rgb red = {0,0,255};
    Rgb gray = {190,190,190};
    Rgb white = {255,255,255};

    if (!strcmp(s_color, "black")) *color = black; else
    if (!strcmp(s_color, "pirple")) *color = pirple; else
    if (!strcmp(s_color, "blue")) *color = blue; else
    if (!strcmp(s_color, "light_blue")) *color = light_blue; else
    if (!strcmp(s_color, "green")) *color = green; else
    if (!strcmp(s_color, "yellow")) *color = yellow; else
    if (!strcmp(s_color, "orange")) *color = orange; else
    if (!strcmp(s_color, "red")) *color = red; else
    if (!strcmp(s_color, "gray")) *color = gray; else
    if (!strcmp(s_color, "white")) *color = white; else
    err = COLOR_ERROR;

    return err;
}

//Помощь
void PrintHelp(){
    printf("Help:\n");
    printf("\t--open <name>\t-o: Считать изображение\n");
    printf("\t--circle\t-c: Нарисовать окружность по следующим условиям:\n");
    printf("\t\t--radius <радиус> <высота x> <ширина y> <толщина> <цвет линии>\t-r: по радиусу и координатам\n");
    printf("\t\t--square <высота x1> <ширина y1> (координаты верхнего левого угла) <высота x2> <ширина y2> (координаты нижнего правого угла) <толщина> <цвет линии>\t-q: по описанному квадрату\n");
    printf("\t\t\tДополнительные флаги для работы с окружностью:\n");
    printf("\t\t\t--fill <color>\t-f: Выбор цвета заливки окружности\n");
    printf("\t--filter <компонент Rgb> <значение компонента>\t-p: Rgb-фильтр\n");
    printf("\t--divide <количество частей вдоль высоты N> <количество частей вдоль ширины M> <толщина> <цвет линии>\t-d: Разделяет изображение линиями на N*M частей\n");
    printf("\t--findAllRectangle <цвет заливки> <цвет обводки> <толщина обводки>\t-a: поиск всех прямоугольников заданного цвета и их обводка\n");
    printf("\t--information\t-i: Вывести информацию об считанном файле\n");
    printf("\t--help\t-h: Открыть подсказку\n");
    printf("\t\tДоступны следующие цвета:\n");
    printf("\t\tblack, pirple, blue, light_blue, green, yellow, orange, red, gray, white\n");
    printf("\tПример работы программы:\n");
    printf("\t\t./edit -o your.bmp -c -r 100 300 400 25 pirple\n");
    printf("\t\t./edit -o your.bmp -c -r 100 300 400 25 pirple -f green\n");
    printf("\t\t./edit -o your.bmp --circle --square 400 200 300 300 100 black -f pirple\n");
    printf("\t\t./edit -o your.bmp --circle --square 400 200 300 300 25 black --fill pirple\n");
    printf("\t\t./edit -o your.bmp -p r 255\n");
    printf("\t\t./edit -o your.bmp --divide 2 2 25 yellow\n");
    printf("\t\t./edit --open your.bmp --findAllRectangle black red 25\n");
    printf("\t\t./edit -o your.bmp -i\n");
    printf("\t\t./edit --help\n");
    printf("\tрезультат сохраняется в файл out.bmp\n");
}

//Проверка ключей
int ArgChecks(char* arg){
    int err = NO_ERROR;
    if (!strcmp(arg, "--open") || !strcmp(arg, "-o")){
        err = ARG_ERROR;
    }
    else if (!strcmp(arg, "--circle") || !strcmp(arg, "-с")) {
        err = ARG_ERROR;
    }
    else if (!strcmp(arg, "--radius") || !strcmp(arg, "-r")) {
        err = ARG_ERROR;
    }
    else if (!strcmp(arg, "--square") || !strcmp(arg, "-q")) {
        err = ARG_ERROR;
    }
    else if (!strcmp(arg, "--fill") || !strcmp(arg, "-f")) {
        err = ARG_ERROR;
    }
    else if (!strcmp(arg, "--divide") || !strcmp(arg, "-d")) {
        err = ARG_ERROR;
    }
    else if (!strcmp(arg, "--filter") || !strcmp(arg, "-p")) {
        err = ARG_ERROR;
    }
    else if (!strcmp(arg, "--findAllRectangle") || !strcmp(arg, "-a")){
        err = ARG_ERROR;
    }
    else if (!strcmp(arg, "--information") || !strcmp(arg, "-i")){
        err = ARG_ERROR;
    }
    else if (!strcmp(arg, "--help") || !strcmp(arg, "-h")) {
        err = ARG_ERROR;
    }

    return err;
}

//Проверки входных параметров функций
int CheckArgCircleRadius(int x1, int y1, int radius, int thickness, int H, int W){
    int err = NO_ERROR;
    if (x1 < 0 || x1 >= H) err = CIRCLE_RADIUS_ERROR; else
    if (y1 < 0 || y1 >= W) err = CIRCLE_RADIUS_ERROR; else
    if (radius <= 0) err = CIRCLE_RADIUS_ERROR; else
    if (thickness < 1 || thickness >= W || thickness >= H) err = CIRCLE_RADIUS_ERROR;

    return err;
}

int CheckArgCircleSquare(int xl, int yl, int xr, int yr, int thickness, int H, int W){
    int err = NO_ERROR;
    int dx = xl - xr; //высоты
    int dy = yr - yl; //широты
    if (xl < 0 || xl >= H) err = CIRCLE_SQUARE_ERROR; else
    if (yl < 0 || yl >= W) err = CIRCLE_SQUARE_ERROR; else
    if (xr < 0 || xr >= H) err = CIRCLE_SQUARE_ERROR; else
    if (yr < 0 || yr >= W) err = CIRCLE_SQUARE_ERROR; else
    if (dx <= 0 || dy <= 0) err = CIRCLE_SQUARE_ERROR; else
    if (dx != dy) err = CIRCLE_SQUARE_ERROR; else
    if (thickness < 1 || thickness >= W || thickness >= H) err = CIRCLE_SQUARE_ERROR;

    return err;
}

int CheckArgRgbFilter(char* component, int value){
    int err = NO_ERROR;
    if (strcmp(component, "r") && strcmp(component, "g") && strcmp(component, "b")) err = RGB_FILTER_ERROR; else
    if (value < 0 || value > 255) err = RGB_FILTER_ERROR;

    return err;
}

int CheckArgDivide(int N, int M, int thickness, int H, int W){
    int err = NO_ERROR;
    if (thickness < 1) err = DIVIDE_ERROR; else
    if (N < 1 || N >= H) err = DIVIDE_ERROR; else
    if (M < 1 || M >= W) err = DIVIDE_ERROR;

    return err;
}

int CheckArgFindAllRectangle(int thickness, int H, int W){
    int err = NO_ERROR;
    if (thickness < 1 || thickness >= W || thickness >= H) err = FIND_ALL_RECTANGLE_ERROR;

    return err;
}
//

int main(int argc, char* argv[]){

    int err = NO_ERROR; // отвечает за ошибки

    //
    BitmapFileHeader* bmfh = (BitmapFileHeader*) malloc(sizeof(BitmapFileHeader));
    BitmapInfoHeader* bmif = (BitmapInfoHeader*) malloc(sizeof(BitmapInfoHeader));
    Rgb** arr;
    int x1 = 0;
    int x2 = 0;
    int y1 = 0;
    int y2 = 0;
    int radius = 0;
    char* color;
    Rgb fill_color = {0, 0, 0};
    Rgb line_color = {0, 0, 0};
    int thickness = 1;
    int N = 1;
    int M = 1;
    char* component;
    int value = 0;
    ChangedFile* newFileInfo;

    if (argc == 1){
        err = KEY_ERROR;
    }

    char* name_file = (char*) calloc (40, sizeof(char)); // название считываемого файла

    //opterr = 0;

    char* short_options = "o:cr:q:f:p:d:a:ih?"; //все короткие параметры
    int count = 0; //количество
    int idx = 0; //индекс
    int write_flag = 0; //разрешение на вывод файла
    int fill_flag = 0; // заполнение круга
    int circle_radius_flag = 0; // круг с радиусом
    int circle_square_flag = 0; // круг с описанным квадратом
    int change_file_flag = 0; //изменение в структуре самого изображения

    struct option long_options[] = { // длинные опции
        {"open", 1, NULL, 'o'},
        {"circle", no_argument, NULL, 'c'},
        {"radius", required_argument, NULL, 'r'},
        {"square", required_argument, NULL, 'q'},
        {"fill", required_argument, NULL, 'f'},
        {"filter", required_argument, NULL, 'p'},
        {"divide", required_argument, NULL, 'd'},
        {"findAllRectangle", required_argument, NULL, 'a'},
        {"information", no_argument, NULL, 'i'},
        {"help", 0, NULL, 'h'},
        {NULL, 0, NULL, 0} //?
    };

    int opt = 0; //текущая опция
    int longIndex = -1; //индекс опции
    while (((opt = getopt_long(argc, argv, short_options, long_options, &longIndex)) != -1) && !err){
        switch (opt){
            case 'o': {
                strcat(name_file, optarg);
                arr = ReadFile(name_file, bmfh, bmif, &err);
                write_flag = 1;
                break;
            };
            case 'c': {
                if (write_flag == 0){
                    err = INFORMATION_ERROR;
                    break;
                }

                opt = getopt_long(argc, argv, short_options, long_options, &longIndex);
                if (opt != -1 && opt == 'r'){
                    err = ArgChecks(optarg);
                    if (!err){
                        idx = optind;
                    }

                    if (!err){
                        radius = atoi(argv[idx-1]);
                    }

                    if (!err && (!ArgChecks(argv[idx-1])) && (idx < argc)){
                        x1 = atoi(argv[idx]);
                    }
                    else {
                        err = ARG_ERROR;
                    }
                    if (!err && (!ArgChecks(argv[idx])) && (idx + 1 < argc)){
                        y1 = atoi(argv[idx+1]);
                    }
                    else {
                        err = ARG_ERROR;
                    }

                    if (!err && (!ArgChecks(argv[idx+1])) && (idx + 2 < argc)){
                        thickness = atoi(argv[idx+2]);
                    }
                    else {
                        err = ARG_ERROR;
                    }

                    if (!err && (!ArgChecks(argv[idx+2])) && (idx + 3 < argc)){
                        color = argv[idx+3];
                        err = ChooseColor(&line_color, color);
                    }
                    else {
                        err = ARG_ERROR;
                    }

                    if (!err){
                        circle_radius_flag = 1;
                    }
                }
                else if (opt != -1 && opt == 'q'){
                    err = ArgChecks(optarg);
                    if (!err){
                        idx = optind;
                    }
                    if (!err){
                        x1 = atoi(argv[idx-1]);
                    }

                    if (!err && (!ArgChecks(argv[idx-1])) && (idx < argc)){
                        y1 = atoi(argv[idx]);
                    }
                    else {
                        err = ARG_ERROR;
                    }

                    if (!err && (!ArgChecks(argv[idx])) && (idx + 1 < argc)){
                        x2 = atoi(argv[idx+1]);
                    }
                    else {
                        err = ARG_ERROR;
                    }

                    if (!err && (!ArgChecks(argv[idx+1])) && (idx + 2 < argc)){
                        y2 = atoi(argv[idx+2]);
                    }
                    else {
                        err = ARG_ERROR;
                    }

                    if (!err && (!ArgChecks(argv[idx+2])) && (idx + 3 < argc)){
                        thickness = atoi(argv[idx+3]);
                    }
                    else {
                        err = ARG_ERROR;
                    }

                    if (!err && (!ArgChecks(argv[idx+3])) && (idx + 4 < argc)){
                        color = argv[idx+4];
                        err = ChooseColor(&line_color, color);
                    }
                    else {
                        err = ARG_ERROR;
                    }

                    if (!err){
                        circle_square_flag = 1;
                    }
                }
                else {
                    err = KEY_ERROR;
                }

                if (!err){
                    if ((opt = getopt_long(argc, argv, short_options, long_options, &longIndex)) == 'f'){
                        color = optarg;
                        err = ChooseColor(&fill_color, color);
                        if (!err){
                            fill_flag = 1;
                        }
                    }
                }

                if (!err){
                    if (circle_radius_flag){
                        err = CheckArgCircleRadius(x1, y1, radius, thickness, bmif->height, bmif->width);
                        if (!err){
                            DrawCircle(x1, y1, radius, arr, bmif->height, bmif->width, thickness, fill_flag, line_color, fill_color);
                        }
                    }
                    else if (circle_square_flag){
                        err = CheckArgCircleSquare(x1, y1, x2, y2, thickness, bmif->height, bmif->width);
                        if (!err){
                            pre_DrawCircle(x1, y1, x2, y2, arr, bmif->height, bmif->width, thickness, fill_flag, line_color, fill_color);
                        }
                    }
                }

                circle_radius_flag = 0;
                circle_square_flag = 0;
                fill_flag = 0;
                break;
            };
            case 'p': {
                if (write_flag == 0){
                    err = INFORMATION_ERROR;
                    break;
                }

                err = ArgChecks(optarg);
                if (!err){
                    idx = optind;
                }

                if (!err){
                    component = argv[idx-1];
                }

                if (!err && (!ArgChecks(argv[idx-1])) && (idx < argc)){
                    value = atoi(argv[idx]);
                }
                else {
                    err = ARG_ERROR;
                }

                if (!err){
                    err = CheckArgRgbFilter(component, value);
                    if (!err){
                        rgb_filter(component, value, bmif->height, bmif->width, arr);
                    }
                }

                break;
            };
            case 'd': {
                if (write_flag == 0){
                    err = INFORMATION_ERROR;
                    break;
                }

                err = ArgChecks(optarg);
                if (!err){
                    idx = optind;
                }

                if (!err){
                    N = atoi(argv[idx-1]);
                }

                if (!err && (!ArgChecks(argv[idx-1])) && (idx < argc)){
                    M = atoi(argv[idx]);
                }
                else {
                    err = ARG_ERROR;
                }

                if (!err && (!ArgChecks(argv[idx])) && (idx + 1 < argc)){
                    thickness = atoi(argv[idx+1]);
                }
                else {
                    err = ARG_ERROR;
                }

                if (!err && (!ArgChecks(argv[idx+1])) && (idx + 2 < argc)){
                    color = argv[idx+2];
                    err = ChooseColor(&line_color, color);
                }
                else {
                    err = ARG_ERROR;
                }

                if (!err){
                    err = CheckArgDivide(N, M, thickness, bmif->height, bmif->width);
                    if (!err){
                        newFileInfo = Divide(arr, bmfh, bmif, thickness, N, M, line_color);
                        change_file_flag = 1;
                        bmif = newFileInfo->bmif;
                        bmfh = newFileInfo->bmfh;
                        arr = newFileInfo->arr;
                    }
                }

                break;
            };
            case 'a': {
                if (write_flag == 0){
                    err = INFORMATION_ERROR;
                    break;
                }

                err = ArgChecks(optarg);
                if (!err){
                    idx = optind;
                }
                if (!err){
                    color = argv[idx-1];
                    err = ChooseColor(&fill_color, color);
                }

                if (!err && (!ArgChecks(argv[idx-1])) && (idx < argc)){
                    color = argv[idx];
                    err = ChooseColor(&line_color, color);
                }
                else {
                    err = ARG_ERROR;
                }

                if (!err && (!ArgChecks(argv[idx])) && (idx + 1 < argc)){
                    thickness = atoi(argv[idx+1]);
                }
                else {
                    err = ARG_ERROR;
                }

                if (!err){
                    err = CheckArgFindAllRectangle(thickness, bmif->height, bmif->width);
                    if (!err){
                        int** position = (int**) calloc(bmif->height, sizeof(int*));
                        for (int i = 0; i<bmif->height; i++){
                            position[i] = (int*) calloc(bmif->width, sizeof(int));
                        }
                        createArrPosition(arr, fill_color, bmif->height, bmif->width, position);
                        findAllRectangle(position, bmif->height, bmif->width, arr, line_color, thickness);
                        for (int i = 0; i<bmif->height; i++){
                            free(position[i]);
                        }
                        free(position);
                    }
                }

                break;
            };
            case 'i':{
                if (err != 2 && write_flag == 1){
                    printFileHeader(*bmfh);
                    printInfoHeader(*bmif);
                }
                else {
                    err = INFORMATION_ERROR;
                }

                break;
            };
            case 'f': {
                err = FLAG_BEFORE_ERROR;
                break;
            };
            case 'r': {
                err = FLAG_BEFORE_ERROR;
                break;
            }
            case 'q':{
                err = FLAG_BEFORE_ERROR;
                break;
            }
            case '?':
                err = KEY_ERROR;
                break;
            case 'h':
                PrintHelp();
                break;
        }
    }


    //Вывод измененного файла
    if (write_flag && !err){
        WriteFile(bmfh, bmif, arr);
    }

    //Вывод ошибок
    if (err){
        PrintHelp();
        fprintf(stderr, "Ошибка: %s.\n", error_msg[err]);
    }

    // Очистка памяти
    free(name_file);
    if (newFileInfo)
        free(newFileInfo);
    free(bmfh);
    free(bmif);
    return 0;
}
