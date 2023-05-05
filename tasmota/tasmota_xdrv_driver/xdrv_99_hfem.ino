#ifdef ESP8266
#ifdef USE_ENERGY_SENSOR

#define XDRV_99                99
#define XSNS_99                99

#define WEB_HANDLE_HFEM "hfem"

// set the following in user_config_override.h
// #define HFEM_INFLUXDB_URL
// #define HFEM_INFLUXDB_TOKEN
// #define HFEM_INFLUXDB_ORG
// #define HFEM_INFLUXDB_BUCKET

#define HFEM_INFLUXDB_MAX_BATCH_SIZE    10
#define HFEM_INFLUXDB_WRITE_BUFFER_SIZE 40

#define HFEM_METADATA_ID           "id"
#define HFEM_METADATA_NAME         "name"
#define HFEM_METADATA_MODEL        "model"
#define HFEM_METADATA_MANUFACTURER "manufacturer"
#define HFEM_METADATA_CATEGORY     "category"
#define HFEM_METADATA_DESCRIPTION  "description"
#define HFEM_METADATA_LOCATION     "location"

#define D_PRFX_HFEM                       "Hfem"
#define D_CMND_HFEM_ENABLED               "Enabled"
#define D_CMND_HFEM_METADATA_NAME         "MetadataName"
#define D_CMND_HFEM_METADATA_MODEL        "MetadataModel"
#define D_CMND_HFEM_METADATA_MANUFACTURER "MetadataManufacturer"
#define D_CMND_HFEM_METADATA_CATEGORY     "MetadataCategory"
#define D_CMND_HFEM_METADATA_DESCRIPTION  "MetadataDescription"
#define D_CMND_HFEM_METADATA_LOCATION     "MetadataLocation"

#define D_HFEM_START_RECORDING "Start Recording"

#include <InfluxDbClient.h>

struct HfemSettings {
  String metadata_name;
  String metadata_model;
  String metadata_manufacturer;
  String metadata_category;
  String metadata_description;
  String metadata_location;
} HfemSettings;

const char kHfemCommands[] PROGMEM = D_PRFX_HFEM "|" 
  D_CMND_HFEM_ENABLED "|"
  D_CMND_HFEM_METADATA_NAME "|"
  D_CMND_HFEM_METADATA_MODEL "|"
  D_CMND_HFEM_METADATA_MANUFACTURER "|"
  D_CMND_HFEM_METADATA_CATEGORY "|"
  D_CMND_HFEM_METADATA_DESCRIPTION "|"
  D_CMND_HFEM_METADATA_LOCATION;

void (* const HfemCommand[])(void) PROGMEM = {
  &CmndHfemEnabled,
  &CmndHfemMetadataName,
  &CmndHfemMetadataModel,
  &CmndHfemMetadataManufacturer,
  &CmndHfemMetadataCategory,
  &CmndHfemMetadataDescription,
  &CmndHfemMetadataLocation
};

const char HTTP_BTN_MENU_HFEM[] PROGMEM =
  "<p><form action='" WEB_HANDLE_HFEM "' method='get'><button>" D_HFEM_START_RECORDING "</button></form></p>";

const char HTTP_SNS_HFEM[] PROGMEM =
  "{s}HFEM{m}{e}"
  "{s}Recording" "{m}%s" "{e}"
  "{s}Name" "{m}%s " "{e}"                // {s} = <tr><th>, {m} = </th><td>, {e} = </td></tr>
  "{s}Model" "{m}%s " "{e}"
  "{s}Manufacturer" "{m}%s " "{e}"
  "{s}Category" "{m}%s " "{e}"
  "{s}Description" "{m}%s " "{e}"
  "{s}Location" "{m}%s " "{e}"
  ;

const char HTTP_FORM_HFEM[] PROGMEM =
  "<fieldset><legend><b>&nbsp;" D_HFEM_START_RECORDING "&nbsp;</b></legend>"
  "<form method='get' action='" WEB_HANDLE_HFEM "'>"
  "<p><b>Name:</b><br><input id='md_name' value=\"%s\" required></p>"
  "<p><b>Model:</b><br><input id='md_model' value=\"%s\" required></p>"
  "<p><b>Manufacturer:</b><br><input id='md_manufacturer' value=\"%s\" required></p>"
  "<p><b>Category:</b><br><input id='md_category' value=\"%s\" required></p>"
  "<p><b>Description:</b><br><input id='md_description' value=\"%s\" required></p>"
  "<p><b>Location:</b><br><input id='md_location' value=\"%s\" required></p>";

InfluxDBClient client(HFEM_INFLUXDB_URL, HFEM_INFLUXDB_ORG, 
  HFEM_INFLUXDB_BUCKET, HFEM_INFLUXDB_TOKEN);

void HfemSaveSettings(void) {
  String cmnd = F(D_CMND_BACKLOG "0 ");
  cmnd += AddWebCommand(PSTR(D_PRFX_HFEM D_CMND_HFEM_METADATA_NAME), PSTR("md_name"), PSTR("1"));
  cmnd += AddWebCommand(PSTR(D_PRFX_HFEM D_CMND_HFEM_METADATA_MODEL), PSTR("md_model"), PSTR("1"));
  cmnd += AddWebCommand(PSTR(D_PRFX_HFEM D_CMND_HFEM_METADATA_MANUFACTURER), PSTR("md_manufacturer"), PSTR("1"));
  cmnd += AddWebCommand(PSTR(D_PRFX_HFEM D_CMND_HFEM_METADATA_CATEGORY), PSTR("md_category"), PSTR("1"));
  cmnd += AddWebCommand(PSTR(D_PRFX_HFEM D_CMND_HFEM_METADATA_DESCRIPTION), PSTR("md_description"), PSTR("1"));
  cmnd += AddWebCommand(PSTR(D_PRFX_HFEM D_CMND_HFEM_METADATA_LOCATION), PSTR("md_location"), PSTR("1"));
  ExecuteWebCommand((char*)cmnd.c_str());
}

const char* getHfemSetting(String &s) {
  if(s.isEmpty()) {
    return "<p style=\"color:rgb(255,0,0);margin: 0;\">not set</p>";
  }
  return s.c_str();
}

void HandleHfemConfiguration(void)
{
  if (!HttpCheckPriviledgedAccess()) { return; }

  if (Webserver->hasArg(F("save"))) {
    HfemSaveSettings();
    HandleRoot();
    return;
  }

  WSContentStart_P(PSTR("Start Recording"));
  WSContentSendStyle();
  WSContentSend_P(HTTP_FORM_HFEM,
    HfemSettings.metadata_name.c_str(), 
    HfemSettings.metadata_model.c_str(), 
    HfemSettings.metadata_manufacturer.c_str(),
    HfemSettings.metadata_category.c_str(), 
    HfemSettings.metadata_description.c_str(), 
    HfemSettings.metadata_location.c_str());
  WSContentSend_P(HTTP_FORM_END);
  WSContentSpaceButton(BUTTON_MAIN);
  WSContentStop();
}

bool isMetadataSet(void) {
  return !HfemSettings.metadata_name.isEmpty() 
    && !HfemSettings.metadata_model.isEmpty() 
    && !HfemSettings.metadata_manufacturer.isEmpty()
    && !HfemSettings.metadata_description.isEmpty()
    && !HfemSettings.metadata_location.isEmpty();
}

void HfEnergyEvery250ms(void) {
  if(Settings->flag6.spare31 && isMetadataSet()) {
    Point measurement("energy");
    measurement.addTag(HFEM_METADATA_ID, WiFi.macAddress());
    measurement.addTag(HFEM_METADATA_NAME, HfemSettings.metadata_name);
    measurement.addTag(HFEM_METADATA_MODEL, HfemSettings.metadata_model);
    measurement.addTag(HFEM_METADATA_MANUFACTURER, HfemSettings.metadata_manufacturer);
    measurement.addTag(HFEM_METADATA_CATEGORY, HfemSettings.metadata_category);
    measurement.addTag(HFEM_METADATA_DESCRIPTION, HfemSettings.metadata_description);
    measurement.addTag(HFEM_METADATA_LOCATION, HfemSettings.metadata_location);
    measurement.addField("current", Energy->current[0]);
    measurement.addField("voltage", Energy->voltage[0]);
    measurement.addField("active_power", Energy->active_power[0]);
    measurement.setTime(getUtcNow().c_str());

    AddLog(LOG_LEVEL_INFO, PSTR("Influxdb: %s"), client.pointToLineProtocol(measurement).c_str());

    client.writePoint(measurement);
  }
}

void HfEnergyShow(void) {
  String recording = Settings->flag6.spare31 && isMetadataSet() ? "Yes" : "No";
  WSContentSend_PD(HTTP_SNS_HFEM,
    recording.c_str(),
    getHfemSetting(HfemSettings.metadata_name),
    getHfemSetting(HfemSettings.metadata_model),
    getHfemSetting(HfemSettings.metadata_manufacturer),
    getHfemSetting(HfemSettings.metadata_category),
    getHfemSetting(HfemSettings.metadata_description),
    getHfemSetting(HfemSettings.metadata_location)
  );
}

/*********************************************************************************************\
 * Commands
\*********************************************************************************************/

void CmndHfemMetadataName(void) {
  if (XdrvMailbox.data_len > 0) {
    HfemSettings.metadata_name = XdrvMailbox.data;
  }
  ResponseCmndChar(HfemSettings.metadata_name.c_str());
}

void CmndHfemMetadataModel(void) {
  if (XdrvMailbox.data_len > 0) {
    HfemSettings.metadata_model = XdrvMailbox.data;
  }
  ResponseCmndChar(HfemSettings.metadata_model.c_str());
}

void CmndHfemMetadataManufacturer(void) {
  if (XdrvMailbox.data_len > 0) {
    HfemSettings.metadata_manufacturer = XdrvMailbox.data;
  }
  ResponseCmndChar(HfemSettings.metadata_manufacturer.c_str());
}

void CmndHfemMetadataCategory(void) {
  if (XdrvMailbox.data_len > 0) {
    HfemSettings.metadata_category = XdrvMailbox.data;
  }
  ResponseCmndChar(HfemSettings.metadata_category.c_str());
}

void CmndHfemMetadataDescription(void) {
  if (XdrvMailbox.data_len > 0) {
    HfemSettings.metadata_description = XdrvMailbox.data;
  }
  ResponseCmndChar(HfemSettings.metadata_description.c_str());
}

void CmndHfemMetadataLocation(void) {
  if (XdrvMailbox.data_len > 0) {
    HfemSettings.metadata_location = XdrvMailbox.data;
  }
  ResponseCmndChar(HfemSettings.metadata_location.c_str());
}

void CmndHfemEnabled(void) {
  if ((XdrvMailbox.payload >= 0) && (XdrvMailbox.payload <= 1)) {
    Settings->flag6.spare31 = XdrvMailbox.payload;
  }
  ResponseCmndChar(GetStateText(Settings->flag6.spare31));
}

void HfEnergyDrvInit(void) {
  client.setWriteOptions(WriteOptions()
    .writePrecision(WritePrecision::MS)
    .batchSize(HFEM_INFLUXDB_MAX_BATCH_SIZE)
    .bufferSize(HFEM_INFLUXDB_WRITE_BUFFER_SIZE));
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
      case FUNC_WEB_ADD_MAIN_BUTTON:
        WSContentSend_P(HTTP_BTN_MENU_HFEM);
        break;
      case FUNC_WEB_ADD_HANDLER:
        WebServer_on(PSTR("/" WEB_HANDLE_HFEM), HandleHfemConfiguration);
        break;
      case FUNC_COMMAND:
        result = DecodeCommand(kHfemCommands, HfemCommand);
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
      case FUNC_WEB_SENSOR:
        HfEnergyShow();
        break;
    }
  }
  return result;
}

#endif  // USE_ENERGY_SENSOR
#endif  // ESP8266
