#ifdef ESP8266
#ifdef USE_ENERGY_SENSOR

#include <InfluxDbClient.h>

#define XDRV_99                99
#define XSNS_99                99

#define HF_ENERGY_INFLUXDB_URL               "http://influxdb.foresight-next.dfki.dev"
#define HF_ENERGY_INFLUXDB_TOKEN             "bN1GKYEG1ATEmZuloS7HZKY_EFLcRI8eY5QxUxaWybim0Nnzq6HAsABuLUatpHusbkzYQLUiRCCxdC9Ld183VQ=="
#define HF_ENERGY_INFLUXDB_ORG               "influxdata"
#define HF_ENERGY_INFLUXDB_BUCKET            "plugs-testing"
#define HF_ENERGY_INFLUXDB_MAX_BATCH_SIZE    20
#define HF_ENERGY_INFLUXDB_WRITE_BUFFER_SIZE 60

#define HF_ENERGY_METADATA_ID           "id"
#define HF_ENERGY_METADATA_NAME         "name"
#define HF_ENERGY_METADATA_MODEL        "model"
#define HF_ENERGY_METADATA_MANUFACTURER "manufacturer"
#define HF_ENERGY_METADATA_DESCRIPTION  "description"
#define HF_ENERGY_METADATA_LOCATION     "location"

#define HF_ENERGY_METADATA_NAME_VALUE         "Kettle"
#define HF_ENERGY_METADATA_MODEL_VALUE        "DFKI-KE-666"
#define HF_ENERGY_METADATA_MANUFACTURER_VALUE "DFKI"
#define HF_ENERGY_METADATA_DESCRIPTION_VALUE  "Additional descriptions"
#define HF_ENERGY_METADATA_LOCATION_VALUE     "Living Room"

#define D_PRFX_HF_ENERGY                       "HFEnergy"
#define D_CMND_HF_ENERGY_METADATA_NAME         "MetadataName"
#define D_CMND_HF_ENERGY_METADATA_MODEL        "MetadataModel"
#define D_CMND_HF_ENERGY_METADATA_MANUFACTURER "MetadataManufacturer"
#define D_CMND_HF_ENERGY_METADATA_DESCRIPTION  "MetadataDescription"
#define D_CMND_HF_ENERGY_METADATA_LOCATION     "MetadataLocation"

const char kHfEnergyCommands[] PROGMEM = D_PRFX_HF_ENERGY "|" 
  D_CMND_HF_ENERGY_METADATA_NAME "|"
  D_CMND_HF_ENERGY_METADATA_MODEL "|"
  D_CMND_HF_ENERGY_METADATA_MANUFACTURER "|"
  D_CMND_HF_ENERGY_METADATA_DESCRIPTION "|"
  D_CMND_HF_ENERGY_METADATA_LOCATION;

void (* const HfEnergyCommand[])(void) PROGMEM = {
  &CmndHfEnergyMetadataName,
  &CmndHfEnergyMetadataModel,
  &CmndHfEnergyMetadataManufacturer,
  &CmndHfEnergyMetadataDescription,
  &CmndHfEnergyMetadataLocation
};

InfluxDBClient client(HF_ENERGY_INFLUXDB_URL, HF_ENERGY_INFLUXDB_ORG, 
  HF_ENERGY_INFLUXDB_BUCKET, HF_ENERGY_INFLUXDB_TOKEN);

void HfEnergyEvery250ms(void) {
  Point measurement("energy");
  measurement.addTag(HF_ENERGY_METADATA_ID, WiFi.macAddress());
  measurement.addTag(HF_ENERGY_METADATA_NAME, HF_ENERGY_METADATA_NAME_VALUE);
  measurement.addTag(HF_ENERGY_METADATA_MODEL, HF_ENERGY_METADATA_MODEL_VALUE);
  measurement.addTag(HF_ENERGY_METADATA_MANUFACTURER, HF_ENERGY_METADATA_MANUFACTURER_VALUE);
  measurement.addTag(HF_ENERGY_METADATA_DESCRIPTION, HF_ENERGY_METADATA_DESCRIPTION_VALUE);
  measurement.addTag(HF_ENERGY_METADATA_LOCATION, HF_ENERGY_METADATA_LOCATION_VALUE);
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

void CmndHfEnergyMetadataName(void) {
  if (XdrvMailbox.data_len > 0) {
    SettingsUpdateText(SET_HF_ENERGY_METADATA_NAME, XdrvMailbox.data);
  }
  ResponseCmndChar(SettingsText(SET_HF_ENERGY_METADATA_NAME));
}

void CmndHfEnergyMetadataModel(void) {
  if (XdrvMailbox.data_len > 0) {
    SettingsUpdateText(SET_HF_ENERGY_METADATA_MODEL, XdrvMailbox.data);
  }
  ResponseCmndChar(SettingsText(SET_HF_ENERGY_METADATA_MODEL));
}

void CmndHfEnergyMetadataManufacturer(void) {
  if (XdrvMailbox.data_len > 0) {
    SettingsUpdateText(SET_HF_ENERGY_METADATA_MANUFACTURER, XdrvMailbox.data);
  }
  ResponseCmndChar(SettingsText(SET_HF_ENERGY_METADATA_MANUFACTURER));
}

void CmndHfEnergyMetadataDescription(void) {
  if (XdrvMailbox.data_len > 0) {
    SettingsUpdateText(SET_HF_ENERGY_METADATA_DESCRIPTION, XdrvMailbox.data);
  }
  ResponseCmndChar(SettingsText(SET_HF_ENERGY_METADATA_DESCRIPTION));
}

void CmndHfEnergyMetadataLocation(void) {
  if (XdrvMailbox.data_len > 0) {
    SettingsUpdateText(SET_HF_ENERGY_METADATA_LOCATION, XdrvMailbox.data);
  }
  ResponseCmndChar(SettingsText(SET_HF_ENERGY_METADATA_LOCATION));
}

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
      case FUNC_COMMAND:
        result = DecodeCommand(kHfEnergyCommands, HfEnergyCommand);
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
