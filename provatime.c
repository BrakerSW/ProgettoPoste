#include <time.h>
#include <stdio.h>

int main() {
            time_t data;
            
            struct tm *tm_info;
            time(&data);
            tm_info= localtime(&data);


            printf("Data: %02d/%02d/%d\n ", tm_info->tm_mday, tm_info->tm_mon, tm_info->tm_year+1900);

}