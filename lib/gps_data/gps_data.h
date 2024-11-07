#ifndef PARSER_H
#define PARSER_H

typedef struct {
    float latitude;
    float longitude;
} GPSData;

int parse_gps_data(const char *response, GPSData *gps_data);

#endif // PARSER_H