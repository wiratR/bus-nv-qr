#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <Arduino.h>
#include <WifiLocation.h>

struct paymentDetails{
    String  passenger_count;
    String  passenger_id;
    String  type;
    String  value;
    String  billId;
};

struct txn_payment {
        String              deviceIp;
        String              deviceType; 
        String              location_code;
        location_t          locationList;
        paymentDetails      paymentList;
        String              txnDate;
        String              txnTime;
        String              txnStatus;
        String              txnType;
};

#endif