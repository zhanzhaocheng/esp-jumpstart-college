/* Network Configuration

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <sys/param.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_event_loop.h>

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/def.h"
#include <lwip/netdb.h>

#include "driver/adc.h"
#include "esp_adc_cal.h"

#include <nvs_flash.h>
#include <conn_mgr_prov.h>
#include <conn_mgr_prov_mode_ble.h>
#include <conn_mgr_prov_mode_softap.h>


#include "app_priv.h"

static const char *TAG = "app_main";

#define CONFIG_EXAMPLE_IPV4
static int sock;
static bool isTcp_server_task_live=0;
//static char sIpAdd[]="000.000.000.000";
static u16_t iPortNum=6666;
volatile static bool bStartRecordflag=0;

#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   10          //Multisampling

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_6;     //GPIO34 if ADC1, GPIO14 if ADC2 ,ADC1 channel 6 is GPIO34
static const adc_atten_t atten = ADC_ATTEN_DB_11; //11dB attenuation (ADC_ATTEN_DB_11) gives full-scale voltage 3.9V
static const adc_unit_t unit = ADC_UNIT_1;
#define SAMPLE_RATE 1000
#define SEND_RATE 10  //send times each second
#define BUFFER_SIZE_TX  (SAMPLE_RATE/SEND_RATE) //number each time
#define DELAY_TIME_MS (1000/SAMPLE_RATE)
uint32_t tx_buffer[BUFFER_SIZE_TX];

static void check_efuse()
{
    //Check TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }

    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
}

static void measure_task(void *pvParameters)
{ //send data

    //Check if Two Point or Vref are burned into eFuse
    check_efuse();

    //Configure ADC
    if (unit == ADC_UNIT_1) {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(channel, atten);
    } else {
        adc2_config_channel_atten((adc2_channel_t)channel, atten);
    }

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    //print_char_val_type(val_type);
    uint32_t voltage;
    static uint32_t bufferCurrentIndex=0;
    //Continuously sample ADC1
    while (1) {
     if (bStartRecordflag == 1)
     {
        uint32_t adc_reading = 0;
        //Multisampling
        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            if (unit == ADC_UNIT_1) {
                adc_reading += adc1_get_raw((adc1_channel_t)channel);
            } else {
                int raw;
                adc2_get_raw((adc2_channel_t)channel, ADC_WIDTH_BIT_12, &raw);
                adc_reading += raw;
            }
        }
        adc_reading /= NO_OF_SAMPLES;
        //Convert adc_reading to voltage in mV
        voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        if(bufferCurrentIndex<BUFFER_SIZE_TX)
        {
            tx_buffer[bufferCurrentIndex]=voltage;
            bufferCurrentIndex++;
        }
        else if(bufferCurrentIndex==BUFFER_SIZE_TX){   
            bufferCurrentIndex=0;
            int err = send(sock, tx_buffer, sizeof(tx_buffer), 0);
            tx_buffer[bufferCurrentIndex]=voltage; 
            printf("Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);
            // if (err < 0) {
            //     ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
            //     //break;
            // }
        }
        else bufferCurrentIndex=0; 
        vTaskDelay(DELAY_TIME_MS/portTICK_PERIOD_MS);
     }
     else vTaskDelay(500/portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

static void tcp_server_task(void *pvParameters)
{
    char rx_buffer[128];
    char addr_str[128];
    int addr_family;
    int ip_protocol;
    TaskHandle_t hld_measure_task;
    while (1) {
#ifdef CONFIG_EXAMPLE_IPV4
        struct sockaddr_in destAddr;
        destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(iPortNum);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);
#else // IPV6
        struct sockaddr_in6 destAddr;
        bzero(&destAddr.sin6_addr.un, sizeof(destAddr.sin6_addr.un));
        destAddr.sin6_family = AF_INET6;
        destAddr.sin6_port = htons(PORT);
        addr_family = AF_INET6;
        ip_protocol = IPPROTO_IPV6;
        inet6_ntoa_r(destAddr.sin6_addr, addr_str, sizeof(addr_str) - 1);
#endif

        int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
        if (listen_sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created");

        int err = bind(listen_sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
        if (err != 0) {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket binded");

        err = listen(listen_sock, 1);
        if (err != 0) {
            ESP_LOGE(TAG, "Error occured during listen: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket listening");

        struct sockaddr_in6 sourceAddr; // Large enough for both IPv4 or IPv6
        socklen_t addrLen = sizeof(sourceAddr);
        sock = accept(listen_sock, (struct sockaddr *)&sourceAddr, &addrLen);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket accepted");

        while (1) {
            isTcp_server_task_live=1;
            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            // Error occured during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recv failed: errno %d", errno);
                break;
            }
            // Connection closed
            else if (len == 0) {
                //ESP_LOGI(TAG, "Check your recv");
                //break;
            }
            // Data received
            else {
                // Get the sender's ip address as string
                // if (sourceAddr.sin6_family == PF_INET) {
                //     inet_ntoa_r(((struct sockaddr_in *)&sourceAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
                // } else if (sourceAddr.sin6_family == PF_INET6) {
                //     inet6_ntoa_r(sourceAddr.sin6_addr, addr_str, sizeof(addr_str) - 1);
                // }
                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                //ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
                ESP_LOGI(TAG, "%s", rx_buffer);
                if(strcmp(rx_buffer,"start")==0){
                    if(bStartRecordflag==0){
                    bStartRecordflag=1;
                    ESP_LOGE(TAG, "Start Recording");
                    set_led_state(TX_ING);
                    xTaskCreate(measure_task, "measure_task", 8192, NULL, 6, &hld_measure_task);
                    }
                }
                else if(strcmp(rx_buffer,"stop")==0){
                    if (bStartRecordflag==1)
                    {
                    bStartRecordflag=0;
                    ESP_LOGE(TAG, "Stop Recording");
                    set_led_state(PRIV_DONE);
                    vTaskDelete(hld_measure_task);
                    }
                    
                }
            }

        }

        if (sock != -1) {
            //ESP_LOGE(TAG, "Shutting down socket and restarting...");
            //shutdown(sock, 0);
            //close(sock);
        }
    }
    ESP_LOGE(TAG, "Shutting down socket and restarting...");
    shutdown(sock, 0);
    close(sock);
    vTaskDelete(NULL);
    isTcp_server_task_live=0;
}

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    /* Pass event information to conn_mgr_prov so that it can
     * maintain it's internal state depending upon the system event */
    conn_mgr_prov_event_handler(ctx, event);

    /* Global event handler */
    switch (event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        set_led_state(PRIV_DONE);
        //sIpAdd=(char*)malloc(sizeof(char)*strlen(ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip)));
        //memcpy(sIpAdd,ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip),strlen(ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip)+1));
        ESP_LOGI(TAG, "Connected with IP Address:%s Port: %u", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip),iPortNum);
        if(isTcp_server_task_live==0) xTaskCreate(tcp_server_task, "tcp_server", 4096, NULL, 7, NULL);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "Disconnected. Connecting to the AP again...");
        esp_wifi_connect();
        break;
    default:
        break;
    }
    return ESP_OK;
}

/* Custom event handler for catching provisioning manager events */
static esp_err_t prov_event_handler(void *user_data, conn_mgr_prov_cb_event_t event)
{
    esp_err_t ret = ESP_OK;
    switch (event) {
        case CMP_PROV_END:
            /* De-initialize manager once
             * provisioning is finished */
            conn_mgr_prov_deinit();
            break;
        default:
            break;
    }
    return ret;
}

static void wifi_init_sta()
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_start() );
}

static void get_device_service_name(char *service_name, size_t max)
{
    uint8_t eth_mac[6];
    const char *ssid_prefix = "PROV_";
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    snprintf(service_name, max, "%s%02X%02X%02X",
             ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);
}


void app_main()
{
    app_driver_init();

    /* Initialize NVS partition */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        /* NVS partition was truncated
         * and needs to be erased */
        ret = nvs_flash_erase();

        /* Retry nvs_flash_init */
        ret |= nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init NVS");
        return;
    }

    /* Initialize TCP/IP and the event loop */
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL) );

    /* Configuration for the provisioning manager */
    conn_mgr_prov_config_t config = {
        /* What is the Provisioning Scheme that we want ?
         * conn_mgr_prov_scheme_softap or conn_mgr_prov_scheme_ble */
        .scheme = conn_mgr_prov_scheme_ble,

        /* Any default scheme specific event handler that you would
         * like to choose. Since our example application requires
         * neither BT nor BLE, we can choose to release the associated
         * memory once provisioning is complete, or not needed
         * (in case when device is already provisioned). Choosing
         * appropriate scheme specific event handler allows the manager
         * to take care of this automatically. This can be set to
         * CMP_EVENT_HANDLER_NONE when using conn_mgr_prov_scheme_softap*/
        .scheme_event_handler = CMP_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM,

        /* Do we want an application specific handler be executed on
         * various provisioning related events */
        .app_event_handler = {
            .event_cb = prov_event_handler,
            .user_data = NULL
        }
    };

    /* Initialize provisioning manager with the
     * configuration parameters set above */
    if (conn_mgr_prov_init(config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize provisioning manager");
        set_led_state(PRIV_FAIL);
        return;
    }
 
    bool provisioned = false;
    set_led_state(PRIV_WAIT);
    /* Let's find out if the device is provisioned */
    if (conn_mgr_prov_is_provisioned(&provisioned) != ESP_OK) {
        ESP_LOGE(TAG, "Error getting device provisioning state");
        set_led_state(PRIV_FAIL);
        return;
    }

    /* If device is not yet provisioned start provisioning service */
    if (!provisioned) {
        ESP_LOGI(TAG, "Starting provisioning");
        set_led_state(PRIV_ING);
        /* What is the Device Service Name that we want
         * This translates to :
         *     - WiFi SSID when scheme is conn_mgr_prov_scheme_softap
         *     - device name when scheme is conn_mgr_prov_scheme_ble
         */
        char service_name[12];
        get_device_service_name(service_name, sizeof(service_name));

        /* What is the security level that we want (0 or 1):
         *      - Security 0 is simply plain text communication.
         *      - Security 1 is secure communication which consists of secure handshake
         *          using X25519 key exchange and proof of possession (pop) and AES-CTR
         *          for encryption/decryption of messages.
         */
        int security = 1;

        /* Do we want a proof-of-possession (ignored if Security 0 is selected):
         *      - this should be a string with length > 0
         *      - NULL if not used
         */
        const char *pop = "abcd1234";

        /* What is the service key (could be NULL)
         * This translates to :
         *     - WiFi password when scheme is conn_mgr_prov_scheme_softap
         *     - simply ignored when scheme is conn_mgr_prov_scheme_ble
         */
        const char *service_key = NULL;

        /* Start provisioning service */
        if(ESP_OK!=conn_mgr_prov_start_provisioning(security, pop, service_name, service_key))
        {
            set_led_state(PRIV_FAIL);
        }
        
        

        /* Uncomment the following to wait for the provisioning to finish and then release
         * the resources of the manager. Since in this case de-initialization is triggered
         * by the configured prov_event_handler(), we don't need to call the following */
        // conn_mgr_prov_wait();
        // conn_mgr_prov_deinit();
    } else {
        ESP_LOGI(TAG, "Already provisioned, starting station");
        /* Start wifi station */
        wifi_init_sta();

        /* We don't need the manager as device is already provisioned,
         * so let's release it's resources */
        conn_mgr_prov_deinit();
    }
}
