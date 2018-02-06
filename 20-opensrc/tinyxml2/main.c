
#include "xmlparser.h"


int main(int argc, char *argv[])
{
    TManscdpXmlResult  result;

    HXmlParser parser = xml_create();
    printf("parser:%d\n", parser);

    xml_parse_text(parser, "<?xml version=\"1.0\" encoding=\"gb2312\"?>\r\n\r\n<Notify>\r\n<CmdType>Keepalive</CmdType>\r\n<SN>1</SN>\r\n<DeviceID>34020000001180000012</DeviceID>\r\n<Status>OK</Status>\r\n<Info>\r\n</Info>\r\n</Notify>\r\n", &result);

    return 0;
}
