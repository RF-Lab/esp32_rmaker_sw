
#include <esp_wifi.h>


float getRSSI(void)
{
    wifi_ap_record_t info ;

    if(!esp_wifi_sta_get_ap_info(&info)) 
    {
        return (info.rssi) ;
    }
    return (0) ;
}
