/**************************************************************************/
/*  resource_format_text.h                                                */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef RESOURCE_COMPAT_TEXT_H
#define RESOURCE_COMPAT_TEXT_H

#include "compat/resource_import_metadatav2.h"
#include "compat/resource_loader_compat.h"
#include "utility/resource_info.h"

#include "core/io/file_access.h"
#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "core/variant/variant_parser.h"
#include "scene/resources/packed_scene.h"

class ResourceLoaderCompatText {
public:
	enum {
		// Version 2: Changed names for Basis, AABB, Vectors, etc.
		// Version 3: New string ID for ext/subresources, breaks forward compat.
		// Version 4: PackedByteArray can be base64 encoded, and PackedVector4Array was added.
		FORMAT_VERSION = 4,
		// For compat, save as version 3 if not using PackedVector4Array or no big PackedByteArray.
		FORMAT_VERSION_COMPAT = 3,
	};

private:
	bool translation_remapped = false;
	String local_path;
	String res_path;
	String error_text;

	Ref<FileAccess> f;

	VariantParser::StreamFile stream;

	struct ExtResource {
		Ref<ResourceLoader::LoadToken> load_token;
		String path;
		String type;
		ResourceUID::ID uid;
		Ref<Resource> fallback;
	};

	uint32_t ver_major = 0;
	uint32_t ver_minor = 0;

	ResourceInfo::LoadType load_type = ResourceInfo::REAL_LOAD;

	int packed_scene_version = 2; // 2 is the lowest packed scene version that text resources supported
	bool is_scene = false;
	int format_version;
	String res_type;

	bool ignore_resource_parsing = false;

	HashMap<String, ExtResource> ext_resources;
	HashMap<String, Ref<Resource>> int_resources;

	int resources_total = 0;
	int resource_current = 0;
	String resource_type;
	String script_class;

	VariantParser::Tag next_tag;

	ResourceFormatLoader::CacheMode cache_mode = ResourceFormatLoader::CACHE_MODE_REUSE;
	ResourceFormatLoader::CacheMode cache_mode_for_external = ResourceFormatLoader::CACHE_MODE_REUSE;

	bool use_sub_threads = false;
	float *progress = nullptr;

	mutable int lines = 0;

	ResourceUID::ID res_uid = ResourceUID::INVALID_ID;

	HashMap<String, String> remaps;

	static Error _parse_sub_resources(void *p_self, VariantParser::Stream *p_stream, Ref<Resource> &r_res, int &line, String &r_err_str) { return reinterpret_cast<ResourceLoaderCompatText *>(p_self)->_parse_sub_resource(p_stream, r_res, line, r_err_str); }
	static Error _parse_ext_resources(void *p_self, VariantParser::Stream *p_stream, Ref<Resource> &r_res, int &line, String &r_err_str) { return reinterpret_cast<ResourceLoaderCompatText *>(p_self)->_parse_ext_resource(p_stream, r_res, line, r_err_str); }

	Error _parse_sub_resource(VariantParser::Stream *p_stream, Ref<Resource> &r_res, int &line, String &r_err_str);
	Error _parse_ext_resource(VariantParser::Stream *p_stream, Ref<Resource> &r_res, int &line, String &r_err_str);

	void set_internal_resource_compat_meta(const String &p_path, const String &p_scene_id, const String &p_type, Ref<Resource> &r_res);
	void set_compat_meta(Ref<Resource> &r_res);

	// for converter
	class DummyResource : public Resource {
	public:
	};

	struct DummyReadData {
		bool no_placeholders = false;
		HashMap<Ref<Resource>, int> external_resources;
		HashMap<String, Ref<Resource>> rev_external_resources;
		HashMap<Ref<Resource>, int> resource_index_map;
		HashMap<String, Ref<Resource>> resource_map;
	};

	static Error _parse_sub_resource_dummys(void *p_self, VariantParser::Stream *p_stream, Ref<Resource> &r_res, int &line, String &r_err_str) { return _parse_sub_resource_dummy(static_cast<DummyReadData *>(p_self), p_stream, r_res, line, r_err_str); }
	static Error _parse_ext_resource_dummys(void *p_self, VariantParser::Stream *p_stream, Ref<Resource> &r_res, int &line, String &r_err_str) { return _parse_ext_resource_dummy(static_cast<DummyReadData *>(p_self), p_stream, r_res, line, r_err_str); }

	static Error _parse_sub_resource_dummy(DummyReadData *p_data, VariantParser::Stream *p_stream, Ref<Resource> &r_res, int &line, String &r_err_str);
	static Error _parse_ext_resource_dummy(DummyReadData *p_data, VariantParser::Stream *p_stream, Ref<Resource> &r_res, int &line, String &r_err_str);
	void _printerr();

	VariantParser::ResourceParser rp;

	friend class ResourceFormatLoaderCompatText;
	friend class ResourceFormatSaverCompatText;

	Error error = OK;

	Ref<Resource> resource;

	Ref<PackedScene> _parse_node_tag(VariantParser::ResourceParser &parser);

	bool is_real_load() const { return load_type == ResourceInfo::REAL_LOAD || load_type == ResourceInfo::GLTF_LOAD; }

	Ref<ResourceLoader::LoadToken> start_ext_load(const String &p_path, const String &p_type_hint, const ResourceUID::ID uid, const String id);
	Ref<Resource> finish_ext_load(Ref<ResourceLoader::LoadToken> &load_token, Error *r_err);

	void _set_main_resource_info(Ref<ResourceInfo> &info);

public:
	bool should_threaded_load() const;
	Ref<ResourceInfo> get_resource_info();
	Ref<Resource> get_resource();
	Error load();
	Error set_uid(Ref<FileAccess> p_f, ResourceUID::ID p_uid);
	int get_stage() const;
	int get_stage_count() const;
	void set_translation_remapped(bool p_remapped);

	void open(Ref<FileAccess> p_f, bool p_skip_first_tag = false);
	String recognize(Ref<FileAccess> p_f);
	String recognize_script_class(Ref<FileAccess> p_f);
	ResourceUID::ID get_uid(Ref<FileAccess> p_f);
	void get_dependencies(Ref<FileAccess> p_f, List<String> *p_dependencies, bool p_add_types);
	Error rename_dependencies(Ref<FileAccess> p_f, const String &p_path, const HashMap<String, String> &p_map);
	Error get_classes_used(HashSet<StringName> *r_classes);

	Error save_as_binary(const String &p_path);
	ResourceLoaderCompatText();
};

class ResourceFormatLoaderCompatText : public CompatFormatLoader {
	GDCLASS(ResourceFormatLoaderCompatText, CompatFormatLoader);

public:
	static ResourceFormatLoaderCompatText *singleton;
	virtual Ref<Resource> load(const String &p_path, const String &p_original_path = "", Error *r_error = nullptr, bool p_use_sub_threads = false, float *r_progress = nullptr, CacheMode p_cache_mode = CACHE_MODE_REUSE) override;
	virtual void get_recognized_extensions_for_type(const String &p_type, List<String> *p_extensions) const override;
	virtual void get_recognized_extensions(List<String> *p_extensions) const override;
	virtual bool handles_type(const String &p_type) const override;
	virtual void get_classes_used(const String &p_path, HashSet<StringName> *r_classes) override;

	virtual String get_resource_type(const String &p_path) const override;
	virtual String get_resource_script_class(const String &p_path) const override;
	virtual ResourceUID::ID get_resource_uid(const String &p_path) const override;
	virtual bool has_custom_uid_support() const override;
	virtual void get_dependencies(const String &p_path, List<String> *p_dependencies, bool p_add_types = false) override;
	virtual Error rename_dependencies(const String &p_path, const HashMap<String, String> &p_map) override;

	virtual Ref<Resource> custom_load(const String &p_path, const String &p_original_path, ResourceInfo::LoadType p_type, Error *r_error = nullptr, bool use_threads = true, ResourceFormatLoader::CacheMode p_cache_mode = CACHE_MODE_REUSE) override;
	virtual Ref<ResourceInfo> get_resource_info(const String &p_path, Error *r_error) const override;
	virtual bool handles_fake_load() const override { return true; }

	ResourceFormatLoaderCompatText() { singleton = this; }
};

class ResourceFormatSaverCompatTextInstance {
	String local_path;

	Ref<PackedScene> packed_scene;

	bool takeover_paths = false;
	bool relative_paths = false;
	bool bundle_resources = false;
	bool skip_editor = false;

	int format_version;
	uint32_t ver_major;
	uint32_t ver_minor;

	struct NonPersistentKey { //for resource properties generated on the fly
		Ref<Resource> base;
		StringName property;
		bool operator<(const NonPersistentKey &p_key) const { return base == p_key.base ? property < p_key.property : base < p_key.base; }
	};

	RBMap<NonPersistentKey, Variant> non_persistent_map;

	HashSet<Ref<Resource>> resource_set;
	List<Ref<Resource>> saved_resources;
	HashMap<Ref<Resource>, String> external_resources;
	HashMap<Ref<Resource>, String> internal_resources;
	bool use_compat = true;

	String script_class;
	ResourceUID::ID res_uid;

	struct ResourceSort {
		Ref<Resource> resource;
		String id;
		bool operator<(const ResourceSort &p_right) const {
			return id.naturalnocasecmp_to(p_right.id) < 0;
		}
	};

	void _find_resources(const Variant &p_variant, bool p_main = false);

	static String _write_resources(void *ud, const Ref<Resource> &p_resource);
	String _write_resource(const Ref<Resource> &res);
	String get_id_for_ext_resource(Ref<Resource> res, int ext_resources_size);
	void ensure_ids_are_unique();
	Error set_save_settings(const Ref<Resource> &p_resource, uint32_t p_flags);

public:
	Error save(const String &p_path, const Ref<Resource> &p_resource, uint32_t p_flags = 0);
};

class ResourceFormatSaverCompatText : public ResourceFormatSaver {
public:
	static ResourceFormatSaverCompatText *singleton;
	virtual Error save(const Ref<Resource> &p_resource, const String &p_path, uint32_t p_flags = 0) override;
	virtual Error set_uid(const String &p_path, ResourceUID::ID p_uid) override;
	virtual bool recognize(const Ref<Resource> &p_resource) const override;
	virtual void get_recognized_extensions(const Ref<Resource> &p_resource, List<String> *p_extensions) const override;

	static int get_default_format_version(int ver_major, int ver_minor);

	static int get_ver_major_from_format_version(int ver_format);
	static int get_ver_minor_from_format_version(int ver_format);

	Error save_custom(const Ref<Resource> &p_resource, const String &p_path, int ver_format, int ver_major, int ver_minor, uint32_t p_flags = 0);

	ResourceFormatSaverCompatText();
};

#endif // RESOURCE_COMPAT_TEXT_H
