/***********************************************************************************************************************
*                                                                                                                      *
* libscopehal v0.1                                                                                                     *
*                                                                                                                      *
* Copyright (c) 2012-2022 Andrew D. Zonenberg and contributors                                                         *
* All rights reserved.                                                                                                 *
*                                                                                                                      *
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the     *
* following conditions are met:                                                                                        *
*                                                                                                                      *
*    * Redistributions of source code must retain the above copyright notice, this list of conditions, and the         *
*      following disclaimer.                                                                                           *
*                                                                                                                      *
*    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the       *
*      following disclaimer in the documentation and/or other materials provided with the distribution.                *
*                                                                                                                      *
*    * Neither the name of the author nor the names of any contributors may be used to endorse or promote products     *
*      derived from this software without specific prior written permission.                                           *
*                                                                                                                      *
* THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED   *
* TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL *
* THE AUTHORS BE HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES        *
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR       *
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE       *
* POSSIBILITY OF SUCH DAMAGE.                                                                                          *
*                                                                                                                      *
***********************************************************************************************************************/

/**
	@file
	@author Andrew D. Zonenberg
	@brief Unit test for Convert16BitSamples primitive
 */
#include <catch2/catch_all.hpp>

#include "../../lib/scopehal/scopehal.h"
#include "../../lib/scopehal/TestWaveformSource.h"
#include "../../lib/scopeprotocols/scopeprotocols.h"
#include "Primitives.h"

using namespace std;

TEST_CASE("Primitive_Convert16BitSamples")
{
	#ifdef __x86_64__
	bool reallyHasAvx2 = g_hasAvx2;
	bool reallyHasFMA = g_hasFMA;
	bool reallyHasAvx512F = g_hasAvx512F;
	#endif

	//Create a queue and command buffer
	vk::CommandPoolCreateInfo poolInfo(
		vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		g_computeQueueType );
	vk::raii::CommandPool pool(*g_vkComputeDevice, poolInfo);

	vk::CommandBufferAllocateInfo bufinfo(*pool, vk::CommandBufferLevel::ePrimary, 1);
	vk::raii::CommandBuffer cmdbuf(move(vk::raii::CommandBuffers(*g_vkComputeDevice, bufinfo).front()));
	vk::raii::Queue queue(*g_vkComputeDevice, g_computeQueueType, 0);

	AcceleratorBuffer<int16_t> data_in;
	AcceleratorBuffer<float> data_out;
	AcceleratorBuffer<float> data_out_golden;

	data_in.SetCpuAccessHint(AcceleratorBuffer<int16_t>::HINT_LIKELY);
	data_in.SetGpuAccessHint(AcceleratorBuffer<int16_t>::HINT_LIKELY);
	data_out.SetCpuAccessHint(AcceleratorBuffer<float>::HINT_LIKELY);
	data_out.SetGpuAccessHint(AcceleratorBuffer<float>::HINT_LIKELY);
	data_out_golden.SetCpuAccessHint(AcceleratorBuffer<float>::HINT_LIKELY);
	data_out_golden.SetGpuAccessHint(AcceleratorBuffer<float>::HINT_LIKELY);

	const size_t wavelen = 1000000;
	data_in.resize(wavelen);
	data_out.resize(wavelen);
	data_out_golden.resize(wavelen);

	uniform_real_distribution<float> gaindesc(0, 1);
	uniform_int_distribution<int16_t> indesc(-32768, 32767);
	uniform_real_distribution<float> offdesc(-10, 10);

	unique_ptr<ComputePipeline> pipe;
	if(g_hasShaderInt16)
	{
		pipe = make_unique<ComputePipeline>(
			"shaders/Convert16BitSamples.spv", 2, sizeof(ConvertRawSamplesShaderArgs) );
	}

	const size_t niter = 8;
	for(size_t i=0; i<niter; i++)
	{
		SECTION(string("Iteration ") + to_string(i))
		{
			LogVerbose("Iteration %zu\n", i);
			LogIndenter li;

			//Generate a random sequence of input
			float gain = gaindesc(g_rng);
			float off = offdesc(g_rng);
			data_in.PrepareForCpuAccess();
			for(size_t j=0; j<wavelen; j++)
				data_in[j] = indesc(g_rng);
			data_in.MarkModifiedFromCpu();
			data_in.PrepareForGpuAccess();

			//Baseline with CPU reference implementation
			#ifdef __x86_64__
				g_hasAvx2 = false;
				g_hasFMA = false;
				g_hasAvx512F = false;
			#endif
			data_out_golden.PrepareForCpuAccess();
			double start = GetTime();
			Oscilloscope::Convert16BitSamplesGeneric(&data_out_golden[0], &data_in[0], gain, off, wavelen);
			double tbase = GetTime() - start;

			data_out_golden.MarkModifiedFromCpu();
			data_out_golden.PrepareForCpuAccess();

			LogVerbose("CPU (no AVX)  : %6.2f ms\n", tbase * 1000);

			#ifdef __x86_64__
			if(reallyHasAvx2)
			{
				g_hasAvx2 = true;

				data_out.PrepareForCpuAccess();
				start = GetTime();
				Oscilloscope::Convert16BitSamplesAVX2(&data_out[0], &data_in[0], gain, off, wavelen);
				float dt = GetTime() - start;

				data_out.MarkModifiedFromCpu();
				data_out.PrepareForCpuAccess();
				LogVerbose("CPU (AVX2)    : %6.2f ms, %.2fx speedup\n", dt * 1000, tbase / dt);
				for(size_t j=0; j<wavelen; j++)
					REQUIRE(fabs(data_out_golden[j] - data_out[j]) < 2e-3f);

				if(reallyHasFMA)
				{
					g_hasFMA = true;

					data_out.PrepareForCpuAccess();
					start = GetTime();
					Oscilloscope::Convert16BitSamplesFMA(&data_out[0], &data_in[0], gain, off, wavelen);
					dt = GetTime() - start;

					data_out.MarkModifiedFromCpu();
					data_out.PrepareForCpuAccess();
					LogVerbose("CPU (FMA)     : %6.2f ms, %.2fx speedup\n", dt * 1000, tbase / dt);
					for(size_t j=0; j<wavelen; j++)
						REQUIRE(fabs(data_out_golden[j] - data_out[j]) < 2e-3f);
				}
			}
			if(reallyHasAvx512F)
			{
				g_hasAvx512F = true;

				data_out.PrepareForCpuAccess();
				start = GetTime();
				Oscilloscope::Convert16BitSamplesAVX512F(&data_out[0], &data_in[0], gain, off, wavelen);
				float dt = GetTime() - start;

				data_out.MarkModifiedFromCpu();
				data_out.PrepareForCpuAccess();
				LogVerbose("CPU (AVX512F) : %6.2f ms, %.2fx speedup\n", dt * 1000, tbase / dt);
				for(size_t j=0; j<wavelen; j++)
					REQUIRE(fabs(data_out_golden[j] - data_out[j]) < 2e-3f);
			}
			#endif

			//Vulkan implementation
			if(pipe)
			{
				data_out.PrepareForGpuAccess();
				data_in.PrepareForGpuAccess();

				start = GetTime();
				cmdbuf.begin({});
				pipe->BindBufferNonblocking(0, data_out, cmdbuf, true);
				pipe->BindBufferNonblocking(1, data_in, cmdbuf);
				ConvertRawSamplesShaderArgs args;
				args.size = wavelen;
				args.gain = gain;
				args.offset = off;
				pipe->Dispatch(cmdbuf, args, GetComputeBlockCount(wavelen, 64));
				cmdbuf.end();
				SubmitAndBlock(cmdbuf, queue);
				float dt = GetTime() - start;
				data_out.MarkModifiedFromGpu();

				data_out.PrepareForCpuAccess();
				LogVerbose("GPU           : %6.2f ms, %.2fx speedup\n", dt * 1000, tbase / dt);
				for(size_t j=0; j<wavelen; j++)
					REQUIRE(fabs(data_out_golden[j] - data_out[j]) < 2e-3f);
			}
		}
	}

	#ifdef __x86_64__
		g_hasAvx2 = reallyHasAvx2;
		g_hasFMA = reallyHasFMA;
		g_hasAvx512F = reallyHasAvx512F;
	#endif
}
