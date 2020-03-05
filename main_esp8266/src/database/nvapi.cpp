#include "Arduino.h"
#include "nvapi.h"

void nvapi::initial_db(String host, String auth)
{
    Serial.println("start initial firebase");
    Firebase.begin(host, auth);
    Firebase.reconnectWiFi(true);
}

void nvapi::printResult(FirebaseData &data)
{
  if (data.dataType() == "int")
    Serial.println(data.intData());
  else if (data.dataType() == "float")
    Serial.println(data.floatData(), 5);
  else if (data.dataType() == "double")
    printf("%.9lf\n", data.doubleData());
  else if (data.dataType() == "boolean")
    Serial.println(data.boolData() == 1 ? "true" : "false");
  else if (data.dataType() == "string")
    Serial.println(data.stringData());
  else if (data.dataType() == "json")
  {
    Serial.println();
    FirebaseJson &json = data.jsonObject();
    //Print all object data
    Serial.println("Pretty printed JSON data:");
    String jsonStr;
    json.toString(jsonStr, true);
    Serial.println(jsonStr);
    Serial.println();
    Serial.println("Iterate JSON data:");
    Serial.println();
    size_t len = json.iteratorBegin();
    String key, value = "";
    int type = 0;
    for (size_t i = 0; i < len; i++)
    {
      json.iteratorGet(i, type, key, value);
      Serial.print(i);
      Serial.print(", ");
      Serial.print("Type: ");
      Serial.print(type == JSON_OBJECT ? "object" : "array");
      if (type == JSON_OBJECT)
      {
        Serial.print(", Key: ");
        Serial.print(key);
      }
      Serial.print(", Value: ");
      Serial.println(value);
    }
    json.iteratorEnd();
  }
  else if (data.dataType() == "array")
  {
    Serial.println();
    //get array data from FirebaseData using FirebaseJsonArray object
    FirebaseJsonArray &arr = data.jsonArray();
    //Print all array values
    Serial.println("Pretty printed Array:");
    String arrStr;
    arr.toString(arrStr, true);
    Serial.println(arrStr);
    Serial.println();
    Serial.println("Iterate array values:");
    Serial.println();
    for (size_t i = 0; i < arr.size(); i++)
    {
      Serial.print(i);
      Serial.print(", Value: ");

      FirebaseJsonData &jsonData = data.jsonData();
      //Get the result data from FirebaseJsonArray object
      arr.get(jsonData, i);
      if (jsonData.typeNum == JSON_BOOL)
        Serial.println(jsonData.boolValue ? "true" : "false");
      else if (jsonData.typeNum == JSON_INT)
        Serial.println(jsonData.intValue);
      else if (jsonData.typeNum == JSON_DOUBLE)
        printf("%.9lf\n", jsonData.doubleValue);
      else if (jsonData.typeNum == JSON_STRING ||
               jsonData.typeNum == JSON_NULL ||
               jsonData.typeNum == JSON_OBJECT ||
               jsonData.typeNum == JSON_ARRAY)
        Serial.println(jsonData.stringValue);
    }
  }
}

// sent response device status to firebase
int nvapi::sentResponseDeviceStatus(
        FirebaseData     &firebaseData, 
        String           refPath, 
        int              status_mode,
        location_t       deviceLocation,
        String           deviceIP, 
        String           dateTime,
        boolean          enableDebug
    )
{

  Serial.println("sentResponseDeviceStatus() : sent to refPath " + refPath);

  if (!Firebase.beginStream(firebaseData, refPath))
  {
    Serial.println("-------------------------------------------------------------");
    Serial.println("sentResponseDeviceStatus() : Can't begin stream connection...");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("--------------------------------------------------------------");
    Serial.println();
    return -1;
  }

  delay(100);

  if (!Firebase.readStream(firebaseData))
  {
    Serial.println("--------------------------------------------------------");
    Serial.println("sentResponseDeviceStatus() : Can't read stream data");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("-------------------------------------------------------");
    Serial.println();
    return -1;
  }

  if (firebaseData.streamTimeout())
  {
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }

  FirebaseJson json_dv_staus;
  // add node for update device status
  // path for db
  //  dv_status
  //    |--item_id
  //        |--device_id  : 'deviceId'                        'parameter is IPAddr'
  //        |--datetime   : <yyyy/mm/dd hh:mm:ss>             'parameter is datetime_now'
  //        |--status     : <In-Service>,<Out-Of-Service>
  //        |--recheck    : <0> default
  //        |--location
  //              |--latitude   :
  //              |--longitude  :
  //              |--accuracy   :

  String strMode = "";

  if (status_mode == 1)
    strMode = "In-Service";
  if (status_mode == 2)
    strMode = "Out-Off-Service";

  json_dv_staus.add("device_id",  String(deviceIP))
      .add("datetime",            String(dateTime))
      .add("status" ,             strMode)
      .add("recheck",             "0")
      .add("/location/latitude",  String(deviceLocation.lat, 7))
      .add("/location/longitude", String(deviceLocation.lon, 7))
      .add("/location/accuracy",  String(deviceLocation.accuracy));

  if (Firebase.updateNode(firebaseData, refPath, json_dv_staus))
  {
    if (enableDebug)
    {
      Serial.println("--------------------------------------------------------");
      Serial.println("sentResponseDeviceStatus() : updatenode with details");
      Serial.println("PATH: " + firebaseData.dataPath());
      Serial.println("TYPE: " + firebaseData.dataType());
      Serial.print("VALUE: ");
      printResult(firebaseData);
      Serial.println("------------------------------------------------------");
      Serial.println();
    }
  }
  else
  {
    Serial.println("--------------------------------------------------------");
    Serial.println("sentResponseDeviceStatus() : updateNode() failed");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("--------------------------------------------------------");
    Serial.println();
    return -1;
  }

  return 0; // done;
}

// =======================================================================
// this function implement for get 'recheck' to Firebase 'dv_staus'to RDS
// ======================================================================
int nvapi::getRecheckStaus(FirebaseData &firebaseData, String refPath, boolean enableDebug)
{
  String ret = "";

  if (!Firebase.beginStream(firebaseData, refPath))
  {
    Serial.println("---------------------------------------------------------");
    Serial.println("getRecheckStaus() : Can't begin stream connection...");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("---------------------------------------------------------");
    Serial.println();
    return -1;
  }

  delay(100);

  if (!Firebase.readStream(firebaseData))
  {
    Serial.println("--------------------------------------------------------");
    Serial.println("getRecheckStaus() : Can't read stream data");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("-------------------------------------------------------");
    Serial.println();
    return -1;
  }

  if (firebaseData.streamTimeout())
  {
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }

  if (!Firebase.getString(firebaseData, refPath + "/recheck", ret))
  {
    Serial.println("--------------------------------------------------------");
    Serial.println("getRecheckStaus() : updateNode() failed");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("--------------------------------------------------------");
    Serial.println();
    return -1;
  }

  if(enableDebug)
  {
    Serial.println("--------------------------------------------------------");
    Serial.println("getRecheckStaus() : done " + ret );
    Serial.println("--------------------------------------------------------");
  }

  if (ret != "")
    return ret.toInt();
  else
    return -1;
}

// ======================================================================================
// this function implement for set Payment to Firebase to RDS
// =====================================================================================
int nvapi::payTxn(
        FirebaseData    &firebaseData, 
        String          refPath, 
        location_t      deviceLocation,
        String          txnDate, 
        String          txnTime,
        String          deviceIP, 
        String          paymentCode,
        //txn_payment     paymentTxn,
        boolean         enableDebug
    )
{

  Serial.println("sendPayTxn() : sent to node " + refPath);
  
  if (!Firebase.beginStream(firebaseData, refPath))
  {
    Serial.println("-------------------------------------------------------------");
    Serial.println("sendPayTxn() : Can't begin stream connection...");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("--------------------------------------------------------------");
    Serial.println();
    return -1;
  }

  delay(100);

  if (!Firebase.readStream(firebaseData))
  {
    Serial.println("--------------------------------------------------------");
    Serial.println("sendPayTxn() : Can't read stream data");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("-------------------------------------------------------");
    Serial.println();
    return -1;
  }

  if (firebaseData.streamTimeout())
  {
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }

  FirebaseJson json_txn_usage;
  // path for db
  //  txn_usage
  //    |--item_id                             'TP_yyyymmdd_hhmmss_dddddd.dat'
  //        |--device_id      : 'deviceId'       'parameter is IPAddr'
  //        |--device_type    : '15'             'ENV_BUS'
  //        |--location_code  : '0'
  //        |--location
  //              |--latitude   :
  //              |--longitude  :
  //              |--accuracy   :
  //        |--payment
  //              |--passenger_count : '1'
  //              |--passenger_id    : 'uuuuuuuuuuuu'   <read from QR>
  //              |--type            : 'T-Wallet'       <Pay-By-TrueMoney>
  //              |--value           : '20000'          <Satang>
  //              |--billId
  //        |--txn_date              : <yyyy/mm/dd>
  //        |--txn_time              : <hh:mm:ss>
  //        |--txn_status            : '200'            <200 requests,404 https not found,
  //        |--txn_type              : 'Payment'        <1 Payment,2 void>
/*
  json_txn_usage
      .add("device_id",                 String(paymentTxn.deviceIp)
      .add("device_type",               String((paymentTxn.deviceType)
      .add("location_code",             paymentTxn.location_code)
      .add("/location/latitude",        String(paymentTxn.locationList.lat, 7))
      .add("/location/longitude",       String(paymentTxn.locationList.lon, 7))
      .add("/location/accuracy",        String(paymentTxn.locationList.accuracy))
      .add("/payment/passenger_count",  String(paymentTxn.paymentList.passenger_count))
      .add("/payment/passenger_id",     String(paymentTxn.paymentList.passenger_id)) // T-Wallet IDss
      .add("/payment/type",             String(paymentTxn.paymentList.type))
      .add("/payment/value",            String(paymentTxn.paymentList.value))
      .add("/payment/billId",           String(paymentTxn.paymentList.billId))
      .add("txn_date",                  String(paymentTxn.txnDate))
      .add("txn_time",                  String(paymentTxn.txnTime))
      .add("txn_status",                String(paymentTxn.txnStatus))
      .add("txn_type",                  String(paymentTxn.txnType));
*/
  json_txn_usage.add("device_id",           deviceIP)
    .add("device_type",         "15")
    .add("location_code",       "0")
    .add("/location/latitude",  String(deviceLocation.lat, 7))
    .add("/location/longitude", String(deviceLocation.lon, 7))
    .add("/location/accuracy",  String(deviceLocation.accuracy))
    .add("/payment/passenger_count", "1")
    .add("/payment/passenger_id", String(paymentCode))
    .add("/payment/value",      "20.00")
    .add("/payment/type",       "scb")
    .add("/payment/billId",      "12345689012345")
    .add("txn_date",             txnDate)
    .add("txn_time",             txnTime)
    .add("txn_status",           "initial")
    .add("txn_type",             "payment");

  if (Firebase.pushJSON(firebaseData, refPath, json_txn_usage))
  {
    if (enableDebug)
    {
      Serial.println("-----------------------------------------------");
      Serial.println("sendPayTxn() : pushJSON with details");
      Serial.println("PATH: " + firebaseData.dataPath());
      Serial.print("PUSH NAME: ");
      Serial.println(firebaseData.pushName());
      Serial.println("ETag: " + firebaseData.ETag());
      //printResult(firebaseData);
      Serial.println("-----------------------------------------------");
      Serial.println();
    }
  }
  else
  {
    Serial.println("--------------------------------------------------");
    Serial.println("sendPayTxn() : updateNode() failed");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("--------------------------------------------------");
    Serial.println();
    return -1;
  }

  return 0; // done
}

// =======================================================================
// this function implement for get 'recheck' to Firebase 'dv_staus'to RDS
// ======================================================================
String nvapi::getTxnStaus(FirebaseData &firebaseData, String refPath)
{
  String ret = "";

  if (!Firebase.beginStream(firebaseData, refPath))
  {
    Serial.println("---------------------------------------------------------");
    Serial.println("getTxnStaus() : Can't begin stream connection...");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("---------------------------------------------------------");
    Serial.println();
    return "error";
  }

  delay(100);

  if (!Firebase.readStream(firebaseData))
  {
    Serial.println("--------------------------------------------------------");
    Serial.println("getTxnStaus() : Can't read stream data");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("-------------------------------------------------------");
    Serial.println();
    return "error";
  }

  if (firebaseData.streamTimeout())
  {
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
    return "timeout";
  }

  if (!Firebase.getString(firebaseData, refPath + "/txn_status", ret))
  {
    Serial.println("--------------------------------------------------------");
    Serial.println("sentResponseDeviceStatus() : updateNode() failed");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("--------------------------------------------------------");
    Serial.println();
    return "error";
  }

  return ret;
}



// sent response device status to firebase
int nvapi::sentPayAPI(
        FirebaseData     &firebaseData, 
        String           refPath, 
        String           paymentCode, 
        String           paymentValue, 
        String           txnDate,
        String           txnTime,
        location_t       deviceLocation,
        String           deviceIP, 
        boolean          enableDebug
    )
{

  Serial.println("sentPayAPI() : sent to path " + refPath);
  /*

  if (!Firebase.beginStream(firebaseData, refPath))
  {
    Serial.println("-------------------------------------------------------------");
    Serial.println("sentPayAPI() : Can't begin stream connection...");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("--------------------------------------------------------------");
    Serial.println();
    return -1;
  }

  delay(100);

  if (!Firebase.readStream(firebaseData))
  {
    Serial.println("--------------------------------------------------------");
    Serial.println("sentPayAPI() : Can't read stream data");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("-------------------------------------------------------");
    Serial.println();
    return -1;
  }

  if (firebaseData.streamTimeout())
  {
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }
  */

  String ETag = "";

  if (Firebase.setString(firebaseData, refPath + "/payment/passenger_id", String(paymentCode), ETag))
  {
    if (enableDebug)
    {
      Serial.println("--------------------------------------------------------");
      Serial.println("sentPayAPI() : set string with details");
      Serial.println("PASSED");
      Serial.println("PATH: " + firebaseData.dataPath());
      Serial.println("TYPE: " + firebaseData.dataType());
      Serial.println("CURRENT ETag: " + firebaseData.ETag());
      ETag = firebaseData.ETag();
      Serial.print("VALUE: ");
      printResult(firebaseData);
      Serial.println("------------------------------------------------------");
      Serial.println();
    }
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
    return -1;
  }

  FirebaseJson json_tx_usage;

  json_tx_usage.add("device_id",      String(deviceIP))
      .add("device_type",             "15")
      .add("location_code",           "0")
      .add("/location/latitude",      String(deviceLocation.lat, 7))
      .add("/location/longitude",     String(deviceLocation.lon, 7))
      .add("/location/accuracy",      String(deviceLocation.accuracy))
      .add("/payment/passenger_count",     "1")
      //.add("/payment/passenger_id",   String(paymentCode))
      .add("/payment/value",          String(paymentValue))
      .add("/payment/type",           "scb")
      .add("/payment/billId",         "12345689012345")
      .add("txn_date",                String(txnDate))
      .add("txn_time",                String(txnTime))
      .add("txn_status",              "initial")
      .add("txn_type",                "payment");

  if (Firebase.updateNode(firebaseData, refPath, json_tx_usage))
  {
    if (enableDebug)
    {
      Serial.println("--------------------------------------------------------");
      Serial.println("sentPayAPI() : updatenode with details");
      Serial.println("PATH: " + firebaseData.dataPath());
      Serial.println("TYPE: " + firebaseData.dataType());
      Serial.print("VALUE: ");
      printResult(firebaseData);
      Serial.println("------------------------------------------------------");
      Serial.println();
    }
  }
  else
  {
    Serial.println("--------------------------------------------------------");
    Serial.println("sentPayAPI() : updateNode() failed");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("--------------------------------------------------------");
    Serial.println();
    return -1;
  }

  return 0; // done;
}


int nvapi::sentRequestPay(
            FirebaseData    &firebaseData,
            String          refPath,
            String          typeQR,
            String          qrData, 
            int             txnValue,
            location_t      deviceLocation,    
            boolean         enableDebug
        )
{
    String ETag = "";

    String txnAmount = "20.00";

    if (Firebase.setString(firebaseData, refPath + "/paymentId", String(qrData), ETag))
    {
      if(enableDebug)
      {
        Serial.println("--------------------------------------------------------");
        Serial.println("sentRequestPay() : set stringValue with details");
        Serial.println("PASSED");
        Serial.println("PATH: " + firebaseData.dataPath());
        Serial.println("TYPE: " + firebaseData.dataType());
        Serial.println("CURRENT ETag: " + firebaseData.ETag());
        ETag = firebaseData.ETag();
        Serial.print("VALUE: ");
        printResult(firebaseData);
        Serial.println("------------------------------------------------------");
        Serial.println();
      }
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + firebaseData.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
      return -1;
    }

    FirebaseJson jsonPaymnetRequest;

    jsonPaymnetRequest
        .add("state",               "initial")
        .add("type",                String(typeQR)) 
        .add("transactionAmount",   "20.00")
        .add("/location/latitude",  String(deviceLocation.lat, 7))
        .add("/location/longitude", String(deviceLocation.lon, 7))
        .add("/location/accuracy",  String(deviceLocation.accuracy));

    if (Firebase.updateNode(firebaseData, refPath, jsonPaymnetRequest))
    {
      if (enableDebug)
      {
        Serial.println("--------------------------------------------------------");
        Serial.println("sentRequestPay() : updatenode with details");
        Serial.println("PATH: " + firebaseData.dataPath());
        Serial.println("TYPE: " + firebaseData.dataType());
        Serial.print("VALUE: ");
        printResult(firebaseData);
        Serial.println("------------------------------------------------------");
        Serial.println();
      }
    }
    else
    {
      Serial.println("--------------------------------------------------------");
      Serial.println("sentRequestPay() : updateNode() failed");
      Serial.println("REASON: " + firebaseData.errorReason());
      Serial.println("--------------------------------------------------------");
      Serial.println();
      return -1;
    }

    /*
    if (Firebase.setString(firebaseData, path_tx_usage + "/state", "initial", ETag))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + firebaseData.dataPath());
      Serial.println("TYPE: " + firebaseData.dataType());
      Serial.println("CURRENT ETag: " + firebaseData.ETag());
      ETag = firebaseData.ETag();
      Serial.print("VALUE: ");
      printResult(firebaseData);
      Serial.println("------------------------------------");
      Serial.println();
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + firebaseData.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
      return -1;
    }

    if (Firebase.setString(firebaseData, path_tx_usage + "/transactionAmount", String(txnAmount), ETag))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + firebaseData.dataPath());
      Serial.println("TYPE: " + firebaseData.dataType());
      Serial.println("CURRENT ETag: " + firebaseData.ETag());
      ETag = firebaseData.ETag();
      Serial.print("VALUE: ");
      printResult(firebaseData);
      Serial.println("------------------------------------");
      Serial.println();
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + firebaseData.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
      return -1;
    }

    if (Firebase.setString(firebaseData, path_tx_usage + "/type", String(typeQR), ETag))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + firebaseData.dataPath());
      Serial.println("TYPE: " + firebaseData.dataType());
      Serial.println("CURRENT ETag: " + firebaseData.ETag());
      ETag = firebaseData.ETag();
      Serial.print("VALUE: ");
      printResult(firebaseData);
      Serial.println("------------------------------------");
      Serial.println();
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + firebaseData.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
      return -1;
    }
    */

    return 0; // done
}

String nvapi::getDevcieResigter(
    FirebaseData &firebaseData,
    String      refPath,
    String      deviceName,
    boolean     enableDebug
)
{
  String ret = "";

  if (!Firebase.beginStream(firebaseData, refPath))
  {
    Serial.println("---------------------------------------------------------");
    Serial.println("getDevcieResigter() : Can't begin stream connection...");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("---------------------------------------------------------");
    Serial.println();
    return "error";
  }

  delay(100);

  if (!Firebase.readStream(firebaseData))
  {
    Serial.println("--------------------------------------------------------");
    Serial.println("getDevcieResigter() : Can't read stream data");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("-------------------------------------------------------");
    Serial.println();
    return "error";
  }

  if (firebaseData.streamTimeout())
  {
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
    return "timeout";
  }

  if (!Firebase.getString(firebaseData, refPath + "/ei_list/" + deviceName + "/status/", ret))
  {
    Serial.println("--------------------------------------------------------");
    Serial.println("sentResponseDevcieResigter() : updateNode() failed");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("--------------------------------------------------------");
    Serial.println();
    return "error";
  }

  return ret;
}