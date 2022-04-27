#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fluidsynth.h>
#include <time.h>

int main(int argc, char *argv[]) {
	if(argc != 2) {
		printf("Usage: %s inputbin\n", argv[0]);
		exit(1);
	}

	FILE *bin = fopen(argv[1], "rb");

	float freq = 72.82;

	fluid_settings_t *settings;
	fluid_synth_t *synth;
	fluid_audio_driver_t *adriver;
	
	settings = new_fluid_settings();
	fluid_settings_setstr(settings, "audio.driver", "pulseaudio");
	fluid_settings_setint(settings, "audio.period-size", 512);
	fluid_settings_setint(settings, "synth.chorus.active", 0);
	
	synth = new_fluid_synth(settings);
	adriver = new_fluid_audio_driver(settings, synth);
	
	if(fluid_synth_sfload(synth, "gm.sf2", 1) == -1) {
		printf("\nError loading \"gm.sf2\". Please place a GM-compatible soundfont in the same directory as this program and rename it to \"gm.sf2\".\n\n");
		exit(1);
	}

	uint8_t msg;
	uint8_t par[2];

	fread(&msg, 1, 1, bin);

	printf("File intended for %s\n",
		(msg == 5) ? "Roland SC-55 (and other GM compatible synths)" :
		((msg == 6) ? "Roland MT-32 (and compatibles)" : "unknown, possibly corrupted file"));

	uint8_t shortdelay;
	uint16_t longdelay;

	int tick = 0, targettick = 0;
	int us = 1000000 / freq;
	struct timespec ticks = { us / 1000000, (us % 1000000) * 1000};

	int maxchannels = 0, channels = 0;

	while(fread(&msg, 1, 1, bin)) {
		if(channels > maxchannels) maxchannels = channels;

		while(targettick > tick) {
			printf("\rPlaying %.2fs (tick %d @ %.2f Hz, read %ld bytes) (%02d/%02d)...", ((float)tick) / freq, tick, freq, ftell(bin), channels, maxchannels);
			fflush(stdout);
			nanosleep(&ticks, NULL);
			tick++;
		}

		if(msg >= 0xD0 && msg <= 0xDD) {
			targettick += (msg & 0xF) + 1;
		} else if(msg == 0xDE) {
			fread(&shortdelay, 1, 1, bin);
			
			shortdelay++;
			targettick += shortdelay;
		} else if(msg == 0xDF) {
			fread(par, 1, 2, bin);

			longdelay = par[0] + (par[1] << 7) + 1;
			targettick += longdelay;
		} else if(msg == 0xFC) {
			break;
		} else switch(msg & 0xF0) {
			case 0x80:
				fread(par, 1, 2, bin);
				fluid_synth_noteoff(synth, msg & 0xF, par[0]);
				channels--;
				break;

			case 0x90:
				fread(par, 1, 2, bin);
				fluid_synth_noteon(synth, msg & 0xF, par[0], par[1]);
				if(par[1]) channels++; else channels--;
				break;

			case 0xB0:
				fread(par, 1, 2, bin);
				fluid_synth_cc(synth, msg & 0xF, par[0], par[1]);
				break;

			case 0xC0:
				fread(par, 1, 1, bin);
				fluid_synth_program_change(synth, msg & 0xF, par[0]);
				break;

			case 0xE0:
				fread(par, 1, 2, bin);
				fluid_synth_pitch_bend(synth, msg & 0xF, ((int)par[0]) + (((int)par[1]) << 7));
				break;
		}
	}

	delete_fluid_audio_driver(adriver);
	delete_fluid_synth(synth);
	delete_fluid_settings(settings);

	fclose(bin);

	printf(" all done!\n");
}
