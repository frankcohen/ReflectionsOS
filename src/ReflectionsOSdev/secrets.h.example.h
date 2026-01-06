/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 For security this repository does not store secrets.h or other files
 containing passwords, SSL certificates, and other sensitive data.

 ReflectionsOS repository has a .gitignore setting to disallow
 Secrets.h and other files from being committed.
 Instead make a copy of this file, name it secrets.h, and customize
 it to the latest secrets.

 If you find secrets.h part of a repository, it is there by mistake. Immediately
 delete the file and report it to fcohen@starlingwatch.com. Thank you!

*/

#define SECRET_SSID ""
#define SECRET_PASS ""

// Must be 8 characters or longer
#define CALLIOPE_WIFI_PASS "notdefined"

#define cloudCityURL ""
#define cloudCityHostURL ""

/*
I used this tutorial to use Chrome to get the root certificate
https://randomnerdtutorials.com/esp32-https-requests/

nginx ssl installation instruction at:
https://help.zerossl.com/hc/en-us/articles/360058295894-Installing-SSL-Certificate-on-NGINX

NOTE: You must combine the certificate.crt and ca_bundle.crt files

/etc/ssl/cloudcity/certificate.crt;
/etc/ssl/cloudcity/private.key;

Then do this from your Mac laptop:
openssl s_client -showcerts -servername url -connect url:443 </dev/null

Then copy the second certificate:
-----END CERTIFICATE-----
 1 s:/C=AT/O=ZeroSSL/CN=ZeroSSL RSA Domain Secure Site CA
   i:/C=US/ST=New Jersey/L=Jersey City/O=The USERTRUST Network/CN=USERTrust RSA Certification Authority
-----BEGIN CERTIFICATE-----
MIIG1TCCBL2gAwIBAgIQbFWr29AHksedBwzYEZ7WvzANBgkqhkiG9w0BAQwFADCB
iDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0pl
*/

// #define ssl_cert "insert cert here"
