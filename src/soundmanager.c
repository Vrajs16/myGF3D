#include <gfc_audio.h>
#include "gfc_hashmap.h"

HashMap *soundPack = NULL;

void setSoundPack(const char *filename)
{
    soundPack = gfc_sound_pack_parse_file(filename);
}

void playSound(const char *name, int loops, float volume, int channel, int group)
{
    gfc_sound_pack_play(soundPack, name, loops, volume, channel, group);
}