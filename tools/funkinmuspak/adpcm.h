/*
 * FNF_PS1 asset tools (extension module)
 * (C) 2021 spicyjpeg
 */

#ifndef _INC_ADPCM_
#define _INC_ADPCM_

#include <cstdint>
#include <algorithm>

namespace adpcm {
	struct Parameters {
		uint32_t bitsPerSample; // 4 or 8
		uint32_t filterID;      // 0-5
		int32_t  shiftFactor;   // 0-12

		inline Parameters(uint32_t bitsPerSample) {
			this->bitsPerSample = bitsPerSample;
			this->filterID      = 0;
			this->shiftFactor   = 0;
		}
		inline Parameters(const Parameters *copyFrom) {
			this->bitsPerSample = copyFrom->bitsPerSample;
			this->filterID      = copyFrom->filterID;
			this->shiftFactor   = copyFrom->shiftFactor;
		}

		inline int32_t getMaxShiftFactor() {
			return 16 - this->bitsPerSample;
		}
		inline size_t getEncodedSize(size_t numSamples) {
			//return numSamples * 8 / this->bitsPerSample;

			if (this->bitsPerSample == 4)
				return numSamples / 2;
			else
				return numSamples;
		}
		inline bool sampleClips(int32_t sample) {
			//int maxAbsValue = 1 << (bitsPerSample - 1);
			//return (sample < -maxAbsValue) || (sample > (maxAbsValue - 1));

			if (this->bitsPerSample == 4)
				return (sample < -8) || (sample > 7);
			else
				return (sample < -128) || (sample > 127);
		}
		inline int32_t clipSample(int32_t sample) {
			//int maxAbsValue = 1 << (bitsPerSample - 1);
			//return std::min(std::max(sample, -maxAbsValue), maxAbsValue - 1);

			if (this->bitsPerSample == 4)
				return std::min(std::max(sample, -8), 7);
			else
				return std::min(std::max(sample, -128), 127);
		}
		inline uint8_t toBlockHeader() {
			return static_cast<uint8_t>(this->shiftFactor | (this->filterID << 4));
		}
	};

	struct FilterCoefficients {
		int32_t a1, a2;
	};

	struct FilterState {
		int32_t s1, s2;

		inline FilterState() {
			this->s1 = 0;
			this->s2 = 0;
		}
		inline FilterState(const FilterState *copyFrom) {
			if (copyFrom) {
				this->s1 = copyFrom->s1;
				this->s2 = copyFrom->s2;
			} else {
				this->s1 = 0;
				this->s2 = 0;
			}
		}

		inline int32_t convolve(const FilterCoefficients &filter) {
			return ((this->s1 * filter.a1) + (this->s2 * filter.a2) + 32) >> 6;
		}
		inline void update(int32_t s0) {
			this->s2 = this->s1;
			this->s1 = s0;
		}
	};

	enum class FilterSet : uint8_t {
		CDXA = 4,
		SPU  = 5
	};

	int32_t getShiftFactor(
		size_t        numSamples,
		const int16_t *samples,
		Parameters    *params,
		FilterState   *filterState
	);

	uint64_t tryEncode(
		size_t        numSamples,
		const int16_t *samples,
		uint8_t       *outputBuffer,
		Parameters    *params,
		FilterState   *filterState
	);

	Parameters encode(
		size_t        numSamples,
		const int16_t *samples,
		uint8_t       *outputBuffer = NULL,
		uint32_t      bitsPerSample = 4,
		FilterSet     numFilters    = adpcm::FilterSet::SPU,
		FilterState   *filterState  = NULL
	);
}

namespace spu {
	const uint32_t BLOCK_LENGTH      = 16;
	const uint32_t BLOCK_NUM_SAMPLES = 28;

	enum LoopFlags : uint8_t {
		LOOP           = 0x1, // Jump to the last block which had SET_LOOP_POINT set
		SUSTAIN        = 0x2, // Do not mute after jumping (effectively allowing looping)
		SET_LOOP_POINT = 0x4  // Set the block (usually the first one) as the loop point
	};

	void encodeBlock(
		const int16_t      *samples,      // 28 samples
		uint8_t            *outputBuffer, // 16 bytes
		uint8_t            loopFlags,
		adpcm::FilterState *filterState
	);

	uint32_t getNumBlocks(uint32_t numSamples);

	uint32_t encodeSound(
		const int16_t *samples,
		uint8_t       *outputBuffer,
		uint32_t      numSamples,
		uint32_t      loopPoint      // Set to >=numSamples to disable looping
	);
}

#endif
