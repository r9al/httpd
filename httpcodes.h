//Copyright Anoop Kumar Narayanan <anoop.kumar.narayanan@gmail.com> , LICENSE - GPLv2 / GPLv3
#ifndef HTTP_STATUS_CODES_H
#define HTTP_STATUS_CODES_H

#include <iostream>
using namespace std;

#define HTTP_RESP_CONTINUE                    100
#define HTTP_RESP_SWITCHPROTO               101
#define HTTP_RESP_PROCESSING     102
#define HTTP_RESP_OK     200
#define HTTP_RESP_CREATED     201
#define HTTP_RESP_ACCEPTED     202
#define HTTP_RESP_NONAUTHINFO    203
#define HTTP_RESP_NOCONTENT     204
#define HTTP_RESP_RESETCONTENT     205
#define HTTP_RESP_PARTIALCONTENT     206
#define HTTP_RESP_MULTISTATUS     207
#define HTTP_RESP_MULTIPLECHOICES     300
#define HTTP_RESP_MOVEDPARMANENTLY     301
#define HTTP_RESP_FOUND     302
#define HTTP_RESP_SEEOTHER     303
#define HTTP_RESP_NOTMODIFIED    304
#define HTTP_RESP_USEPROXY     305
#define HTTP_RESP_TEMPREDIRECT     307
#define HTTP_RESP_BADREQ     400
#define HTTP_RESP_UNAUTHORIZED     401
#define HTTP_RESP_PAYMENTREQ     402
#define HTTP_RESP_FORBIDDEN     403
#define HTTP_RESP_NOTFOUND     404
#define HTTP_RESP_METHODNOTALLOWED     405
#define HTTP_RESP_NOTACCEPTABLE     406
#define HTTP_RESP_PROXYAUTHREQ     407
#define HTTP_RESP_REQTIMEOUT     408
#define HTTP_RESP_CONFLICT
#define HTTP_RESP_GONE     410
#define HTTP_RESP_LENREQ   411
#define HTTP_RESP_PRECONDITIONFAIL     412
#define HTTP_RESP_REQENTITYTOOLARGE     413
#define HTTP_RESP_URILONG     414
#define HTTP_RESP_UNSUPPORTEDMEDIA     415
#define HTTP_RESP_REQRANGENOTSATISFIABLE     416
#define HTTP_RESP_EXPFAILED         417
#define HTTP_RESP_UNPROCESSABELENTITY     422
#define HTTP_RESP_LOCKED     423
#define HTTP_RESP_FAILEDDEP   424
#define HTTP_RESP_UPGRADEREQD     426
#define HTTP_RESP_INTERNALERR     500
#define HTTP_RESP_NOTIMPL     501
#define HTTP_RESP_BADGATEWAY     502
#define HTTP_RESP_SERVICEUNAVAILABLE     503
#define HTTP_RESP_GATEWAYTIMEOUT     503
#define HTTP_RESP_HTTPVERSIONUNSUPPORTED     504
#define HTTP_RESP_INSUFFICIENTSTORAGE     507


void setupStatusCodes();
const char *getStatusInfo ( int code );

#endif
