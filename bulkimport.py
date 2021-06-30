import sys
import os
import subprocess
import zipfile
import shutil
import io
import json

debug_preserve_osx_dir = False
debug_preserve_temps = True
debug_preserve_resources = True
debug_preserve_qt = False


def invoke_command(process_path, args, output_lines=None):
	print("Running " + str(process_path) + " with " + str(args))
	args_concatenated = [process_path + ".exe"] + args
	if output_lines != None:
		completed_process = subprocess.run(args_concatenated, capture_output=True)
	else:
		completed_process = subprocess.run(args_concatenated)

	if completed_process.returncode != 0:
		print("Process crashed/failed with return code " + str(completed_process.returncode))
		return False

	if output_lines != None:
		output_lines.clear()
		output_lines.extend(completed_process.stdout.decode("utf-8", "ignore").splitlines(False))

	return True

def recursive_scan_dir(out_paths, dir_path):
	dir = os.scandir(dir_path)
	for entry in dir:
		if entry.is_dir():
			recursive_scan_dir(out_paths, entry.path)
		if entry.is_file():
			out_paths.append(entry.path)

def decompress_zip(ftagdata_path, source_path, ts_path, decompress_path):
	with zipfile.ZipFile(source_path, "r") as zfile:
		zfile.extractall(decompress_path)

	file_names = []
	recursive_scan_dir(file_names, decompress_path)

	for path in file_names:
		if not invoke_command(ftagdata_path, [ts_path, path + ".gpf", "DATA", "DATA", "0", "0"]):
			return False
		os.replace(path, path + ".gpd")

	return True

def fixup_macos_dir(ftagdata_path, asadtool_path, ts_path, dir_path, osx_path):
	contents = []
	recursive_scan_dir(contents, osx_path)

	print("recursive_scan_dir results: " + str(contents))
	for content_path in contents:
		osx_rel_path = os.path.relpath(content_path, osx_path)
		osx_rel_dir, osx_rel_file = os.path.split(osx_rel_path)

		if osx_rel_file.startswith("._") and osx_rel_file.endswith(".gpd"):
			out_path = os.path.join(dir_path, osx_rel_dir, osx_rel_file[2:-4])
			if not invoke_command(asadtool_path, [content_path, ts_path, out_path]):
				return False

	return True

def recursive_fixup_macosx_dir(ftagdata_path, asadtool_path, ts_path, dir_path):
	osx_path = os.path.join(dir_path, "__MACOSX")
	if os.path.isdir(osx_path):
		if not fixup_macos_dir(ftagdata_path, asadtool_path, ts_path, dir_path, osx_path):
			print("fixup_macos_dir failed?")
			return False
		if not debug_preserve_osx_dir:
			shutil.rmtree(osx_path)

	dir = os.scandir(dir_path)
	for entry in dir:
		if entry.is_dir():
			if not recursive_fixup_macosx_dir(ftagdata_path, asadtool_path, ts_path, entry.path):
				return False

	return True

def convert_movies(tools_dir, dir_path):
	contents = []
	recursive_scan_dir(contents, dir_path)
	for content_path in contents:
		print("convert_movies content path: " + content_path)
		if content_path.endswith(".mov.gpf"):
			if not os.path.isfile(content_path[:-4] + ".gpd"):
				# Res-only movie, probably only contains external references, a.k.a. unusable
				os.remove(content_path)
				if os.path.isfile(content_path[:-4] + ".gpr"):
					os.remove(content_path[:-4] + ".gpr")
			else:
				content_dir = os.path.dirname(content_path)
				mov_path = content_path[:-4]
				res_path = mov_path + ".gpr"
				data_path = mov_path + ".gpd"
				if os.path.isfile(res_path):
					if not invoke_command(os.path.join(tools_dir, "flattenmov"), [data_path, res_path, mov_path]):
						return False

					if not debug_preserve_qt:
						os.remove(res_path)
						os.remove(data_path)
				else:
					if os.path.isfile(mov_path):
						os.remove(mov_path)
					os.rename(data_path, mov_path)

				probe_lines = []
				if not invoke_command(os.path.join(tools_dir, "ffprobe"), ["-show_streams", mov_path], probe_lines):
					return False

				v_index = None
				v_fps_num = None
				v_fps_denom = None

				a_index = None
				a_nbframes = None
				a_sample_rate = None

				current_fps = None
				current_index = None
				current_type = None
				current_nbframes = None
				current_sample_rate = None
				is_stream = False
				for l in probe_lines:
					if is_stream:
						if l == "[/STREAM]":
							print("Closing stream: " + str(current_type) + " " + str(current_index) + " " + str(current_fps) + " " + str(current_nbframes) + " " + str(current_sample_rate))
							if current_type == "video" and current_index != None and current_fps != None:
								fps_list = current_fps.split("/")
								v_index = current_index
								v_fps_num = fps_list[0]
								v_fps_denom = fps_list[1]
							if current_type == "audio" and current_index != None and current_nbframes != None and current_sample_rate != None:
								a_index = current_index
								a_nbframes = current_nbframes
								a_sample_rate = current_sample_rate
							
							current_fps = None
							current_index = None
							current_type = None
							current_nbframes = None
							current_sample_rate = None
							is_stream = False
						elif l.startswith("codec_type="):
							current_type = l[11:]
						elif l.startswith("index="):
							current_index = l[6:]
						elif l.startswith("r_frame_rate="):
							current_fps = l[13:]
						elif l.startswith("nb_frames="):
							current_nbframes = l[10:]
						elif l.startswith("sample_rate="):
							current_sample_rate = l[12:]
					elif l == "[STREAM]":
						current_fps_num = None
						current_fps_denom = None
						current_index = None
						current_type = None
						is_stream = True

				wav_path = None
				if a_index != None:
					sample_rate_int = int(a_sample_rate)
					target_sample_rate = "22254"
					if sample_rate_int == 11025 or sample_rate_int == 44100:
						target_sample_rate = "22050"
					elif sample_rate_int < 22000 or sample_rate_int > 23000:
						target_sample_rate = a_sample_rate
					
					wav_path = os.path.join(content_dir, "0.wav")
					if not invoke_command(os.path.join(tools_dir, "ffmpeg"), ["-y", "-i", mov_path, "-ac", "1", "-ar", target_sample_rate, "-c:a", "pcm_u8", wav_path]):
						return False

				if v_index != None:
					if not invoke_command(os.path.join(tools_dir, "ffmpeg"), ["-y", "-i", mov_path, os.path.join(content_dir, "%d.bmp")]):
						return False

				if a_index != None or v_index != None:
					with zipfile.ZipFile(mov_path + ".gpa", "w") as vid_archive:
						metaf = io.StringIO()

						if v_index != None:
							metaf.write("{\n")
							metaf.write("\t\"frameRateNumerator\" : " + v_fps_num + ",\n")
							metaf.write("\t\"frameRateDenominator\" : " + v_fps_denom + "\n")
							metaf.write("}\n")
						else:
							metaf.write("{\n")
							metaf.write("\t\"frameRateNumerator\" : " + a_nbframes + ",\n")
							metaf.write("\t\"frameRateDenominator\" : " + a_sample_rate + "\n")
							metaf.write("}\n")

						vid_archive.writestr("muvi/0.json", metaf.getvalue(), compress_type=zipfile.ZIP_DEFLATED, compresslevel=9)

						if v_index != None:
							frame_num = 1
							bmp_name = str(frame_num) + ".bmp"
							bmp_path = os.path.join(content_dir, bmp_name)
							while os.path.isfile(bmp_path):
								vid_archive.write(bmp_path, arcname=("PICT/" + bmp_name), compress_type=zipfile.ZIP_DEFLATED, compresslevel=9)
								os.remove(bmp_path)
								frame_num = frame_num + 1
								bmp_name = str(frame_num) + ".bmp"
								bmp_path = os.path.join(content_dir, bmp_name)

						if a_index != None:
							vid_archive.write(wav_path, arcname=("snd$20/0.wav"), compress_type=zipfile.ZIP_DEFLATED, compresslevel=9)
							os.remove(wav_path)

				if not debug_preserve_qt:
					os.remove(mov_path)

	return True

def reprocess_children(source_paths, dir_path):
	reprocess_extensions = [ "sea", "bin", "hqx", "zip", "cpt", "sit" ]
	contents = []
	recursive_scan_dir(contents, dir_path)
	for ext in reprocess_extensions:
		full_ext = "." + ext + ".gpf"
		for content_path in contents:
			if content_path.endswith(full_ext):
				truncated_path = content_path[:-4]
				data_path = truncated_path + ".gpd"
				if os.path.isfile(data_path):
					os.rename(data_path, truncated_path)
					source_paths.append(truncated_path)
					print("Requeueing subpath " + truncated_path)

	return True

def convert_resources(tools_dir, ts_path, qt_convert_dir, dir_path):
	contents = []
	recursive_scan_dir(contents, dir_path)
	for content_path in contents:
		if content_path.endswith(".gpr"):
			if not invoke_command(os.path.join(tools_dir, "gpr2gpa"), [content_path, ts_path, content_path[:-4] + ".gpa", "-dumpqt", qt_convert_dir]):
				return False

			qt_convert_contents = []
			recursive_scan_dir(qt_convert_contents, qt_convert_dir)
			
			converted_pict_ids = []

			# Convert inline QuickTime PICT resources
			for convert_content_path in qt_convert_contents:
				if convert_content_path.endswith(".mov"):
					if not invoke_command(os.path.join(tools_dir, "ffmpeg"), ["-y", "-i", convert_content_path, convert_content_path[:-4] + ".bmp"]):
						return False
					os.remove(convert_content_path)

				converted_pict_ids.append(os.path.basename(convert_content_path[:-4]))

			if len(converted_pict_ids) > 0:
				print("Reimporting converted QuickTime PICTs")
				qt_convert_json_path = os.path.join(dir_path, "qt_convert.json")

				convert_dict = { }
				convert_dict["delete"] = []
				convert_dict["add"] = { }

				for pict_id in converted_pict_ids:
					convert_dict["add"]["PICT/" + pict_id + ".bmp"] = os.path.join(qt_convert_dir, pict_id + ".bmp")

				with open(qt_convert_json_path, "w") as f:
					json.dump(convert_dict, f)

				if not invoke_command(os.path.join(tools_dir, "gpr2gpa"), [content_path, ts_path, content_path[:-4] + ".gpa", "-patch", qt_convert_json_path]):
					return False

				for pict_id in converted_pict_ids:
					os.remove(os.path.join(qt_convert_dir, pict_id + ".bmp"))
				os.remove(qt_convert_json_path)

			if not debug_preserve_resources:
				os.remove(content_path)

	return True

def scoop_files(tools_dir, output_dir, dir_path):
	mergegpf_path = os.path.join(tools_dir, "MergeGPF")
	contents = []
	recursive_scan_dir(contents, dir_path)
	scooped_files = []
	for content_path in contents:
		if content_path.endswith(".gpf"):
			is_house = False
			with zipfile.ZipFile(content_path, "r") as zfile:
				meta_contents = None
				with zfile.open("!!meta", "r") as metafile:
					meta_contents = metafile.read()
				if meta_contents[0] == 103 and meta_contents[1] == 108 and meta_contents[2] == 105 and meta_contents[3] == 72:
					is_house = True

			if is_house:
				if not invoke_command(mergegpf_path, [content_path]):
					return False
				scooped_files.append(content_path)

				mov_path = content_path[:-4] + ".mov.gpf"
				if os.path.isfile(mov_path):
					if not invoke_command(mergegpf_path, [mov_path]):
						return False
					scooped_files.append(mov_path)

	for scoop_path in scooped_files:
		os.replace(scoop_path, os.path.join(output_dir, os.path.basename(scoop_path)))

	return True

class ImportContext:
	def __init__(self):
		pass

	def run(self):
		os.makedirs(self.qt_convert_dir, exist_ok=True)
		os.makedirs(self.output_dir, exist_ok=True)

		invoke_command(self.make_timestamp_path, [self.ts_path])

		print("Looking for input files in " + self.source_dir)

		source_paths = []
		recursive_scan_dir(source_paths, self.source_dir)

		pending_result_directories = []
		result_dir_index = 0

		while len(source_paths) > 0:
			source_path = source_paths[0]
			source_paths = source_paths[1:]

			unpack_dir = os.path.join(self.output_dir, str(len(pending_result_directories)))
			try:
				os.mkdir(unpack_dir)
			except FileExistsError as error:
				pass

			print("Attempting to unpack " + source_path)
			decompressed_ok = False
			should_decompress = True
			if source_path.endswith(".zip"):
				decompressed_ok = decompress_zip(self.ftagdata_path, source_path, self.ts_path, unpack_dir)
			elif source_path.endswith(".sit") or source_path.endswith(".cpt") or source_path.endswith(".sea"):
				decompressed_ok = invoke_command(os.path.join(self.tools_dir, "unpacktool"), [source_path, self.ts_path, unpack_dir, "-paranoid"])
			elif source_path.endswith(".bin"):
				decompressed_ok = invoke_command(os.path.join(self.tools_dir, "bin2gp"), [source_path, self.ts_path, os.path.join(unpack_dir, os.path.basename(source_path[:-4]))])
			elif source_path.endswith(".hqx"):
				decompressed_ok = invoke_command(os.path.join(self.tools_dir, "hqx2gp"), [source_path, self.ts_path, os.path.join(unpack_dir, os.path.basename(source_path[:-4]))])
			else:
				should_decompress = False

			if should_decompress and not decompressed_ok:
				return

			if decompressed_ok:
				pending_result_directories.append(unpack_dir)

			while result_dir_index < len(pending_result_directories):
				if not self.process_dir(pending_result_directories, result_dir_index, source_paths):
					return

				result_dir_index = result_dir_index + 1

		# Clear temporaries
		if not debug_preserve_temps:
			for dir_path in pending_result_directories:
				shutil.rmtree(dir_path)


	def process_dir(self, all_dirs, dir_index, source_paths):
		root = all_dirs[dir_index]
		print("Processing directory " + root)

		if not recursive_fixup_macosx_dir(self.ftagdata_path, os.path.join(self.tools_dir, "ASADTool"), self.ts_path, root):
			return False

		if not convert_movies(self.tools_dir, root):
			return False
		if not convert_resources(self.tools_dir, self.ts_path, self.qt_convert_dir, root):
			return False
		if not reprocess_children(source_paths, root):
			return False
		if not scoop_files(self.tools_dir, self.output_dir, root):
			return False

		return True


def main():
	import_context = ImportContext()

	#script_dir = sys.argv[0]
	#source_dir = sys.argv[1]
	import_context.source_dir = "C:\\Users\\Eric\\Downloads\\gliderfiles\\archives"
	#output_dir = sys.argv[2]
	import_context.output_dir = "C:\\Users\\Eric\\Downloads\\gliderfiles\\converted"

	import_context.qt_convert_dir = os.path.join(import_context.output_dir, "qtconvert")
	import_context.tools_dir = "D:\\src\\GlidePort\\x64\\Release"
	import_context.make_timestamp_path = os.path.join(import_context.tools_dir, "MakeTimestamp")
	import_context.ts_path = os.path.join(import_context.output_dir, "Timestamp.ts")
	import_context.ftagdata_path = os.path.join(import_context.tools_dir, "FTagData")
	
	import_context.run()

main()
