#ifdef NOBD_PATCH
	if(islike(param, "/nobd.ps3"))
	{
		// /nobd.ps3          toggle noBD
		// /nobd.ps3?         status noBD
		// /nobd.ps3?enable   patch noBD
		// /nobd.ps3?disable  patch normal

		u8 noBD = 0;

		// noBD LV1 4.75 - 4.92
		if(ALLOW_NOBD)
		{
			if(!param[9])
				noBD = !isNOBD;  // toggle noBD
			else if(!param[10])
				noBD =  isNOBD;  // status noBD
			else if( param[10] & 1) noBD = 1; //enable
			else if(~param[10] & 1) noBD = 0; //disable

			if(!param[9] || param[10])
			{
				apply_noBD_patches(noBD, false);
			}

			_concat2(&sbuffer, "noBD: ", noBD ? STR_ENABLED : STR_DISABLED);

			#if defined(DEBUG_MEM) || defined(PS3MAPI)
			sprintf(param, "/peek.lv1?712790");
			ps3mapi_find_peek_poke_hexview(pbuffer, templn, param);
			#endif
		}
		else
			_concat2(&sbuffer, "noBD: ", STR_ERROR);
	}
	else
#endif // #ifdef NOBD_PATCH
