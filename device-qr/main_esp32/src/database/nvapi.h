#ifndef NVAPI_H
#define NVAPI_H

#include "Arduino.h"
#ifdef ESP8266
    #include <ESP8266WiFi.h>
    #include "FirebaseESP8266.h"
#else
    #include <WiFi.h>
    #include <FirebaseESP32.h>
#endif

#include "../utils/utils.h"
#include "../utils/data_types.h"

class nvapi
{
    public:
        void initial_db(String host, String auth);

        int sentResponseDeviceStatus (
            FirebaseData &firebaseData, 
            String       refPath, 
            int          status_mode,
            location_t   deviceLocation,
            String       deviceIP, 
            String       dateTime,
            boolean      enableDebug
        );

        int getRecheckStaus(
            FirebaseData &firebaseData, 
            String       refPath
        );

        int payTxn(
            FirebaseData    &firebaseData,
            String          refPath,
            location_t      deviceLocation,
            String          txnDate,
            String          txnTime,
            String          deviceIP,
            String          paymentCode,
            //txn_payment     paymentTxn,
            boolean         enableDebug
        );

        String getTxnStaus (
            FirebaseData &firebaseData, 
            String       refPath
        );
        
        int sentPayAPI(
            FirebaseData     &firebaseData, 
            String           refPath, 
            String           paymentCode, 
            String           paymentValue, 
            String           txnDate,
            String           txnTime,
            location_t       deviceLocation,
            String           deviceIP, 
            boolean          enableDebug
        );

        int sentRequestPay(
            FirebaseData    &firebaseData,
            String          refPath,
            String          typeQR,
            String          qrData, 
            int             txnValue,
            location_t      deviceLocation,   
            boolean         enableDebug
        );

    private : 
        void printResult(FirebaseData &data);
};

#endif