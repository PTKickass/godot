#include "core/io/dir_access_hybrid.h"
#include "core/io/file_access_pack.h"
#if defined(WINDOWS_ENABLE)
	#include "drivers/windows/dir_access_windows.h"
#elif defined(UNIX_ENABLE)
	#include "drivers/unix/dir_access_unix.h"
#endif

Error DirAccessHybrid::list_dir_begin() {

	String path;

	if (err_os == OK) {
		dir_access_os->list_dir_begin();

		while (!(path = dir_access_os->get_next()).empty()) {

			if (dir_access_os->current_is_dir()) {

				if (!list_dirs.find(path))
					list_dirs.push_back(path);
			} else {

				if (!list_files.find(path))
					list_files.push_back(path);
			}
		}

		dir_access_os->list_dir_end();
	}

	if (err_pack == OK) {
		dir_access_pack->list_dir_begin();

		while (!(path = dir_access_pack->get_next()).empty()) {

			if (dir_access_pack->current_is_dir()) {

				if (!list_dirs.find(path))
					list_dirs.push_back(path);
			} else {

				if (!list_files.find(path))
					list_files.push_back(path);
			}
		}

		dir_access_pack->list_dir_end();
	}

	return OK;
}

String DirAccessHybrid::get_next() {

	if (!list_dirs.empty()) {
		cdir = true;
		String d = list_dirs.front()->get();
		list_dirs.pop_front();
		return d;
	} else if (!list_files.empty()) {
		cdir = false;
		String f = list_files.front()->get();
		list_files.pop_front();
		return f;
	} else {
		return String();
	}
}

bool DirAccessHybrid::current_is_dir() const {

	return cdir;
}

bool DirAccessHybrid::current_is_hidden() const {

	return false;
}

void DirAccessHybrid::list_dir_end() {

	list_dirs.clear();
	list_files.clear();
}

int DirAccessHybrid::get_drive_count() {

	return dir_access_os->get_drive_count();
}

String DirAccessHybrid::get_drive(int p_drive) {

	return dir_access_os->get_drive(p_drive);
}

Error DirAccessHybrid::change_dir(String p_dir) {

	dir_access_os->set_access_type(get_access_type());
	dir_access_pack->set_access_type(get_access_type());

	err_pack = dir_access_pack->change_dir(p_dir);

	if (err_pack == OK) {
		err_os = dir_access_os->change_dir(dir_access_pack->get_current_dir());
	} else {
		err_os = dir_access_os->change_dir(p_dir);
		if (err_os == OK) {
			err_pack = dir_access_pack->change_dir(dir_access_os->get_current_dir());
		}
	}

	return err_os == OK || err_pack == OK ? OK : ERR_INVALID_PARAMETER;
}

String DirAccessHybrid::get_current_dir() {

	return err_os == OK	  ? dir_access_os->get_current_dir() :
		   err_pack == OK ? dir_access_pack->get_current_dir() :
							  "";
}

Error DirAccessHybrid::make_dir(String p_dir) {

	return dir_access_os->make_dir(p_dir);
}

bool DirAccessHybrid::file_exists(String p_file) {

	bool exist = false;

	if (err_os == OK)
		exist = dir_access_os->file_exists(p_file);

	if (!exist && err_pack == OK)
		exist = dir_access_pack->file_exists(p_file);

	return exist;
}

bool DirAccessHybrid::dir_exists(String p_dir) {

	bool exist = false;

	if (err_os == OK)
		exist = dir_access_os->dir_exists(p_dir);

	if (!exist && err_pack == OK)
		exist = dir_access_pack->dir_exists(p_dir);

	return exist;
}

size_t DirAccessHybrid::get_space_left() {

	return dir_access_os->get_space_left();
}

Error DirAccessHybrid::rename(String p_from, String p_to) {

	return dir_access_os->rename(p_from, p_to);
}

Error DirAccessHybrid::remove(String p_name) {

	return dir_access_os->remove(p_name);
}

String DirAccessHybrid::get_filesystem_type() const {

	return dir_access_os->get_filesystem_type();
}

DirAccessHybrid::DirAccessHybrid() :
#if defined(WINDOWS_ENABLE)
		dir_access_os(memnew(DirAccessWindows)),
#elif defined(UNIX_ENABLED)
		dir_access_os(memnew(DirAccessUnix)),
#elif
		// ??? IDK I don't care about Apple systems
		// so I won't add that.
		// In fact, screw Tim Cook:
		*(float*)1987 = 69;
#endif
		dir_access_pack(memnew(DirAccessPack)),
		err_os(OK),
		err_pack(OK),
		cdir(false) {
}

DirAccessHybrid::~DirAccessHybrid() {

	if (dir_access_os)
		memdelete(dir_access_os);

	if (dir_access_pack)
		memdelete(dir_access_pack);
}
