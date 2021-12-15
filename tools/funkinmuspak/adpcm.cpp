/*
 * FNF_PS1 asset tools (extension module)
 * (C) 2021 spicyjpeg
 *
 * https://problemkaputt.de/psx-spx.htm#cdromxaaudioadpcmcompression
 */

#include <cstdint>
#include <algorithm>
#include <cmath>
#include "adpcm.h"

// List of filter coefficients associated with each filter ID. These are the
// values used internally by the SPU (fixed-point?) and must be divided by 64
// to obtain the "actual" coefficients.
static const adpcm::FilterCoefficients ADPCM_FILTERS[] = {
	{   0,   0 },
	{  60,   0 },
	{ 115, -52 },
	{  98, -55 },
	{ 122, -60 }  // This one can only be used in SPU ADPCM
};

/* Low-level ADPCM encoder */

int32_t adpcm::getShiftFactor(
	size_t             numSamples,
	const int16_t      *samples,
	adpcm::Parameters  *params,
	adpcm::FilterState *filterState
) {
	auto &filter = ADPCM_FILTERS[params->filterID];

	adpcm::FilterState tempState(filterState);
	int32_t            minSample = 0;
	int32_t            maxSample = 0;

	// Determine the minimum and maximum sample for this block.
	for (size_t pos = 0; pos < numSamples; pos++) {
		int32_t sample       = samples[pos];
		int32_t filterOutput = tempState.convolve(filter);

		int32_t filtered = sample - filterOutput;
		if (filtered < minSample)
			minSample = filtered;
		if (filtered > maxSample)
			maxSample = filtered;

		tempState.update(sample);
	}

	int32_t maxShift = params->getMaxShiftFactor();
	int32_t shift    = 0;

	// Increment (well, actually decrement) the shift factor until the samples
	// no longer clip.
	while ((shift < maxShift) && params->sampleClips(maxSample >> shift))
		shift++;
	while ((shift < maxShift) && params->sampleClips(minSample >> shift))
		shift++;

	return maxShift - shift;
}

uint64_t adpcm::tryEncode(
	size_t             numSamples,
	const int16_t      *samples,
	uint8_t            *outputBuffer,
	adpcm::Parameters  *params,
	adpcm::FilterState *filterState
) {
	auto &filter = ADPCM_FILTERS[params->filterID];

	int32_t  maxShift = params->getMaxShiftFactor();
	uint64_t error    = 0;

	for (size_t pos = 0; pos < numSamples; pos++) {
		//int32_t sample       = samples[pos] + filterState->quantError;
		int32_t sample       = samples[pos];
		int32_t filterOutput = filterState->convolve(filter);

		int32_t filtered = sample - filterOutput;
		filtered       <<= params->shiftFactor;
		filtered        += 1 << (maxShift - 1);
		filtered       >>= maxShift;

		int32_t encoded = params->clipSample(filtered);

		// TODO: why is this needed?
		if (params->bitsPerSample == 4)
			encoded &= 0xf;
		else
			encoded &= 0xff;

		// Decode the freshly encoded sample back into a PCM sample (so we can
		// calculate how different it is from the original one).
		int16_t expanded = encoded << maxShift;
		int32_t decoded  = expanded >> params->shiftFactor;
		decoded         += filterOutput;
		decoded          = std::min(std::max(decoded, -32768), 32767);

		// Calculate the difference between the original and decoded samples
		// and add it to the mean square error.
		int64_t _error = decoded - sample;
		error         += static_cast<uint64_t>(_error * _error);

		//filterState->quantError += _error;
		filterState->update(decoded);

		// Finally write the sample to the buffer. If the bit width is 4, pack
		// two samples (nibbles) into each byte.

		if (params->bitsPerSample == 4) {
			// Write the nibble to the appropriate nibble, depending on whether
			// the current sample is odd or even (the SPU plays lower nibbles
			// first).
			uint8_t &output = outputBuffer[pos / 2];

			if (!(pos & 1))
				output  = static_cast<uint8_t>(encoded);
			else
				output |= static_cast<uint8_t>(encoded << 4);
		} else {
			outputBuffer[pos] = static_cast<uint8_t>(encoded);
		}
	}

	return error;
}

adpcm::Parameters adpcm::encode(
	size_t             numSamples,
	const int16_t      *samples,
	uint8_t            *outputBuffer,
	uint32_t           bitsPerSample,
	adpcm::FilterSet   numFilters,
	adpcm::FilterState *filterState
) {
	adpcm::Parameters params(bitsPerSample);
	adpcm::Parameters finalParams(bitsPerSample);
	uint64_t          bestError = 0xffffffffffffffff;

	// Find the best filter by brute-forcing all of them.
	for (
		params.filterID = 0;
		params.filterID <static_cast<uint32_t>(numFilters);
		params.filterID++
	) {
		// Calculate what shift factor would be the "ideal" one when using this
		// filter.
		int32_t shiftBase = adpcm::getShiftFactor(
			numSamples,
			samples,
			&params,
			filterState
		);

		// Try other shift factors in a +/-1 range. According to comments in
		// the original code, sometimes the actual "best" shift factor can be
		// off by 1.
		for (
			params.shiftFactor  = std::max(shiftBase - 1, 0);
			params.shiftFactor <= std::min(shiftBase + 1, params.getMaxShiftFactor());
			params.shiftFactor++
		) {
			// Create a copy of the filter state to ensure all tries start with
			// the same initial state.
			adpcm::FilterState tempState(filterState);

			uint64_t error = adpcm::tryEncode(
				numSamples,
				samples,
				outputBuffer, // We'll overwrite it later anyway
				&params,
				&tempState
			);

			// If this is the lowest mean square error yet, save the parameters
			// so they can be used later.
			if (error < bestError) {
				bestError   = error;
				finalParams = adpcm::Parameters(&params);
			}
		}
	}

	adpcm::tryEncode(
		numSamples,
		samples,
		outputBuffer,
		&finalParams,
		filterState
	);

	return finalParams;
}

/* SPU block encoder */

void spu::encodeBlock(
	const int16_t      *samples,      // 28 samples
	uint8_t            *outputBuffer, // 16 bytes
	uint8_t            loopFlags,
	adpcm::FilterState *filterState
) {
	adpcm::Parameters params = adpcm::encode(
		spu::BLOCK_NUM_SAMPLES,
		samples,
		&outputBuffer[2],
		4,
		adpcm::FilterSet::SPU,
		filterState
	);

	outputBuffer[0] = params.toBlockHeader();
	outputBuffer[1] = loopFlags;
}

uint32_t spu::getNumBlocks(uint32_t numSamples) {
	uint32_t numBlocks = numSamples / spu::BLOCK_NUM_SAMPLES;

	// If sample length is not an exact multiple of 28, add an extra block
	// (which will be partially empty).
	if (numSamples % spu::BLOCK_NUM_SAMPLES)
		numBlocks++;

	return numBlocks;
}

uint32_t spu::encodeSound(
	const int16_t *samples,
	uint8_t       *outputBuffer,
	uint32_t      numSamples,
	uint32_t      loopPoint      // Set to numSamples to disable looping
) {
	adpcm::FilterState filterState;

	uint32_t numBlocks = spu::getNumBlocks(numSamples);
	uint32_t loopBlock = loopPoint / spu::BLOCK_NUM_SAMPLES;

	for (uint32_t block = 0; block < numBlocks; block++) {
		uint32_t sampleOffset = block * spu::BLOCK_NUM_SAMPLES;
		uint32_t blockOffset  = block * spu::BLOCK_LENGTH;
		uint32_t length       = numSamples - sampleOffset;

		uint8_t loopFlags = 0;
		if (block == loopBlock)
			loopFlags |= spu::LoopFlags::SET_LOOP_POINT;
		if (block == (numBlocks - 1)) {
			loopFlags |= spu::LoopFlags::LOOP;

			// Only set the "sustain" (i.e. do not mute after looping) flag if
			// the sound is actually meant to loop.
			if (loopBlock < numBlocks)
				loopFlags |= spu::LoopFlags::SUSTAIN;
		}

		if (length >= spu::BLOCK_NUM_SAMPLES) {
			// Read samples directly from the input.
			spu::encodeBlock(
				&samples[sampleOffset],
				&outputBuffer[blockOffset],
				loopFlags,
				&filterState
			);
		} else {
			// Pad by copying the last samples into a temporary buffer.
			int16_t padBuffer[spu::BLOCK_NUM_SAMPLES];

			for (uint32_t pos = 0; pos < spu::BLOCK_NUM_SAMPLES; pos++) {
				if (pos < length)
					padBuffer[pos] = samples[sampleOffset + pos];
				else
					padBuffer[pos] = 0;
			}

			spu::encodeBlock(
				padBuffer,
				&outputBuffer[blockOffset],
				loopFlags,
				&filterState
			);
		}
	}

	return numBlocks * spu::BLOCK_LENGTH;
}
