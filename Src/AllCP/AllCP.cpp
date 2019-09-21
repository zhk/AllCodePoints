#include "stdafx.h"

#include <map>
#include <string>
#include <iostream>
#include <fstream>

#include <ft2build.h>
#include FT_FREETYPE_H

using namespace std;

int ToUtf8(char* buffer, uint32_t cp)
{
    if (cp < 0x80)
    {
        if (isprint(cp))
        {
            *buffer++ = (char)cp;
            return 1;
        }
    }
    else if (cp < 0x0800)
    {
        *buffer++ = (char)((cp >> 6) | 0xC0);
        *buffer++ = (char)((cp & 0x3F) | 0x80);
        return 2;
    }
    else if (cp < 0x10000)
    {
        *buffer++ = (char)((cp >> 12) | 0xE0);
        *buffer++ = (char)(((cp >> 6) & 0x3F) | 0x80);
        *buffer++ = (char)((cp & 0x3F) | 0x80);
        return 3;
    }
    else if (cp < 0x110000)
    {
        *buffer++ = (char)((cp >> 18) | 0xF0);
        *buffer++ = (char)(((cp >> 12) & 0x3F) | 0x80);
        *buffer++ = (char)(((cp >> 6) & 0x3F) | 0x80);
        *buffer++ = (char)((cp & 0x3F) | 0x80);
        return 4;
    }

    return 0;
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        cout << "Usage: " << argv[0] << " <ttf|otf file path name> <output cp file pathname>" << endl;
        cout << "Example: " << argv[0] << " simsun.ttf all_cp.txt" << endl;
        return -1;
    }

    FT_Library library;
    if (FT_Init_FreeType(&library))
    {
        cerr << "FT_Init_FreeType failed" << endl;
        return -1;
    }

    FT_Face face;
    if (FT_New_Face(library, argv[1], -1, &face))
    {
        cerr << "The font file could be opened and read, or it is broken! " << argv[1] << endl;

        FT_Done_FreeType(library);
        return -1;
    }

    int face_cout = face->num_faces;
    FT_Done_Face(face);
    cout << "face count: " << face_cout << endl;

    string strUtf8;
    map<int, int> theMaps;

    for (int i = 0; i < face_cout; ++i)
    {
        if (FT_New_Face(library, argv[1], i, &face))
        {
            cerr << "The font file could be opened and read, or it is broken! " << argv[1] << endl;
            continue;
        }

        cout << "Family: " << face->family_name << ", Style: " << face->style_name << ", Charmaps Num: " << face->num_charmaps << endl;
        
        for (int n = 0; n < face->num_charmaps; n++)
        {
            FT_CharMap charmap = face->charmaps[n];
            FT_Set_Charmap(face, charmap);

            size_t count = 0;
            FT_UInt agindex;
            FT_ULong cp = FT_Get_First_Char(face, &agindex);
            while (agindex != 0)
            {
                count++;
                
                theMaps[cp] += 1;

                char buffer[8] = { 0 };
                int len = ToUtf8(buffer, cp);
                if (len != 0)
                    strUtf8.append(buffer, len);

                cp = FT_Get_Next_Char(face, cp, &agindex);
            }

            cout << "encoding: " << charmap->encoding
                << ", platform_id: " << charmap->platform_id
                << ", encoding_id: " << charmap->encoding_id
                << ", code points count: " << count
                << endl;
        }
    }

    cout << "CP count: " << theMaps.size() << endl;

    ofstream ofs8(argv[2]);
    if (!ofs8.is_open())
    {
        cerr << "Cannot write output file! " << argv[2] << endl;
    }
    else
    {
        ofs8 << strUtf8;
        ofs8.close();
    }

    FT_Done_FreeType(library);

    cout << "Done!" << endl;

	return 0;
}

 