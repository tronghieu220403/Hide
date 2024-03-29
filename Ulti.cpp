#include "Ulti.h"

bool ulti::GetNameWithoutDirectory(PWCHAR short_name, int size, PUNICODE_STRING long_name)
{
    int long_name_size = long_name->Length / 2;
    int ans = -1;
    for (int i = long_name_size - 1; i >= 0; i--)
    {
        if (long_name->Buffer[i] == WCHAR(L'\\'))
        {
            ans = i;
            break;
        }
    }

    int mem_size = (long_name_size - (ans + 1)) * 2;
    if (mem_size >= size)
    {
        return false;
    }
    RtlCopyMemory(short_name, &long_name->Buffer[ans + 1], mem_size);
    short_name[mem_size/2] = 0;
    return true;
}

int ulti::MergeString(PWCHAR des, int size , PWCHAR src1, PWCHAR src2)
{
	int size1 = GetWstringSize(src1);
	int size2 = GetWstringSize(src2);
	if (size1 + size2 > size)
	{
		return 0;
	}
	RtlCopyMemory(&des[0], src1, size1 * sizeof(WCHAR));
	RtlCopyMemory(&des[size1], src2, size2 * sizeof(WCHAR));
	return size1 + size2;
}

int ulti::GetWstringSize(PWCHAR str)
{
	for (int i = 0; ; i++)
	{
		if (str[i] == 0)
		{
			return i;
		}
	}
}

bool ulti::CheckSubstring(
    PWCHAR string,
    PWCHAR sub_string)
{
    UNREFERENCED_PARAMETER(sub_string);
    int string_len;
    int sub_string_len;

    for (string_len = 0; ; string_len++)
    {
        if (string_len > MAX_SIZE || string[string_len] == 0)
        {
            break;
        }
    }

    for (sub_string_len = 0; ; sub_string_len++)
    {
        if (sub_string_len > MAX_SIZE || sub_string[sub_string_len] == 0)
        {
            break;
        }
    }

    if (string_len == 0 || string_len > MAX_SIZE || sub_string_len == 0 || sub_string_len > MAX_SIZE)
    {
        return false;
    }

    if (string_len < sub_string_len)
    {
        return false;
    }

    for (int i = 0; i <= string_len - sub_string_len; i++)
    {
        int j;
        for (j = 0; j < sub_string_len; j++)
        {
            if (string[i + j] != sub_string[j])
                break;
        }

        if (j == sub_string_len)
        {
            return true;
        }
    }
    return false;

}

bool ulti::CheckEqualString(PWCHAR first_string, PWCHAR second_string)
{
    for (int i = 0; ; i++)
    {
        if (first_string[i] != second_string[i])
        {
            return false;
        }
        if (first_string[i] == 0 || second_string[i] == 0)
        {
            break;
        }
    }
    return true;
}

void ulti::SetUlongAt(long long addr, ULONG value)
{
    *(ULONG *)addr = value;
}

ULONG ulti::GetUlongAt(long long addr)
{
    return *(ULONG *)addr;
}
