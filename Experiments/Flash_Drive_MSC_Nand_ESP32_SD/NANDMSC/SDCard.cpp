#include <Arduino.h>
#include "SDCard.h"

SDCard::SDCard(Stream &debug, const char *mount_point) : m_debug(debug), m_mount_point(mount_point)
{
}

SDCard::~SDCard()
{
}
