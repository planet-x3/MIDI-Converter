#!/usr/bin/env python3

# Parameters: input_csv output_bin frequency
# Does not perform any validation checks on the input data.
# Expects sorted output of midicsv with spaces and the first column removed.

import sys


# Function that performs rudimentary MT-32 to General MIDI instrument mapping
# Only intended for "Apprehension" and "Valiant fighters"
def prog_gm(n):
	mappings = {
		24: 60,
		29: 122,
		44: 61,
		45: 28,
		47: 94,
		63: 88,
		65: 16,
		66: 32,
		104: 13,
		105: 14,
		109: 36
	}
	return mappings[n] if n in mappings else n


csv = open(sys.argv[1], "r", errors='replace')
entries = csv.read().splitlines()
csv.close()

bin = open(sys.argv[2], "wb")
bin.write(bytes([int(sys.argv[3])]))

convert_from_mt32 = int(sys.argv[3]) == 5 and sys.argv[4] == "MT-32"

# Read the time division

division = 0

for entry in entries:
	split = entry.split(",")

	if split[1] == "Header":
		division = int(split[4])
		break

# Read the initial tempo

tempo = 0

for entry in entries:
	split = entry.split(",")

	if split[1] == "Tempo":
		tempo = int(split[2])
		break

# Start the conversion

print("Time division: %d, initial tempo: %d" % (division, tempo))
wantedtick = 0
step = (float(division) * (1000000.0 / float(tempo))) / 72.82

#noteon = 0
#skipstartsilence = sys.argv[4].lower() in [ "true", "1", "t", "y", "yes" ]

for entry in entries:
	split = entry.split(",")

	# Convert MIDI time to system time

	delayticks = 0

	while int(split[0]) > wantedtick:
		wantedtick += step
		delayticks += 1

#	if noteon or skipstartsilence:
	if delayticks > 0 and delayticks <= 14:
		bin.write(bytes([0xD0 + delayticks - 1]))

	elif delayticks > 14 and delayticks <= 128:
		bin.write(bytes([0xDE, delayticks - 1]))

	elif delayticks > 128 and delayticks <= 16384:
		bin.write(bytes([0xDF, (delayticks - 1) & 0x7F, (delayticks - 1) >> 7]))

	elif delayticks > 16384:
		print("You're nuts.")
		exit(1)

	# Encode MIDI messages into binary

	if split[1] == "Tempo":
		tempo = int(split[2])
		print("\nSet tempo to %d" % (tempo))

		step = (float(division) * (1000000.0 / float(tempo))) / 72.82
		print("MIDI ticks per system tick: %f" % (step))

	elif split[1] == "Note_off_c":
		bin.write(bytes([0x80 + int(split[2]), int(split[3]), int(split[4])]))

	elif split[1] == "Note_on_c":
		bin.write(bytes([0x90 + int(split[2]), int(split[3]), int(split[4])]))
#		noteon = 1

	elif split[1] == "Control_c":
		bin.write(bytes([0xB0 + int(split[2]), int(split[3]), int(split[4])]))

	elif split[1] == "Program_c":
		bin.write(bytes([0xC0 + int(split[2]), prog_gm(int(split[3])) if convert_from_mt32 else int(split[3])]))

	elif split[1] == "Pitch_bend_c":
		bin.write(bytes([0xE0 + int(split[2]), int(split[3]) & 0x7F, int(split[3]) >> 7]))

	elif split[1] == "System_exclusive":
		bin.write(bytes([0xF0]))

		for i in range(int(split[2])):
			bin.write(bytes([int(split[3 + i])]))

		print(split)

bin.write(bytes([0xFC]))

bin.close()
