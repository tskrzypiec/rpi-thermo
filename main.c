#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <wiringPi.h>
#include <stdint.h>
#include <ctype.h>
#define URL_SIZE_MAX 120
static void post_event(char *url);
float curr_temp = 0;
#define MAXTIMINGS      85
#define DHTPIN          7
int dht11_dat[5] = {0, 0, 0, 0, 0};


float read_dht11_dat(void)
{
    static float result = 0.0f;
    uint8_t laststate = HIGH;
    uint8_t counter = 0;
    uint8_t j = 0, i;

    dht11_dat[0] = dht11_dat[1] = dht11_dat[2] = dht11_dat[3] = dht11_dat[4] = 0;

    /* pull pin down for 18 milliseconds */
    pinMode(DHTPIN, OUTPUT);
    digitalWrite(DHTPIN, LOW);
    delay(18);
    /* then pull it up for 40 microseconds */
    digitalWrite(DHTPIN, HIGH);
    delayMicroseconds(40);
    /* prepare to read the pin */
    pinMode(DHTPIN, INPUT);

    /* detect change and read data */
    for (i = 0; i < MAXTIMINGS; i++)
    {
        counter = 0;
        while (digitalRead(DHTPIN) == laststate) {
            counter++;
            delayMicroseconds(1);
            if (counter == 255) {
                break;
            }
        }
        laststate = digitalRead(DHTPIN);

        if (counter == 255)
            break;

        /* ignore first 3 transitions */
        if ((i >= 4) && (i % 2 == 0))
        {
            /* shove each bit into the storage bytes */
            dht11_dat[j / 8] <<= 1;
            if (counter > 16)
                dht11_dat[j / 8] |= 1;
            j++;
        }
    }

    /*
     * check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
        printf( "Temperature = %d.%d *C (%.1f *F)\n",
        dht11_dat[2], dht11_dat[3], f );
        * print it out if data is good
     */
    if ((j >= 40) &&
        (dht11_dat[4] == ((dht11_dat[0] + dht11_dat[1] + dht11_dat[2] + dht11_dat[3]) & 0xFF)))
    {
        printf("Temperature = %d.%d *C\n", dht11_dat[2], dht11_dat[3]);
        result = (float) dht11_dat[2]; /* todoa: add fract part to the result */
    }
    else
    {
        //printf("dbg: Data not good, skip\n"); /* uncomnent for debug */
        // data is not good, keep precious result
    }

    return result;
}

static void post_event(char *url)
{
    CURL *curl = curl_easy_init();
    CURLcode curl_res = CURLE_FAILED_INIT;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        //curl_easy_setopt(curl, CURLOPT_NOBODY, 1); /* do not print page content */

        curl_res = curl_easy_perform(curl);
        printf("\n"); /* print new line after page content */

        if (CURLE_OK != curl_res) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(curl_res));
        }
        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "curl: %s\n", curl_easy_strerror(curl_res));
    }
}

int main(int argc, char **argv)
{
    if (wiringPiSetup() == -1)
    {
        exit(1);
    }

    // handle command line args
    if (1 > argc)
    {
          std::cerr << "Usage: " << name << " <option>\n"
              << "Options:\n"
              << "\t--[number_of_degress_you_want_to_start_coolig_from]\n"
              << std::endl;
        exit(EXIT_FAILURE);
    }

    int config_arg = strtol(argv[1], NULL, 10);
    printf("dbg arg: %d\n", config_arg);

    // build full url with the secret key
    char *socket_evt_on = "https://maker.ifttt.com/trigger/gniazdko_on/with/key/";
    char *socket_evt_off = "https://maker.ifttt.com/trigger/gniazdko_off/with/key/";
    char full_url_socket_on[URL_SIZE_MAX] = {0};
    char full_url_socket_off[URL_SIZE_MAX] = {0};
    snprintf(full_url_socket_on, sizeof(full_url_socket_on), "%s%s", socket_evt_on, getenv("KEY"));
    snprintf(full_url_socket_off, sizeof(full_url_socket_off), "%s%s", socket_evt_off, getenv("KEY"));

    // main program
    float read_temp = 0.0f;
    float temp_down = 5.0f; /* magic number */

    while (1)
    {
        read_temp = read_dht11_dat();

        if (config_arg < read_temp)
        {
            post_event(full_url_socket_on);
        }

        if (((float)config_arg-temp_down) > read_temp)
        {
            post_event(full_url_socket_off);
        }
    }

    return 0;
}
