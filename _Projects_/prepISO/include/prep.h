//--- build .ntfs[ file
static void build_file(char *filename, int parts, uint32_t num_tracks, uint64_t device_id, uint8_t profile, uint8_t m)
{
	if (parts > 0)
	{
		uint32_t emu_mode = EMU_PS3;
//		if(m == PS3ISO) emu_mode = EMU_PS3; else
		if(m == BDISO ) emu_mode = EMU_BD;  else
		if(m == DVDISO) emu_mode = EMU_DVD; else
		if(m == PSXISO) emu_mode = EMU_PSX;

		//--- build .ntfs[ file
		p_args = (rawseciso_args *)plugin_args; _memset(p_args, PLUGIN_ARGS_SIZE);
		p_args->device = device_id;
		p_args->emu_mode = emu_mode;
		p_args->num_sections = parts;

		uint32_t array_len = parts * sizeof(uint32_t);

		#ifdef MULTIMAN
		_memcpy(plugin_args + sizeof(rawseciso_args), sections, parts*sizeof(u32));
		_memcpy(plugin_args + sizeof(rawseciso_args) + (parts*sizeof(u32)), sections_size, parts*sizeof(u32));
		#else
		_memcpy(plugin_args + sizeof(rawseciso_args), sections, array_len);
		_memcpy(plugin_args + sizeof(rawseciso_args) + array_len, sections_size, array_len);
		#endif

		if(emu_mode == EMU_PSX)
		{
			if(num_tracks < 1) num_tracks = 1;

			int max = MAX_SECTIONS - ((num_tracks * sizeof(ScsiTrackDescriptor)) / 8);

			if (parts >= max) return;

			p_args->num_tracks = num_tracks | cd_sector_size_param;

			scsi_tracks = (ScsiTrackDescriptor *)(plugin_args + sizeof(rawseciso_args) + (2 * array_len));

			if(num_tracks <= 1)
			{
				scsi_tracks[0].adr_control = 0x14;
				scsi_tracks[0].track_number = 1;
				scsi_tracks[0].track_start_addr = 0;
			}
			else
			{
				for (u8 t = 0; t < num_tracks; t++)
				{
					scsi_tracks[t].adr_control = (tracks[t].is_audio) ? 0x10 : 0x14;
					scsi_tracks[t].track_number = t + 1;
					scsi_tracks[t].track_start_addr = tracks[t].lba;
				}
			}
		}
		else
			num_tracks = 0;

		//--- write .ntfs[ file
		FILE *flistW;
		snprintf(path, sizeof(path), CACHE_PATH "/%s%s.ntfs[%s]", filename, SUFIX2(profile), c_path[m]);
		flistW = fopen(path, "wb");
		if(flistW)
		{
			#ifdef MULTIMAN
			fwrite(plugin_args, (sizeof(rawseciso_args) + (2 * parts * sizeof(u32))) + (num_tracks * sizeof(ScsiTrackDescriptor)), 1, flistW);
			#else
			fwrite(plugin_args, (sizeof(rawseciso_args) + (2 * array_len) + (num_tracks * sizeof(ScsiTrackDescriptor))), 1, flistW);
			#endif
			fclose(flistW);
			sysFsChmod(path, 0666);
		}
	}
}