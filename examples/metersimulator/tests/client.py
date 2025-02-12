import logging
from pprint import pprint
from time import sleep

from dateutil import parser as dateparser

from dlms_cosem import a_xdr, cosem, enumerations
from dlms_cosem.security import (
    NoSecurityAuthentication,
    HighLevelSecurityGmacAuthentication,
)
from dlms_cosem.client import DlmsClient
from dlms_cosem.io import BlockingTcpIO, TcpTransport
from dlms_cosem.cosem import selective_access
from dlms_cosem.cosem.selective_access import RangeDescriptor
from dlms_cosem.parsers import ProfileGenericBufferParser
from dlms_cosem.protocol.xdlms.conformance import Conformance
from dlms_cosem.time import datetime_from_bytes

# set up logging so you get a bit nicer printout of what is happening.
logging.basicConfig(
    level=logging.DEBUG,
    format="%(asctime)s,%(msecs)d : %(levelname)s : %(message)s",
    datefmt="%H:%M:%S",
)

c = Conformance(
    general_protection=False,
    general_block_transfer=False,
    delta_value_encoding=False,
    attribute_0_supported_with_set=False,
    priority_management_supported=False,
    attribute_0_supported_with_get=False,
    block_transfer_with_get_or_read=True,
    block_transfer_with_set_or_write=False,
    block_transfer_with_action=True,
    multiple_references=True,
    data_notification=False,
    access=False,
    get=True,
    set=True,
    selective_access=True,
    event_notification=False,
    action=True,
)

encryption_key = bytes.fromhex("990EB3136F283EDB44A79F15F0BFCC21")
authentication_key = bytes.fromhex("EC29E2F4BD7D697394B190827CE3DD9A")
auth = enumerations.AuthenticationMechanism.LLS
host = "127.0.0.1"
port = 4063


tcp_io = BlockingTcpIO(host=host, port=port)
public_tcp_transport = TcpTransport(
    client_logical_address=16,
    server_logical_address=1,
    io=tcp_io,
)
public_client = DlmsClient(
    transport=public_tcp_transport, authentication=NoSecurityAuthentication()
)

from datetime import datetime

def decode_dlms_datetime(dt_bin):
    # Assumer que dt_bin est un octet de 12 octets
    if len(dt_bin) != 12:
        raise ValueError("La taille de la donnée datetime doit être de 12 octets")
    
    # Extraire chaque champ de la chaîne binaire
    year_high = dt_bin[0]
    year_low = dt_bin[1]
    month = dt_bin[2]
    day_of_month = dt_bin[3]
    day_of_week = dt_bin[4]
    hour = dt_bin[5]
    minute = dt_bin[6]
    second = dt_bin[7]
    hundredths = dt_bin[8]
    deviation_high = dt_bin[9]
    deviation_low = dt_bin[10]
    clock_status = dt_bin[11]
    
    # Calculer l'année
    year = (year_high << 8) + year_low
    
    # Si un champ est à 0xFF, cela signifie "non spécifié"
    if hour == 0xFF:
        hour = None
    if minute == 0xFF:
        minute = None
    if second == 0xFF:
        second = None
    if hundredths == 0xFF:
        hundredths = None
    
    # Construire la date
    date_str = f"{year:04d}-{month:02d}-{day_of_month:02d}"
    time_str = f"{hour:02d}:{minute:02d}:{second:02d}" if hour is not None else "Not specified"
    
    # Afficher les informations
    print(f"Date: {date_str}")
    print(f"Time: {time_str}")
    print(f"Day of Week: {day_of_week}")
    print(f"Deviation: {deviation_high:02x}{deviation_low:02x}")
    print(f"Clock Status: {clock_status}")
    

with public_client.session() as client:

    response_data = client.get(
        cosem.CosemAttribute(
            interface=enumerations.CosemInterface.CLOCK,
            instance=cosem.Obis.from_string("0.0.1.0.0.255"),
            attribute=2,
        )
    )
    data_decoder = a_xdr.AXdrDecoder(
        encoding_conf=a_xdr.EncodingConf(
            attributes=[a_xdr.Sequence(attribute_name="data")]
        )
    )

    dt = data_decoder.decode(response_data)["data"]
    print(f"datetime = {datetime_from_bytes(dt)}")

    decode_dlms_datetime(dt)

