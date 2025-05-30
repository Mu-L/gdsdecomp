#include "sample_loader_compat.h"
#include "scene/resources/audio_stream_wav.h"

Ref<Resource> SampleConverterCompat::convert(const Ref<MissingResource> &res, ResourceInfo::LoadType p_type, int ver_major, Error *r_error) {
	String name = get_resource_name(res, ver_major);
	AudioStreamWAV::LoopMode loop_mode = (AudioStreamWAV::LoopMode) int(res->get("loop_format"));
	// int loop_begin, loop_end, mix_rate, data_bytes = 0;
	int loop_begin = res->get("loop_begin");
	int loop_end = res->get("loop_end");
	bool stereo = res->get("stereo");

	int mix_rate = res->get("mix_rate");
	if (!mix_rate) {
		mix_rate = 44100;
	}
	Vector<uint8_t> data;
	AudioStreamWAV::Format format = (AudioStreamWAV::Format) int(res->get("format"));
	int data_bytes = 0;
	Dictionary dat = res->get("data");
	if (dat.is_empty()) {
		data = res->get("data");
	} else {
		String fmt = dat.get("format", "");
		if (fmt == "ima_adpcm") {
			format = AudioStreamWAV::FORMAT_IMA_ADPCM;
		} else if (fmt == "pcm16") {
			format = AudioStreamWAV::FORMAT_16_BITS;
		} else if (fmt == "pcm8") {
			format = AudioStreamWAV::FORMAT_8_BITS;
		}
		data = dat.get("data", Vector<uint8_t>());
		if (dat.has("stereo")) {
			stereo = dat["stereo"];
		}
		if (dat.has("length")) {
			data_bytes = dat["length"];
		}
	}
	if (data_bytes == 0) {
		data_bytes = data.size();
	}
	// else if (data_bytes != data.size()) {
	// 	 TODO: something?
	// 	 WARN_PRINT("Data size mismatch: " + itos(data_bytes) + " vs " + itos(data.size()));
	// }
	// create a new sample
	Ref<AudioStreamWAV> sample = memnew(AudioStreamWAV);
	sample->set_name(name);
	sample->set_data(data);
	sample->set_format(format);
	sample->set_loop_mode(loop_mode);
	sample->set_stereo(stereo);
	sample->set_loop_begin(loop_begin);
	sample->set_loop_end(loop_end);
	sample->set_mix_rate(mix_rate);
	auto info = ResourceInfo::get_info_from_resource(res);
	ERR_FAIL_COND_V_MSG(!info.is_valid(), res, "Missing resource has no compat metadata??????????? This should have been set by the missing resource instance function(s)!!!!!!!!");
	info->set_on_resource(sample);
	return sample;
}

bool SampleConverterCompat::handles_type(const String &p_type, int ver_major) const {
	return (p_type == "AudioStreamWAV" || p_type == "AudioStreamSample" || p_type == "Sample") && ver_major < 4;
}