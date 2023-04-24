#ifdef ESP8266
#ifdef USE_ENERGY_SENSOR
/*********************************************************************************************\
 * Energy for ESP8266 and legacy ESP32 with max three phases/channels using Settings from flash
\*********************************************************************************************/

#define XDRV_99                99
#define XSNS_99                99

#define HF_ENERGY_INFLUXDB_URL               "http://influxdb.foresight-next.dfki.dev"
#define HF_ENERGY_INFLUXDB_TOKEN             "P8UayR2UykLP4exJbZi23UxmgaI3V3y1YG8iDVmBfJphCL6ndk-tNsjXD4iaCpQDKxsCupabaKE6nY89qTZUhA=="
#define HF_ENERGY_INFLUXDB_ORG               "influxdata"
#define HF_ENERGY_INFLUXDB_BUCKET            "plugs-testing"
#define HF_ENERGY_INFLUXDB_MAX_BATCH_SIZE    20
#define HF_ENERGY_INFLUXDB_WRITE_BUFFER_SIZE 60

#include <InfluxDbClient.h>

InfluxDBClient client(HF_ENERGY_INFLUXDB_URL, HF_ENERGY_INFLUXDB_ORG, 
  HF_ENERGY_INFLUXDB_BUCKET, HF_ENERGY_INFLUXDB_TOKEN);

void HfEnergyEvery250ms(void) {
  Point measurement("energy");
  measurement.addField("current", Energy->current[0]);
  measurement.addField("voltage", Energy->voltage[0]);
  measurement.addField("active_power", Energy->active_power[0]);
  measurement.setTime(getUtcNow().c_str());

  AddLog(LOG_LEVEL_INFO, PSTR("Influxdb: %s"), client.pointToLineProtocol(measurement).c_str());

  client.writePoint(measurement);
}

/*********************************************************************************************\
 * Commands
\*********************************************************************************************/

void HfEnergyDrvInit(void) {
  client.setWriteOptions(WriteOptions()
    .writePrecision(WritePrecision::MS)
    .batchSize(HF_ENERGY_INFLUXDB_MAX_BATCH_SIZE)
    .bufferSize(HF_ENERGY_INFLUXDB_WRITE_BUFFER_SIZE));
}

void HfEnergySnsInit(void)
{
  XnrgCall(FUNC_INIT);
}

/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

bool Xdrv99(uint32_t function)
{
  bool result = false;

  if (FUNC_PRE_INIT == function) {
    HfEnergyDrvInit();
  }

  else if (TasmotaGlobal.energy_driver) {
    switch (function) {
      case FUNC_LOOP:
      case FUNC_SLEEP_LOOP:
        XnrgCall(FUNC_LOOP);
        break;
      case FUNC_EVERY_250_MSECOND:
        if (TasmotaGlobal.uptime > 4) {
          XnrgCall(FUNC_EVERY_250_MSECOND);
        }
        break;
      case FUNC_EVERY_SECOND:
        XnrgCall(FUNC_EVERY_SECOND);
        break;
      case FUNC_SERIAL:
        result = XnrgCall(FUNC_SERIAL);
        break;
      case FUNC_NETWORK_UP:
        XnrgCall(FUNC_NETWORK_UP);
        break;
      case FUNC_NETWORK_DOWN:
        XnrgCall(FUNC_NETWORK_DOWN);
        break;
    }
  }
  return result;
}

bool Xsns99(uint32_t function)
{
  bool result = false;

  if (TasmotaGlobal.energy_driver) {
    switch (function) {
      case FUNC_EVERY_250_MSECOND:
        HfEnergyEvery250ms();
        break;
      case FUNC_INIT:
        HfEnergySnsInit();
        break;
    }
  }
  return result;
}

#endif  // USE_ENERGY_SENSOR
#endif  // ESP8266
