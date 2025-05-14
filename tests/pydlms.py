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
auth = enumerations.AuthenticationMechanism.HLS_GMAC
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


with public_client.session() as client:

    response_data = client.get(
        cosem.CosemAttribute(
            interface=enumerations.CosemInterface.ASSOCIATION_LN,
            instance=cosem.Obis(0, 0, 0x28, 0, 0),
            attribute=2,
        )
    )
    data_decoder = a_xdr.AXdrDecoder(
        encoding_conf=a_xdr.EncodingConf(
            attributes=[a_xdr.Sequence(attribute_name="data")]
        )
    )
    object_list = data_decoder.decode(response_data)["data"]
    print(f"objectlist = {object_list}")


