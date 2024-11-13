
# CosemReader

Example of DLMS/COSEM Client software with XML export of read data.

# Change history

## Version 1.2.0

  * Add HDLC retry in case of bad communication channel (partial frame received)
  * Oversized the Application buffer from 200KB to 2MB
  * Better management of authentication in case of bad security key (explicit error message)

## Version 1.1.0

  * Generation of a communication report for each meter, XML format (_result.xml)
  * Added HDLC retries in case of frame parsing error

## Version 1.0.0

  * Initial version, supports SL7K and Saphir meters
