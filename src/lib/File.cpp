/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 1999-2008 Soeren Sonnenburg
 * Copyright (C) 1999-2008 Fraunhofer Institute FIRST and Max-Planck-Society
 */

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

#include "lib/File.h"
#include "lib/SimpleFile.h"

CFile::CFile(FILE* f)
{
	file=f;
	filename=NULL;
	expected_type=F_UNKNOWN;
}

CFile::CFile(CHAR* fname, CHAR rw, EFeatureType typ, CHAR file_fourcc[4]) : CSGObject()
{
	status=false;
	task=rw;
	expected_type=typ;
	filename=strdup(fname);
	CHAR mode[2];
	mode[0]=rw;
	mode[1]='\0';


	if (rw=='r' || rw == 'w')
	{
		if (filename)
		{
			if ((file=fopen((const CHAR*) filename, (const CHAR*) mode)))
				status=true;
		}
	}

	if (file_fourcc)
	{
		if (rw=='r')
			status=read_header();
		else if (rw=='w')
			status=write_header();

		if (!status)
			fclose(file);

		file=NULL;
	}
}

CFile::~CFile()
{
	free(filename);
	if (file)
	  fclose(file);
	filename=NULL;
	file=NULL;
}

INT* CFile::load_int_data(INT* target, LONG& num)
{
	ASSERT(expected_type==F_INT);
	CSimpleFile<INT> f(filename, file);
	target=f.load(target, num);
	status=(target!=NULL);
	return target;
}

bool CFile::save_int_data(INT* src, LONG num)
{
	ASSERT(expected_type==F_INT);
	CSimpleFile<INT> f(filename, file);
	status=f.save(src, num);
	return status;
}

DREAL* CFile::load_real_data(DREAL* target, LONG& num)
{
	ASSERT(expected_type==F_DREAL);
	CSimpleFile<DREAL> f(filename, file);
	target=f.load(target, num);
	status=(target!=NULL);
	return target;
}

SHORTREAL* CFile::load_shortreal_data(SHORTREAL* target, LONG& num)
{
	ASSERT(expected_type==F_SHORTREAL);
	CSimpleFile<SHORTREAL> f(filename, file);
	target=f.load(target, num);
	status=(target!=NULL);
	return target;
}

bool CFile::save_real_data(DREAL* src, LONG num)
{
	ASSERT(expected_type==F_DREAL);
	CSimpleFile<DREAL> f(filename, file);
	status=f.save(src, num);
	return status;
}

bool CFile::save_shortreal_data(SHORTREAL* src, LONG num)
{
	ASSERT(expected_type==F_SHORTREAL);
	CSimpleFile<SHORTREAL> f(filename, file);
	status=f.save(src, num);
	return status;
}

CHAR* CFile::load_char_data(CHAR* target, LONG& num)
{
	ASSERT(expected_type==F_CHAR);
	CSimpleFile<CHAR> f(filename, file);
	target=f.load(target, num);
	status=(target!=NULL);
	return target;
}

bool CFile::save_char_data(CHAR* src, LONG num)
{
	ASSERT(expected_type==F_CHAR);
	CSimpleFile<CHAR> f(filename, file);
	status=f.save(src, num);
	return status;
}

BYTE* CFile::load_byte_data(BYTE* target, LONG& num)
{
	ASSERT(expected_type==F_BYTE);
	CSimpleFile<BYTE> f(filename, file);
	target=f.load(target, num);
	status=(target!=NULL);
	return target;
}

bool CFile::save_byte_data(BYTE* src, LONG num)
{
	ASSERT(expected_type==F_BYTE);
	CSimpleFile<BYTE> f(filename, file);
	status=f.save(src, num);
	return status;
}

WORD* CFile::load_word_data(WORD* target, LONG& num)
{
	ASSERT(expected_type==F_WORD);
	CSimpleFile<WORD> f(filename, file);
	target=f.load(target, num);
	status=(target!=NULL);
	return target;
}

bool CFile::save_word_data(WORD* src, LONG num)
{
	ASSERT(expected_type==F_WORD);
	CSimpleFile<WORD> f(filename, file);
	status=f.save(src, num);
	return status;
}

SHORT* CFile::load_short_data(SHORT* target, LONG& num)
{
	ASSERT(expected_type==F_SHORT);
	CSimpleFile<SHORT> f(filename, file);
	target=f.load(target, num);
	status=(target!=NULL);
	return target;
}

bool CFile::save_short_data(SHORT* src, LONG num)
{
	ASSERT(expected_type==F_SHORT);
	CSimpleFile<SHORT> f(filename, file);
	status=f.save(src, num);
	return status;
}

INT CFile::parse_first_header(EFeatureType &type)
{
	return -1;
}

INT CFile::parse_next_header(EFeatureType &type)
{
	return -1;
}


bool CFile::read_header()
{
    ASSERT(file!=NULL);
    UINT intlen=0;
    UINT endian=0;
    UINT file_fourcc=0;
    UINT doublelen=0;

	if ( (fread(&intlen, sizeof(BYTE), 1, file)==1) &&
			(fread(&doublelen, sizeof(BYTE), 1, file)==1) &&
			(fread(&endian, (UINT) intlen, 1, file)== 1) &&
			(fread(&file_fourcc, (UINT) intlen, 1, file)==1))
		return true;
	else
		return false;
}

bool CFile::write_header()
{
    BYTE intlen=sizeof(UINT);
    BYTE doublelen=sizeof(double);
    UINT endian=0x12345678;

	if ((fwrite(&intlen, sizeof(BYTE), 1, file)==1) &&
			(fwrite(&doublelen, sizeof(BYTE), 1, file)==1) &&
			(fwrite(&endian, sizeof(UINT), 1, file)==1) &&
			(fwrite(&fourcc, 4*sizeof(char), 1, file)==1))
		return true;
	else
		return false;
}

template <class T> void CFile::append_item(CDynamicArray<T>* items, CHAR* ptr_data, CHAR* ptr_item)
{
	size_t len=(ptr_data-ptr_item)/sizeof(CHAR);
	CHAR* item=new CHAR[len+1];
	memset(item, 0, sizeof(CHAR)*(len+1));
	item=strncpy(item, ptr_item, len);

	SG_DEBUG("current %c, len %d, item %s\n", *ptr_data, len, item);
	items->append_element(item);
}

bool CFile::read_real_valued_dense(DREAL*& matrix, INT& num_feat, INT& num_vec)
{
	ASSERT(expected_type==F_DREAL);

	struct stat stats;
	if (stat(filename, &stats)!=0)
		SG_ERROR("Could not get file statistics.\n");

	CHAR* data=new CHAR[stats.st_size+1];
	memset(data, 0, sizeof(CHAR)*(stats.st_size+1));
	size_t nread=fread(data, sizeof(CHAR), stats.st_size, file);
	if (nread<=0)
		SG_ERROR("Could not read data from %s.\n");

	SG_DEBUG("data read from file:\n%s\n", data);

	// determine num_feat and num_vec, populate dynamic array
	INT nf=0;
	num_feat=0;
	num_vec=0;
	CHAR* ptr_item=NULL;
	CHAR* ptr_data=data;
	CDynamicArray<CHAR*>* items=new CDynamicArray<CHAR*>();
	ASSERT(items);

	while (*ptr_data)
	{
		if (*ptr_data=='\n')
		{
			if (ptr_item)
				nf++;

			if (num_feat!=0 && nf!=num_feat)
				SG_ERROR("Number of features mismatches (%d != %d) in vector %d in file %s.\n", num_feat, nf, num_vec, filename);

			append_item(items, ptr_data, ptr_item);
			num_feat=nf;
			num_vec++;
			nf=0;
			ptr_item=NULL;
		}
		else if (!isblank(*ptr_data) && !ptr_item)
		{
			ptr_item=ptr_data;
		}
		else if (isblank(*ptr_data) && ptr_item)
		{
			append_item(items, ptr_data, ptr_item);
			ptr_item=NULL;
			nf++;
		}

		ptr_data++;
	}

	SG_DEBUG("num feat: %d, num_vec %d\n", num_feat, num_vec);
	delete[] data;

	// now copy data into matrix
	matrix=new DREAL[num_vec*num_feat];
	ASSERT(matrix);
	for (INT i=0; i<num_vec; i++)
	{
		for (INT j=0; j<num_feat; j++)
		{
			CHAR* item=items->get_element(i*num_feat+j);
			matrix[j*num_vec+i]=atof(item);
			delete[] item;
		}
	}
	delete items;

	//CMath::display_matrix(matrix, num_vec, num_feat);
	return true;
}

bool CFile::read_real_valued_sparse(TSparse<DREAL>*& matrix, INT& num_feat, INT& num_vec)
{
	return false;
}

bool CFile::read_char_valued_strings(T_STRING<CHAR>*& strings, INT& num_str, INT& max_string_len)
{
	return false;
}


