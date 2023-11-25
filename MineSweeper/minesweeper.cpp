// minesweeper.cpp
//目标窗口16：9
//原始分辨率1600x900
//音频管理:修复音频缓存严重内存泄漏问题，代价为载入到内存的音频不再释放
//主线程存在内存泄漏问题
//解决dpi缩放问题


#include "framework.h"
#include "resource.h"
#include <string>
#include <cmath>
#include <cstring>
#include <chrono>
#include <windows.h>
#include<conio.h>
#include <io.h>
#include <shlobj_core.h>
#include <process.h>
#include <d3d11.h>
#include <d2d1_1.h>
#include <dxgi1_6.h>
#include <dwrite.h>
#include <wincodec.h>
#include<dinput.h>
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
//#pragma comment(lib, "d3xd11.lib")
#pragma comment(lib, "D3DCompiler.lib")
//#pragma comment(lib, "Effects11.lib")
//#pragma comment(lib, "dxerr.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#include <al.h>
#include <alut.h>
#include <alc.h>
#include <vorbis\vorbisfile.h>
#include <cstdio>
#include <iostream>
#include <vector>
#include <fstream>
//#include <time.h>
#define BUFFER_SIZE     16384       // 16 KB buffers

#pragma comment(lib,"libvorbisfile_static.lib")
#pragma comment(lib,"libogg.lib")
//#pragma comment(lib,"vorbisenc_static_d.lib") 
#pragma comment(lib,"libvorbis_static.lib")
//#pragma comment(lib,"EFX-Util_MT/EFX-Util.lib")
#pragma comment(lib,"OpenAL32.lib")

#include <openssl/md5.h>
#pragma comment(lib, "openssl/libcrypto_static.lib")
#pragma comment(linker, "/STACK:32768")
#define SAFE_RELEASE(P) if(P){P->Release() ; P = NULL ;}    //安全释放对象
#define MAX_LOADSTRING 100
using namespace std;
SYSTEMTIME st;
bool quit_single = 0, normal_quit=0;
IDXGIOutput* g_output = NULL;
bool rightclick = 0;
int resolution_allow = 2, game_mode = 0;
struct setting_general_desc
{
    bool language_translation = 1;
    int map_size = 1;
    bool step_time_level = 0;
    bool tips = 0;
    //bool mouse_input = 0;
};
setting_general_desc set1[2];
struct setting_graphics_desc
{
    bool visual_effect = 1;
    bool vsync = 1;
    bool MSAA = 1;
    int sampleCount = 0;
    int resolution = 1;
    bool show_framerate = 0;
};
setting_graphics_desc set2[2];
struct setting_audio_desc
{
    int music_volume = 30;
    int se_volume = 50;
    bool thread_method = 0;
};
setting_audio_desc set3[2];
struct stringTable
{
    string test = "test";
    string setting = "Setting";
    string quit = "Quit";
    string str_return = "Return";
    string general = "General";
    string graphics = "Graphics";
    string audio = "Audio";
    string language_translation = "Language Translation";
    string map_size = "Map Size";
    string step_time_level = "Animation Speed";
    string tips = "Tips";
    //string mouse_input = "";
    string visual_effect = "Visual Effect";
    string vsync = "V-Sync";
    string MSAA = "Anti-aliasing";
    string sampleCount = "Current SampleCount: ";
    string resolution = "Resolution";
    string show_framerate = "Show Framerate";
    string music_volume = "Music Volume";
    string se_volume = "SE Volume";
    string thread_method = "Thread Method";
    string on = "On";
    string off = "Off";
    string mode1 = "Mode1";
    string mode2 = "Mode2";
    string lastpage = "Last Page";
    string nextpage = "Next Page";
    string highscore = "Highscore";
    //string scoresum = "Sum of Score:";
    //string stepsum = "Sum of Step:";
    string timesum = "Play time:";
    string accountTime = "Account time at";
    string save_succeed = "save succeed.";
    string save_failed = "save failed.";
    string load_failed = "load failed.";
    string enter_username = "Enter Username:";
    string notice_maxnum_not_enough = "Reaching 512 is required.";
    string easy = "Easy";
    string normal = "Normal";
    string hard = "Hard";
    

    string language_translation_description = "Use external translation data.";
    string mapsize_description = "Change the difficulty of the game.";
    string step_time_level_description = "Change the speed of animation in the game.\nMode1=Fast   Mode2=Slow";
    string tips_description = "Show tips when starting.";
    //string mouse_input_description = "";
    string visual_effect_description = "Present more visual effect.";
    string vsync_description = "Enable vertical synchronization to decrease screen tearing and save power.";
    string MSAA_description = "Enable highest level of MultiSampling Anti-Aliasing to make the image smoother.\nRestart application to apply.";
    //string sampleCount_description = "";
    string resolution_description = "Change the resolution of the program.\nRestart application to apply.";
    string show_framerate_description = "Show realtime fps and drawcall times per frame.";
    //string music_volume_description = "";
    //string se_volume_description = "";
    string thread_method_description = "Change sleeping methed of the audio thread. Mode1=Balance Mode2=Performance\nKeep this option at mode1 unless the audio latency is unbearble.";

    string save_overwrite_warning = "Overwrite current save, Sure?";
    string load_warning = "Load save and LOSE current progress, Sure?";
};
stringTable lan[3];

//线程间通信
bool thread_IO_request_save_config = 0, thread_IO_config_read = 0, thread_IO_request_read_all_info = 0, thread_IO_request_update_profile=0, thread_IO_request_read_config=0;
int thread_IO_request_userinit = 0;
bool savebuf[1024] = { 0 };
int  opened_cubenum_buf=0, mine_remain = 0, flagbuf=0;

int   thread_IO_request_save_gameMS = 0, thread_IO_request_load_gameMS = 0, thread_IO_request_delete_game = 0, thread_IO_request_record_game = 0, thread_IO_request_read_history=0, thread_IO_request_read_highscore=0;
float time_used = 0, timebuf = 0;
string usernameC = "";
int currentSave = 0;
int thread_Audio_target_music = 2;
bool thread_Audio_switch_immediately = 0, thread_Audio_quit_single=0, thread_Audio_volume_changed=0, thread_Audio_volume_changed_music=0;
//int max2048_numc = 2, max2048_numh = 0;
int md5_result = 0;
int page_index = 0, page_status = 0;
struct read_info
{
    int active = 0;
    string opened_cube = "";
    string mapsize = "";
    string flag = "";
    string time = "";
    string totle_num = "";
    string date = "";
    string mine_sum = "";
    string win_num = "";
    string gamedata = "";
    D2D1_RECT_F rect = D2D1::RectF();
};
read_info read_infos[10];
read_info history_buf[5];
read_info highscore[4];

//读写线程部分
const char* filename_dbg = "debug.log";
string filebuf1[64] = { "" };
//struct stringgroup
//{
//    string filename;
//    string content;
//    LPCWSTR Lfilename = NULL;
//};
//stringgroup apc[8];

void writelog(string content)
{
    //string buf[1] = { content };
    for (int i = 0; i < 64; i++)
    {
        if (filebuf1[i] == "")
        {
            filebuf1[i] = content;
            break;
        }
    }

    return;
}

struct resource
{
    LPCWSTR Lfilename = L"";
    string filename = "";
    string md5 = "";
}res[128];

bool md5_verify(int filenum)
{


    MD5_CTX ctx;
    int len = 0;
    unsigned char buffer[BUFFER_SIZE] = { 0 };
    unsigned char digest[16] = { 0 };
    const char* k = res[filenum].filename.c_str();
    FILE* pFile;
    errno_t err;

    if (err = fopen_s(&pFile, k, "rb") != 0)
    {

        return 0;
    }

    MD5_Init(&ctx);
    if (pFile == 0)
    {


        return 0;
    }
    while ((len = fread(buffer, 1, 1020, pFile)) > 0)
    {
        MD5_Update(&ctx, buffer, len);
    }

    MD5_Final(digest, &ctx);

    fclose(pFile);


    int i = 0;
    char buf[33] = { 0 };
    char tmp[3] = { 0 };
    for (i = 0; i < 16; i++)
    {
        sprintf_s(tmp, "%02X", digest[i]);
        strcat_s(buf, tmp);
    }

    char cp[33];
    for (int i = 0; i < 32; i++)
    {
        cp[i] = res[filenum].md5[i];
    }
    cp[32] = '\0';


    if (strcmp(cp, buf) == 0)
    {

        return 1;
    }
    else
    {

        return 0;
    }

}
int md5_private_verify(const char* k, string *md5)
{

    
    MD5_CTX ctx;
    int len = 0, check = 0;
    unsigned char buffer[1024] = { 0 };
    unsigned char digest[16] = { 0 };

    FILE* pFile=0;
    errno_t err;

    err = fopen_s(&pFile, k, "rb");
    

    
    if (pFile == 0||err!=0)
    {
        writelog("Open file failed!");
        writelog(k);
        return -1;

        
    }
    MD5_Init(&ctx);
    while ((len = fread(buffer, 1, 1020, pFile)) > 0)
    {
        MD5_Update(&ctx, buffer, len);
    }

    MD5_Final(digest, &ctx);

    fclose(pFile);


    int i = 0;
    char buf[33] = { 0 };
    char tmp[3] = { 0 };
    for (i = 0; i < 16; i++)
    {
        sprintf_s(tmp, "%02X", digest[i]);
        strcat_s(buf, tmp);
    }



    *md5 = buf;

    check = (int)buf[1] * (int)buf[10] + (int)buf[2] * (int)buf[9]+ (int)buf[4] * (int)buf[8];
    check *= (int)buf[14]+ (int)buf[6];
    check /= (int)buf[3]/3+1;
    check += (int)buf[5] * (int)buf[7];


    return check;


}
void playeffectsound(int num);
int ReadFile(const char* filename, string filebuf[64])
{
    
    ifstream file;
    int num = 0;
    file.open(filename, ios::in);
    if (file.is_open())
    {
        while (getline(file, filebuf[num]))
        {
            if (filebuf[num] == "")
            {
                continue;
            }
            else
            {
                num++;
                if (num == 64)
                {
                    writelog("IO Warning: lines beyond 64 of filebuf[64]!");
                    writelog(filename);
                    break;
                }
            }
        }
        file.close();
        return num;
    }
    else
    {
        playeffectsound(15);
        writelog("ReadFile failed!");
        writelog(filename);
        return -1;
    }
}
bool WriteFile(const char* filename, string filebuf[64])
{
    ofstream file;
    file.open(filename, ios::out);
    int end = 0;
    if (file.is_open())
    {
        for (int i = 0; i < 64; i++)
        {
            if (filebuf[i] == "")
            {
                end = 1;
                continue;
            }
            if (!end)
            {
                file << filebuf[i] << endl;
            }
            else
            {
                writelog("Possible file corruption: Discontinuous writebuf!");
            }
            
        }
        file.close();
        return 1;
    }
    else
    {
        playeffectsound(15);
        string err = "WriteFile Error!\n";
        err += filename;
        writelog(err);
        //MessageBox(NULL, err.c_str(), "IO Error", MB_OK | MB_ICONSTOP);
        return 0;
    }
}
//std::string TCHAR2STRING(TCHAR* STR)
//{
//    int iLen = WideCharToMultiByte(CP_ACP, 0, STR, -1, NULL, 0, NULL, NULL);
//    char* chRtn = new char[iLen * sizeof(char)];
//    WideCharToMultiByte(CP_ACP, 0, STR, -1, chRtn, iLen, NULL, NULL);
//    std::string str(chRtn);
//    delete chRtn;
//    return str;
//}
string tcharToChar(TCHAR* buffer)
{
    char* charBuffer = NULL;
    std::string returnValue;
    int lengthOfbuffer = lstrlenA(buffer);
    if (buffer != NULL)
    {
        charBuffer = (char*)calloc(lengthOfbuffer + 1, sizeof(char));
    }
    else
    {
        return "";
    }

    for (int index = 0;
        index < lengthOfbuffer;
        index++)
    {
        char* singleCharacter = (char*)calloc(2, sizeof(char));
        //char* singleCharacter = '\0';
        singleCharacter[0] = (char)buffer[index];
        singleCharacter[1] = '\0';
        strcat_s(charBuffer, strlen(charBuffer)+ strlen(singleCharacter)+1,singleCharacter);
        free(singleCharacter);
    }
    strcat_s(charBuffer, strlen(charBuffer)+2,"\0");
    returnValue.append(charBuffer);
    free(charBuffer);
    return returnValue;

}
bool self_restarted = 0;
int ed_stage = 0;
void self_restart()
{

    if (self_restarted)
    {
        return;
    }
    writelog("try to restart.");
    string writebuff[64] = {""};
    //writebuff[0] = "chcp 65001";
    writebuff[0] = "ping 127.1 /n 5";
    int ci = 1;
    
    writebuff[ci] = "start \"\" ";
    writebuff[ci+1] = "del /f /q %0&exit";
    
    //writebuff[3] = "del /f /q %0&exit";
    //writebuff[4] = "exit";
    //writebuff[2] = "start \"\" ";
    const int nBufSize = 512;
    string quote = "\"";
    TCHAR chBuf[nBufSize];
    ZeroMemory(chBuf, nBufSize);
    if (GetModuleFileName(NULL, chBuf, nBufSize))
    {
        writebuff[ci] += quote;
        writebuff[ci] += tcharToChar(chBuf);
        writebuff[ci] += quote;
        WriteFile("./restart.bat", writebuff);
        writelog("Now execute:");
        writelog(chBuf);
        system("start /min .\\restart.bat");
        normal_quit = 1;
        self_restarted = 1;
    }
    else
    {
        writelog("GetModuleFileName Failed!");
    }
    return;
}
bool file_private_verify(string filename)
{
    //return 1;
    string checkfilename = filename.substr(0, filename.length() - 3), filemd5 = "", readbuf[64] = { "" };
    checkfilename += "check";
    int privatesum=0,lines=0;
    privatesum = md5_private_verify(filename.c_str(), &filemd5);
    lines=ReadFile(checkfilename.c_str(), readbuf);
    if (lines > 0)
    {
        if (filemd5 == readbuf[0]&&to_string(privatesum)== readbuf[1])
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        writelog("file_private_verify() failed!");
    }
    return 0;
}
void certfile(string filename)
{
    
    int check = 0;
    string writebuff[64] = { "" }, md5buf="",checkfilename= filename.substr(0, filename.length() - 3);
    checkfilename += "check";
    check = md5_private_verify(filename.c_str(), &md5buf);
    //string dbg = "md5 ";
    //dbg += md5buf;
    if (check >0)
    {
        //writelog(md5buf);
        writebuff[0] = md5buf;
        writebuff[1] = to_string(check);
        WriteFile(checkfilename.c_str(), writebuff);
    }
   

    //cout << md5buf << " a ";
    //char c = _getch();
    return;
}
void Wchar_tToString(std::string& szDst, wchar_t* wchar)
{
    wchar_t* wText = wchar;
    DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, NULL, 0, NULL, FALSE);//WideCharToMultiByte的运用
    char* psText;  // psText为char*的临时数组，作为赋值给std::string的中间变量
    psText = new char[dwNum];
    WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, psText, dwNum, NULL, FALSE);//WideCharToMultiByte的再次运用
    szDst = psText;// std::string赋值
    delete[]psText;// psText的清除
}

void cleanStrBuff(string buf[64])
{
    for (int i = 0; i < 64; i++)
    {
        buf[i] = "";
    }
    return;
}
string addFormat(string ori)
{
    string mark = "[";
    mark += ori; 
    mark += "]";
    return mark;
}
string getTimeStr()
{
    GetLocalTime(&st);
    string buf = "";
    buf = to_string(st.wYear);
    buf += "/";
    buf += to_string(st.wMonth);
    buf += "/";
    buf += to_string(st.wDay);
    buf += " ";
    buf += to_string(st.wHour);
    buf += ":";
    buf += to_string(st.wMinute);
    return buf;
}
void fileinit();
void userinit()
{
    
    string pathname = "./save/", fullfilename = "", writebuf[64] = { "" };
    int flag = 0;
    flag = CreateDirectory(pathname.c_str(), NULL);
    if (flag == 0)
    {
        writelog("CreateDirectory Error or folder has existed!(./save)");
        writelog(to_string(flag));
    }
    fullfilename = pathname;
    fullfilename += "/AllUsersProfile.dat";
    pathname += usernameC;
    
    flag = CreateDirectory(pathname.c_str(), NULL);
    if (flag == 0)
    {
        writelog("CreateDirectory Error or folder has existed!(./save/username)");
        writelog(to_string(flag));
    }
    writebuf[0] = "[AllUsers]";
    writebuf[1] = "[Username]";
    writebuf[2] = usernameC;
    writebuf[3] = "[long_credit]";
    writebuf[4] = "1";
    WriteFile(fullfilename.c_str(), writebuf);
    
    certfile(fullfilename);
    /*cleanStrBuff(writebuf);
    writebuf[0] = "chcp 65001";
    writebuf[1] = "systeminfo>>.\\debug.log";
    writebuf[2] = "del /f /q %0&exit";
    WriteFile("./restart.bat", writebuf);
    system("start /min .\\restart.bat");*/
    
    fileinit();
    if (_access(fullfilename.c_str(), 4) != 0)
    {
        thread_IO_request_userinit = -1;
    }
    else
    {
        thread_IO_request_userinit = 0;
    }
    return;
}
bool long_credit = 1;
void fileinit()
{
    string writebuf[64] = { "" },readbuf[64] = { "" };
    int flag = 0,lines=0;
    string pathname = "./save/", fullfilename = "";
    fullfilename = pathname;
    fullfilename += "/AllUsersProfile.dat";
    if (_access(fullfilename.c_str(), 4) != 0)
    {
        writelog("./save/AllUsersProfile.dat not exist!");
        /*flag = CreateDirectory(pathname.c_str(), NULL);
        if (flag == 0)
        {
            writelog("CreateDirectory Error or folder has existed!(./save)");
            writelog(to_string(flag));

        }*/
        usernameC = "";
        
        return;
    }
    if (file_private_verify(fullfilename))
    {
        lines = ReadFile(fullfilename.c_str(), readbuf);
        if (lines < 1)
        {
            writelog("file corrupted!");
            writelog(fullfilename.c_str());
            usernameC = "";
            return;
        }
        else
        {
            for (int i = 0; i < 63; i++)
            {
                if (readbuf[i] == "[Username]")
                {
                    usernameC = readbuf[i + 1];
                }
                else if (readbuf[i] == "[long_credit]")
                {
                    long_credit = (bool)stoi(readbuf[i + 1]);
                }
            }
        }
    }
    else
    {
        writelog("file cert failed!");
        writelog(fullfilename.c_str());
        usernameC = "";
        return;
    }
    pathname += usernameC;
    //int flag = 0, result = 0;
    
    fullfilename = pathname;
    fullfilename += "/highscore.dat";
    if (_access(fullfilename.c_str(), 4) != 0)
    {
        writelog("./save/username/highscore.dat not exist!");
        flag = CreateDirectory(pathname.c_str(), NULL);
        if (flag == 0)
        {
            writelog("CreateDirectory Error or folder has existed!(./save/username)");
            writelog(to_string(flag));
        }
        cleanStrBuff(writebuf);
        writebuf[0] = "[user]";
        writebuf[1] = usernameC;
        writebuf[2] = "[mine_sum]";
        writebuf[3] = to_string(0);
        writebuf[4] = "[time_sum]";
        writebuf[5] = to_string(0.0f);
        writebuf[6] = "[win_num]";
        writebuf[7] = to_string(0);
        writebuf[8] = to_string(0);
        writebuf[9] = to_string(0);
        writebuf[10] = to_string(0);
        writebuf[11] = to_string(0);
        writebuf[12] = to_string(0);
        writebuf[13] = "[highscore]";
        writebuf[14] = to_string(0.0f);
        writebuf[15] = to_string(0.0f);
        writebuf[16] = to_string(0.0f);
        writebuf[17] = getTimeStr();
        writebuf[18] = getTimeStr();
        writebuf[19] = getTimeStr();
        writebuf[20] = "[date]";
        writebuf[21] = getTimeStr();
        WriteFile(fullfilename.c_str(), writebuf);
        
        certfile(fullfilename);
    }
    cleanStrBuff(writebuf);
    //int lines = ReadFile("./save/AllUsersProfile.dat", readbuf);
    //string t_filename = "./save/AllUsersProfile.dat";
    //if (_access(t_filename.c_str(), 4) != 0)
    //{
    //    writelog("./save/AllUsersProfile.dat not exist!");
    //    flag = CreateDirectory(".\\save", NULL);
    //    if ( flag== 0)
    //    {
    //        writelog("CreateDirectory Error or folder has existed!(./save)");
    //        writelog(to_string(flag));
    //    }
    //    //system("md .\\save\\");
    //    writebuf[0] = "[AllUsers]";
    //    writebuf[1] = "[Username]";
    //    writebuf[2] = "[Avatar]";
    //    writebuf[3] = "[LastUser]";
    //    WriteFile("./save/AllUsersProfile.dat", writebuf);
    //    certfile(t_filename);


    //}
    //cleanStrBuff(writebuf);
    //lines = ReadFile("./save/config.dat", readbuf);
    
    if (_access("./save/config.dat", 4) != 0)
    {
        //int music_volume = 50;
        //int se_volume = 50;
        //bool thread_method = 0;
        writelog("./save/config.dat not exist!");
        writebuf[0] = addFormat(lan[1].language_translation);
        writebuf[1] = to_string(set1[1].language_translation);
        writebuf[2] = addFormat(lan[1].map_size);
        writebuf[3] = to_string(set1[1].map_size);
        writebuf[4] = addFormat(lan[1].step_time_level);
        writebuf[5] = to_string(set1[1].step_time_level);
        writebuf[6] = addFormat(lan[1].tips);
        writebuf[7] = to_string(set1[1].tips);
        //writebuf[8] = addFormat(lan[1].mouse_input);
        //writebuf[9] = to_string(set1[1].mouse_input);
        writebuf[8] = addFormat(lan[1].visual_effect);
        writebuf[9] = to_string(set2[1].visual_effect);
        writebuf[10] = addFormat(lan[1].vsync);
        writebuf[11] = to_string(set2[1].vsync);
        writebuf[12] = addFormat(lan[1].MSAA);
        writebuf[13] = to_string(set2[1].MSAA);
        writebuf[14] = addFormat(lan[1].resolution);
        writebuf[15] = to_string(set2[1].resolution);
        writebuf[16] = addFormat(lan[1].show_framerate);
        writebuf[17] = to_string(set2[1].show_framerate);
        writebuf[18] = addFormat(lan[1].music_volume);
        writebuf[19] = to_string(set3[1].music_volume);
        writebuf[20] = addFormat(lan[1].se_volume);
        writebuf[21] = to_string(set3[1].se_volume);
        writebuf[22] = addFormat(lan[1].thread_method);
        writebuf[23] = to_string(set3[1].thread_method);
        //writebuf[3] = "[Easy]";
        WriteFile("./save/config.dat", writebuf);
        
    }
    
    wchar_t* pathbuf1 = NULL;
    string pathbuf = "";
    SHGetKnownFolderPath(FOLDERID_ProgramData, KF_FLAG_DEFAULT_PATH, NULL, &pathbuf1);
    Wchar_tToString(pathbuf, pathbuf1);
    cleanStrBuff(writebuf);
    writebuf[0] = "1101";
    pathbuf += "\\MS\\";
    string filepath = pathbuf;
    filepath += "init_ver.dat";
    //writelog(filepath);
    //lines = ReadFile(filepath.c_str(), readbuf);
    if (_access(filepath.c_str(), 4) != 0)
    {
        /*string pathbuf1 = "md ";
        pathbuf1 += pathbuf;*/
        flag = CreateDirectory(pathbuf.c_str(), NULL);
        if ( flag== 0)
        {
            writelog("IO Error!(FOLDERID_ProgramData)");
        }
        //system(pathbuf1.c_str());
        
        WriteFile(filepath.c_str(), writebuf);
        
    }
    return;
}


void saveConfig()
{
    string  writebuf[64] = { "" };
    //fileinit();
    Sleep(1);
    writebuf[0] = addFormat(lan[1].language_translation);
    writebuf[1] = to_string(set1[0].language_translation);
    writebuf[2] = addFormat(lan[1].map_size);
    writebuf[3] = to_string(set1[0].map_size);
    writebuf[4] = addFormat(lan[1].step_time_level);
    writebuf[5] = to_string(set1[0].step_time_level);
    writebuf[6] = addFormat(lan[1].tips);
    writebuf[7] = to_string(set1[0].tips);
    //writebuf[8] = addFormat(lan[1].mouse_input);
    //writebuf[9] = to_string(set1[0].mouse_input);
    writebuf[8] = addFormat(lan[1].visual_effect);
    writebuf[9] = to_string(set2[0].visual_effect);
    writebuf[10] = addFormat(lan[1].vsync);
    writebuf[11] = to_string(set2[0].vsync);
    writebuf[12] = addFormat(lan[1].MSAA);
    writebuf[13] = to_string(set2[0].MSAA);
    writebuf[14] = addFormat(lan[1].resolution);
    writebuf[15] = to_string(set2[0].resolution);
    writebuf[16] = addFormat(lan[1].show_framerate);
    writebuf[17] = to_string(set2[0].show_framerate);
    writebuf[18] = addFormat(lan[1].music_volume);
    writebuf[19] = to_string(set3[0].music_volume);
    writebuf[20] = addFormat(lan[1].se_volume);
    writebuf[21] = to_string(set3[0].se_volume);
    writebuf[22] = addFormat(lan[1].thread_method);
    writebuf[23] = to_string(set3[0].thread_method);

    WriteFile("./save/config.dat", writebuf);
    return;
}
void readConfig()
{
    string readbuf[64] = { "" };
    //fileinit();
    if (ReadFile("./save/config.dat", readbuf)==-1)
    {
        writelog("read config failed!");
        return;
    }

    for (int i = 0; i < 64; i++)
    {
        if (readbuf[i] == "")
        {
            break;
        }
        if (readbuf[i] == addFormat(lan[1].language_translation))
        {
            
            set1[0].language_translation = atoi(readbuf[i + 1].c_str());
        }
        else if (readbuf[i] == addFormat(lan[1].map_size))
        {
            set1[0].map_size = atoi(readbuf[i + 1].c_str());
        }
        else if (readbuf[i] == addFormat(lan[1].step_time_level))
        {
            set1[0].step_time_level = atoi(readbuf[i + 1].c_str());
        }
        else if (readbuf[i] == addFormat(lan[1].tips))
        {
            set1[0].tips = atoi(readbuf[i + 1].c_str());
        }
        /*else if (readbuf[i] == addFormat(lan[1].mouse_input))
        {
            set1[0].mouse_input = atoi(readbuf[i + 1].c_str());
        }*/
        else if (readbuf[i] == addFormat(lan[1].visual_effect))
        {
            set2[0].visual_effect = atoi(readbuf[i + 1].c_str());
        }
        else if (readbuf[i] == addFormat(lan[1].vsync))
        {
            set2[0].vsync = atoi(readbuf[i + 1].c_str());
        }
        else if (readbuf[i] == addFormat(lan[1].MSAA))
        {
            set2[0].MSAA = atoi(readbuf[i + 1].c_str());
        }
        else if (readbuf[i] == addFormat(lan[1].resolution))
        {
            set2[0].resolution = atoi(readbuf[i + 1].c_str());
        }
        else if (readbuf[i] == addFormat(lan[1].show_framerate))
        {
            set2[0].show_framerate = atoi(readbuf[i + 1].c_str());
        }
        else if (readbuf[i] == addFormat(lan[1].music_volume))
        {
            set3[0].music_volume = atoi(readbuf[i + 1].c_str());
        }
        else if (readbuf[i] == addFormat(lan[1].se_volume))
        {
            set3[0].se_volume = atoi(readbuf[i + 1].c_str());
        }
        else if (readbuf[i] == addFormat(lan[1].thread_method))
        {
            set3[0].thread_method = atoi(readbuf[i + 1].c_str());
        }
        
    }
    thread_IO_config_read = 1;
    thread_Audio_volume_changed = 1;
    thread_Audio_volume_changed_music = 1;
    
    cleanStrBuff(readbuf);
    if (ReadFile("./lan/language.dat", readbuf) <1)
    {
        writelog("read language failed!");
        return;
    }
    for (int i = 0; i < 64; i++)
    {
        if (readbuf[i] == "")
        {
            break;
        }
        if (readbuf[i] == addFormat("general"))
        {

            lan[2].general = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("map_size"))
        {
            lan[2].map_size = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("step_time_level"))
        {
            lan[2].step_time_level = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("tips"))
        {
            lan[2].tips = readbuf[i + 1];
        }
        /*else if (readbuf[i] == addFormat("mouse_input"))
        {
            lan[2].mouse_input = readbuf[i + 1];
        }*/
        else if (readbuf[i] == addFormat("visual_effect"))
        {
            lan[2].visual_effect = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("vsync"))
        {
            lan[2].vsync = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("MSAA"))
        {
            lan[2].MSAA = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("resolution"))
        {
            lan[2].resolution = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("show_framerate"))
        {
            lan[2].show_framerate = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("music_volume"))
        {
            lan[2].music_volume = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("se_volume"))
        {
            lan[2].se_volume = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("thread_method"))
        {
            lan[2].thread_method = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("graphics"))
        {
            lan[2].graphics = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("audio"))
        {
            lan[2].audio = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("sampleCount"))
        {
            lan[2].sampleCount = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("on"))
        {
            lan[2].on = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("off"))
        {
            lan[2].off = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("mode1"))
        {
            lan[2].mode1 = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("mode2"))
        {
            lan[2].mode2 = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("lastpage"))
        {
            lan[2].lastpage = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("nextpage"))
        {
            lan[2].nextpage = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("highscore"))
        {
            lan[2].highscore = readbuf[i + 1];
        }
        /*else if (readbuf[i] == addFormat("scoresum"))
        {
            lan[2].scoresum = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("stepsum"))
        {
            lan[2].stepsum = readbuf[i + 1];
        }*/
        else if (readbuf[i] == addFormat("timesum"))
        {
        lan[2].timesum = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("accountTime"))
        {
        lan[2].accountTime = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("save_succeed"))
        {
        lan[2].save_succeed = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("save_failed"))
        {
        lan[2].save_failed = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("load_failed"))
        {
        lan[2].load_failed = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("notice_maxnum_not_enough"))
        {
        lan[2].notice_maxnum_not_enough = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("enter_username"))
        {
        lan[2].enter_username = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("easy"))
        {
        lan[2].easy = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("normal"))
        {
        lan[2].normal = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("hard"))
        {
        lan[2].hard = readbuf[i + 1];
        }

    }
    cleanStrBuff(readbuf);
    if (ReadFile("./lan/desc.dat", readbuf) < 1)
    {
        writelog("read desc failed!");
        return;
    }
    for (int i = 0; i < 64; i++)
    {
        if (readbuf[i] == "")
        {
            break;
        }
        if (readbuf[i] == addFormat("language_translation_description"))
        {

            lan[2].language_translation_description = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("mapsize_description"))
        {
            lan[2].mapsize_description = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("step_time_level_description"))
        {
            lan[2].step_time_level_description = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("tips_description"))
        {
            lan[2].tips_description = readbuf[i + 1];
        }
        /*else if (readbuf[i] == addFormat("mouse_input_description"))
        {
            lan[2].mouse_input_description = readbuf[i + 1];
        }*/
        else if (readbuf[i] == addFormat("visual_effect_description"))
        {
            lan[2].visual_effect_description = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("vsync_description"))
        {
            lan[2].vsync_description = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("MSAA_description"))
        {
            lan[2].MSAA_description = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("resolution_description"))
        {
            lan[2].resolution_description = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("show_framerate_description"))
        {
            lan[2].show_framerate_description = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("music_volume"))
        {
            lan[2].music_volume = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("thread_method_description"))
        {
            lan[2].thread_method_description = readbuf[i + 1];
            lan[2].thread_method_description += "\n";
            lan[2].thread_method_description+= readbuf[i + 2];
        }
        else if (readbuf[i] == addFormat("save_overwrite_warning"))
        {
            lan[2].save_overwrite_warning = readbuf[i + 1];
        }
        else if (readbuf[i] == addFormat("load_warning"))
        {
            lan[2].load_warning = readbuf[i + 1];
        }
        

    }
    if (set1[0].language_translation)
    {
        lan[0] = lan[2];
    }
    return;
}

//bool filecheck(const char* k1, const char* k2)
//{
//    fileinit();
//    int check = 0;
//    string readbuff[64] = { "" }, md5buf;
//    check = md5_private_verify(k1, &md5buf);
//    ReadFile(k2, readbuff);
//    
//    if (md5buf == readbuff[0] && to_string(check) == readbuff[1])
//    {
//        return 1;
//    }
//    else
//    {
//        return 0;
//    }
//}
//int response_count[3] = { 0 };
//bool bark = 0;
//bool watchdog(int thread_num)
//{
//    for (int i = 0; i < 3; i++)
//    {
//        response_count[i]++;
//    }
//    response_count[thread_num] = 0;
//    for (int i = 0; i < 3; i++)
//    {
//        if (response_count[i] > 500)
//        {
//            bark = 1;
//        }
//    }
//    return bark;
//}

bool AllisNum(string str)
{
    for (int i = 0; i < str.size(); i++)
    {
        int tmp = (int)str[i];
        if (tmp >= 48 && tmp <= 57)
        {
            continue;
        }
        else
        {
            return false;
        }
    }
    return true;
}


void game_read_all_info()
{
    string readbuf[64] = { "" };
    string pathname = "./save/",fullfilename="";
    pathname += usernameC;
    int flag = 0,lines=0;
    
    for (int i = 0; i < 10; i++)
    {
        fullfilename = pathname;
        fullfilename += "/save";
        fullfilename += to_string(i+1);
        fullfilename += ".dat";
        flag = _access(fullfilename.c_str(), 4);
        if (flag != 0)
        {
            read_infos[i].active = 0;
        }
        else
        {
            //writelog("found save.");
            
            lines = ReadFile(fullfilename.c_str(), readbuf);
            if (lines > 0)
            {
                read_infos[i].active = 1;
                if (!file_private_verify(fullfilename))
                {
                    writelog("file cert failed!");
                    writelog(fullfilename.c_str());
                    read_infos[i].active = 2;
                }
                for (int j = 0; j < 64; j++)
                {
                    if (readbuf[j] == "[user]")
                    {
                        if (readbuf[j + 1] != usernameC)
                        {
                            read_infos[i].active = 2;
                            writelog("save cert failed!(username)");
                            writelog(fullfilename.c_str());
                            //continue;
                        }
                    }
                    else if (readbuf[j] == "[index]")
                    {
                        if (stoi(readbuf[j + 1]) != i+1)
                        {
                            read_infos[i].active = 2;
                            writelog("save cert failed!(index)");
                            writelog(fullfilename.c_str());
                            //continue;
                        }
                    }
                    else if (readbuf[j] == "[mapsize]")
                    {
                        
                        read_infos[i].mapsize = readbuf[j + 1];
                        
                    }
                    else if (readbuf[j] == "[flag]")
                    {
                        read_infos[i].flag = readbuf[j + 1];

                    }
                    else if (readbuf[j] == "[time]")
                    {
                        read_infos[i].time = readbuf[j + 1];
                    }
                    else if (readbuf[j] == "[opened_cube]")
                    {
                        read_infos[i].opened_cube = readbuf[j + 1];
                    }
                    else if (readbuf[j] == "[gamedata]")
                    {
                        read_infos[i].gamedata = readbuf[j + 1];
                    }
                }
                
            }
            else
            {
                writelog("corrupted file!");
                writelog(fullfilename.c_str());
                read_infos[i].active = 2;
            }
            
        }
    }
    
    //int lines = ReadFile("./save/AllUsersProfile.dat", readbuf);
    
    thread_IO_request_read_all_info = 0;
    return;
}
void update_profile()
{
    string readbuf[64] = { "" }, writebuf[64] = { "" };
    string fullfilename = "./save/AllUsersProfile.dat";
    int lines;
    thread_IO_request_update_profile = 0;
    if (!file_private_verify(fullfilename))
    {
        writelog("file cert failed!");
        writelog(fullfilename.c_str());
        return;
    }
    else
    {
        lines = ReadFile(fullfilename.c_str(), readbuf);
    }
    
    if (lines < 1)
    {
        writelog("corrupted file!");
        writelog(fullfilename.c_str());
    }
    else
    {
        writebuf[0] = "[AllUsers]";
        writebuf[1] = "[Username]";
        writebuf[2] = usernameC;
        writebuf[3] = "[long_credit]";
        writebuf[4] = "0";
        WriteFile(fullfilename.c_str(), writebuf);
        certfile(fullfilename);
    }
    return;
}
void game_write_single_save();
void game_read_single_save();
void game_delete_single_save();
void game_record();
void game_read_history();
void game_read_highscore();


unsigned __stdcall File_IO(LPVOID lpParameter)
{
    ofstream file;
    int waitnum = 0,errnum=0;
    bool r = 0;
    string err = "Open File Error!\n";
    err += filename_dbg;

    


    

    file.open(filename_dbg, ios::app);

    while (!file.is_open())
    {
        file.open(filename_dbg, ios::app);
        errnum++;
        if (errnum > 5)
        {
            quit_single = 1;
            return 0;
        }
        MessageBoxEx(NULL, err.c_str(), "IO Error", MB_OK | MB_ICONSTOP, NULL);
        Sleep(500);
    }
    filebuf1[0] = "Program start at ";
    
    filebuf1[0]+=to_string(st.wYear);
    filebuf1[0] += "-";
    filebuf1[0] += to_string(st.wMonth);
    filebuf1[0] += "-";
    filebuf1[0] += to_string(st.wDay);
    filebuf1[0] += " ";
    filebuf1[0] += to_string(st.wHour);
    filebuf1[0] += ":";
    filebuf1[0] += to_string(st.wMinute);
    filebuf1[0] += ":";
    filebuf1[0] += to_string(st.wSecond);
    fileinit();
    readConfig();
    writelog("read config and translation complete.");
    while (1)
    {
        waitnum++;
        for (int i = 0; i < 64; i++)
        {
            if (filebuf1[i] != "")
            {
                
                file << filebuf1[i] << endl;
                filebuf1[i] = "";
                r = 1;
            }


        }

        if (waitnum == 200)
        {
            waitnum = 0;
            if (r)
            {
                file.close();
                Sleep(100);
                file.open(filename_dbg, ios::app);
                while (!file.is_open())
                {
                    errnum++;
                    if (errnum > 5)
                    {
                        quit_single = 1;
                        return 0;
                    }
                    MessageBoxEx(NULL, err.c_str(), "IO Error", MB_OK | MB_ICONSTOP, NULL);
                    Sleep(500);
                    file.open(filename_dbg, ios::app);
                }
                r = 0;
            }


        }
        if (thread_IO_request_update_profile)
        {
            update_profile();
        }
        if (thread_IO_request_record_game>0)
        {
            //writelog("record0");
            thread_IO_request_save_gameMS = 0;
            game_record();
            //writelog("record1");
        }
        if (thread_IO_request_delete_game>0)
        {
            game_delete_single_save();
        }
        if (thread_IO_request_read_all_info)
        {
            game_read_all_info();
        }
        if (thread_IO_request_load_gameMS > 0)
        {
            game_read_single_save();
        }
        if (thread_IO_request_save_gameMS > 0)
        {
            game_write_single_save();
        }
        
        if (thread_IO_request_save_config)
        {
            Sleep(50);
            saveConfig();
            thread_IO_request_save_config = 0;
        }
        if (thread_IO_request_read_history > 0)
        {
            game_read_history();
        }
        if (thread_IO_request_read_highscore > 0)
        {
            game_read_highscore();
        }
        if (thread_IO_request_userinit>0)
        {
            userinit();
        }
        if (thread_IO_request_read_config)
        {
            thread_IO_request_read_config = 0;
            readConfig();
        }
        if (normal_quit||quit_single)
        {
            //writelog("quit single received");
            file.close();
            return 0;
        }
        Sleep(10);
    }
}


//读写线程部分结束


//音频线程部分
struct ALcoms
{
    string name = "";
    bool loaded = 0;
    ALint state = NULL;                            // The state of the sound source
    ALuint bufferID = NULL;                        // The OpenAL sound buffer ID
    ALuint sourceID = NULL;                        // The OpenAL sound source
    ALsizei size2 = NULL;                          // For wave format
    ALenum format = NULL;                          // The sound data format
    ALsizei freq = NULL;                           // The frequency of the sound data
    //vector<char> bufferData;                // The sound buffer data from file
};
int se[256] = { 0 };
bool LoadOGG(const char* name, ALuint* buffer, ALsizei* freq,
    ALenum* format,
    ALsizei* size)
{

    //writelog("in load 1");
    if (strcmp(name,"")==0)
    {
        writelog("loadogg : filename cannot be null!");
        return 0;
    }
    vector<char> bufferData;                // The sound buffer data from file
    bool endian = 0;                         // 0 for Little-Endian, 1 for Big-Endian
    int bitStream;
    long bytes;
    char array[BUFFER_SIZE];                // Local fixed size array
    FILE* f;
    //bool success = 0;
    fopen_s(&f, name, "rb");

    if (f == NULL)
    {
        writelog("loadogg : can not open file");
        writelog(name);
        return false;
    }

    vorbis_info* pInfo;
    OggVorbis_File oggFile;

    // Try opening the given file
    if (ov_open(f, &oggFile, NULL, 0) != 0)
    {
        writelog("loadogg : format error");
        writelog(name);
        fclose(f);
        return false;
    }


    pInfo = ov_info(&oggFile, -1);
    //writelog("in load 2");
    if (pInfo->channels == 1)
        *format = AL_FORMAT_MONO16;
    else
        *format = AL_FORMAT_STEREO16;

    *freq = pInfo->rate;


    do
    {
        bytes = ov_read(&oggFile, array, BUFFER_SIZE, endian, 2, 1, &bitStream);

        if (bytes < 0)
        {
            ov_clear(&oggFile);
            fclose(f);
            writelog("loadogg : format error 2");
            writelog(name);
            bufferData.clear();
            return 0;
        }

        bufferData.insert(bufferData.end(), array, array + bytes);
    } while (bytes > 0);
    // Upload sound data to buffer
    //writelog("in load 3");

    ov_clear(&oggFile);
    fclose(f);
    *size = bufferData.size();
    //writelog(to_string(*format));
    //writelog(to_string(*freq));
    ////writelog(to_string(*sourceID));
    //writelog(to_string(*size));
    //writelog(to_string(*buffer));
    /*writelog("alBufferData 0");*/
    //Sleep(1000);
    int t = alGetError();
    if (t != AL_NO_ERROR)
    {
        writelog("alBufferData preparation error in load ogg!");
        writelog(name);
    }
    alBufferData(*buffer, *format, &bufferData[0],
        *size, *freq);
    /*writelog("alBufferData 1");*/
    //Sleep(1000);
    t = alGetError();
    if ( t!= AL_NO_ERROR)
    {
        writelog("alBufferData error in load ogg!");
        writelog(name);
    }
    bufferData.clear();
    return true;
}


struct WAVE_Data {
    char subChunkID[4]; //should contain the word data
    long subChunk2Size; //Stores the size of the data block
};

struct WAVE_Format {
    char subChunkID[4];
    long subChunkSize;
    short audioFormat;
    short numChannels;
    long sampleRate;
    long byteRate;
    short blockAlign;
    short bitsPerSample;
};

struct RIFF_Header {
    char chunkID[4];
    //char chunkSize2[4];
    long chunkSize;//size not including chunkSize or chunkID
    char format[4];
};

bool loadWavFile(const std::string filename, ALuint* buffer,
    ALsizei* size, ALsizei* frequency,
    ALenum* format) {
    //Local Declarations
    FILE* soundFile = NULL;
    WAVE_Format wave_format;
    RIFF_Header riff_header;
    WAVE_Data wave_data;
    unsigned char* data;
    //string dbg = "";
    //int buf = 0;
    /*writelog("load wave 1");*/
    try {
        fopen_s(&soundFile, filename.c_str(), "rb");
        if (!soundFile)
            throw (filename);

        // Read in the first chunk into the struct
        fread(&riff_header, sizeof(RIFF_Header), 1, soundFile);
        /*dbg += to_string(buf);
        writelog("load wave 2");
        Sleep(500);
        for (int i = 0; i < 4; i++)
        {
            dbg += to_string(riff_header.chunkID[i]);
        }
        for (int i = 0; i < 4; i++)
        {
            dbg += to_string(riff_header.format[i]);
        }*/
        //check for RIFF and WAVE tag in memeory
        //writelog(to_string(riff_header.chunkID));
        if ((riff_header.chunkID[0] != 'R' ||
            riff_header.chunkID[1] != 'I' ||
            riff_header.chunkID[2] != 'F' ||
            riff_header.chunkID[3] != 'F') ||
            (riff_header.format[0] != 'W' ||
                riff_header.format[1] != 'A' ||
                riff_header.format[2] != 'V' ||
                riff_header.format[3] != 'E'))
            throw ("Invalid RIFF or WAVE Header");

        //Read in the 2nd chunk for the wave info
        fread(&wave_format, sizeof(WAVE_Format), 1, soundFile);
        /*dbg += to_string(buf);
        for (int i = 0; i < 4; i++)
        {
            dbg += to_string(wave_format.subChunkID[i]);
        }*/
        //check for fmt tag in memory
        if (wave_format.subChunkID[0] != 'f' ||
            wave_format.subChunkID[1] != 'm' ||
            wave_format.subChunkID[2] != 't' ||
            wave_format.subChunkID[3] != ' ')
            throw ("Invalid Wave Format");
        /*writelog("load wave 3");
        Sleep(500);*/
        //check for extra parameters;
        if (wave_format.subChunkSize > 16)
            fseek(soundFile, sizeof(short), SEEK_CUR);

        //Read in the the last byte of data before the sound file
        fread(&wave_data, sizeof(WAVE_Data), 1, soundFile);
        /*dbg += to_string(buf);
        for (int i = 0; i < 4; i++)
        {
            dbg += to_string(wave_data.subChunkID[i]);
        }*/
        /*writelog("load wave 2");
        writelog(dbg);
        writelog(to_string(alGetError()));
        Sleep(500);*/
        //check for data tag in memory
        if (wave_data.subChunkID[0] != 'd' ||
            wave_data.subChunkID[1] != 'a' ||
            wave_data.subChunkID[2] != 't' ||
            wave_data.subChunkID[3] != 'a')
            throw ("Invalid data header");
       
        //Allocate memory for data
        data = new unsigned char[wave_data.subChunk2Size];
        
        // Read in the sound data into the soundData variable
        if (!fread(data, wave_data.subChunk2Size, 1, soundFile))
            throw ("error loading WAVE data into struct!");
        
        //Now we set the variables that we passed in with the
        //data from the structs
        *size = wave_data.subChunk2Size;
        *frequency = wave_format.sampleRate;
        //The format is worked out by looking at the number of
        //channels and the bits per sample.
        if (wave_format.numChannels == 1) {
            if (wave_format.bitsPerSample == 8)
                *format = AL_FORMAT_MONO8;
            else if (wave_format.bitsPerSample == 16)
                *format = AL_FORMAT_MONO16;
        }
        else if (wave_format.numChannels == 2) {
            if (wave_format.bitsPerSample == 8)
                *format = AL_FORMAT_STEREO8;
            else if (wave_format.bitsPerSample == 16)
                *format = AL_FORMAT_STEREO16;
        }
        //create our openAL buffer and check for success
        //alGenBuffers(1, buffer);
        //errorCheck();
        //now we put our data into the openAL buffer and
        //check for success
        
        alBufferData(*buffer, *format, (void*)data,
            *size, *frequency);
        if (alGetError() != AL_NO_ERROR)
        {
            writelog("alBufferData error in load wav!");
        }
        
        //errorCheck();
        //clean up and return true if successful
        fclose(soundFile);
        return true;
    }
    catch (std::string error) {
        //our catch statement for if we throw a string
        writelog("failed to load wave file:"+ error);
        
        Sleep(100);
        //clean up memory if wave loading fails
        if (soundFile != NULL)
            fclose(soundFile);
        //return false to indicate the failure to load wave
        return false;
    }
}
//string ogginfo1[16] = { "" }, ogginfo2[16] = { "" };


struct musicinfo
{
    string name = "";
};


struct seinfo
{
    
    string filename = "";
    //bool isplaying = 0;
    
    ALcoms ALwav[8];
};
void playeffectsound(int num)
{
    for (int i = 0; i < 256; i++)
    {
        if (se[i] != 0)
        {
            continue;
        }
        se[i] = num;
        break;
    }
    return;
}



//int stage1_music_num = 0, stage2_music_num = 0;
//接收主线程信号播放音频
ALCdevice* device;
ALCcontext* context;


int thread_smusic_response = 0, isloop = 1, last_music_request = 2;
int current_playing_music = 0;
ALcoms ALogg[32];
unsigned __stdcall ThreadPlaySingleMusic(LPVOID lpParameter)
{
    
    thread_smusic_response = 1;
    int current_playing_music_s = current_playing_music;
    //float check_status_time = 0;
    //writelog("start swtich music.");
    if (!ALogg[current_playing_music_s].loaded)
    {
        alGenBuffers(1, &ALogg[current_playing_music_s].bufferID);
        //writelog(to_string(alGetError()));
        alGenSources(1, &ALogg[current_playing_music_s].sourceID);
        int t = alGetError();
        if (t != AL_NO_ERROR&& t != AL_INVALID_NAME)    //bug?
        {
            writelog(ALogg[current_playing_music_s].name + " alGenSources error in load ogg!  "+ to_string(t));
            
        }
        // Set the source and listener to the same location

        alSource3f(ALogg[current_playing_music_s].sourceID, AL_POSITION, 0.0f, 0.0f, 0.0f);
        t = alGetError();
        if (t != AL_NO_ERROR)
        {
            writelog("AL_POSITION error in ThreadPlaySingleMusic!");
            writelog(to_string(t));
        }
        if (!LoadOGG(ALogg[current_playing_music_s].name.c_str(), &ALogg[current_playing_music_s].bufferID, &ALogg[current_playing_music_s].freq, &ALogg[current_playing_music_s].format, &ALogg[current_playing_music_s].size2))
        {
            writelog("load oggfile failed");
        }
        if (alGetError() != AL_NO_ERROR)
        {
            writelog("error in ThreadPlaySingleMusic!  " + ALogg[current_playing_music_s].name);

            Sleep(200);
        }
        else
        {
            writelog("alBufferData passed. " + ALogg[current_playing_music_s].name);

        }
        alSourcei(ALogg[current_playing_music_s].sourceID, AL_BUFFER, ALogg[current_playing_music_s].bufferID);
        ALogg[current_playing_music_s].loaded = 1;
    }
    else
    {
        writelog("audio already in buffer. " + ALogg[current_playing_music_s].name);
    }
    
    
    
    if (ALogg[current_playing_music_s].size2 > 1000)
    {

        
        // Attach sound buffer to source
        //Sleep(1000);
        
        //writelog("ogg 2");
        alSourcef(ALogg[current_playing_music_s].sourceID, AL_GAIN, (float)set3[0].music_volume / 100.0F);  //音量
        if (isloop)
        {
            alSourcei(ALogg[current_playing_music_s].sourceID, AL_LOOPING, AL_TRUE);
        }
        else
        {
            alSourcei(ALogg[current_playing_music_s].sourceID, AL_LOOPING, AL_FALSE);
        }
        
        alSourcePlay(ALogg[current_playing_music_s].sourceID);
        //curnum = mainpagemusic;
        //writelog("playing");
    }
    else
    {
        writelog("Exception: bufferData size too low!   "+to_string(current_playing_music_s));
        Sleep(200);
        thread_smusic_response = 0;
        return 0;
    }
    while (1)
    {
        Sleep(100);
        if (thread_smusic_response == -1||normal_quit)
        {
            alSourceStop(ALogg[current_playing_music_s].sourceID);
            //alDeleteBuffers(1, &ALogg[current_playing_music_s].bufferID);
            //alDeleteSources(1, &ALogg[current_playing_music_s].sourceID);
            thread_smusic_response = 0;
            return 0;
        }
        if (thread_Audio_volume_changed_music)
        {
            
            
            alSourcef(ALogg[current_playing_music_s].sourceID, AL_GAIN, (float)set3[0].music_volume / 100.0F);  //音量
            

            thread_Audio_volume_changed_music = 0;
        }
        alGetSourcei(ALogg[current_playing_music_s].sourceID, AL_SOURCE_STATE, &ALogg[current_playing_music_s].state);
        if (ALogg[current_playing_music_s].state == AL_STOPPED || thread_Audio_switch_immediately)
        {
            thread_Audio_switch_immediately = 0;

            if (last_music_request == thread_Audio_target_music&& thread_Audio_target_music!=1)
            {
                continue;
            }
            if (thread_Audio_target_music == 0)
            {
                if (ALogg[current_playing_music_s].state != AL_STOPPED)
                {
                    Sleep(500);
                    alSourceStop(ALogg[current_playing_music_s].sourceID);
                }
                continue;
            }
            last_music_request = thread_Audio_target_music;
            alSourceStop(ALogg[current_playing_music_s].sourceID);
            //alDeleteBuffers(1, &ALogg.bufferID);
            //alDeleteSources(1, &ALogg.sourceID);
            thread_smusic_response = 0;
            return 0;
        }
        
    }
}
unsigned __stdcall ThreadPlayMusic(LPVOID lpParameter)
{
    //ALCdevice* pDevice;
    //ALCcontext* pContext;
    writelog("ThreadPlayMusic started.");
    //ALcoms ALwav[64];
    //ALcoms ALogg[64];
    srand((unsigned int)GetTickCount64());
    seinfo se_p[24];
    musicinfo stage1[16], sys[5];
    int musicnum1 = 0;
    bool no_exmusic = 0;
    
    
    sys[1].name = ".\\music\\sys\\ed.ogg";
    sys[2].name = ".\\music\\sys\\atmosphere_rainy.ogg";

    se_p[0].filename = "sounds\\Button1.wav"; //"sounds\\save.wav";
    se_p[1].filename = "sounds\\Button2.wav"; //"sounds\\alertTick.wav";
    se_p[2].filename = "sounds\\save.wav"; //"sounds\\fx_failsmoke.wav";
    se_p[3].filename = "sounds\\load.wav";
    se_p[4].filename =  "sounds\\gameover.wav";
    se_p[5].filename = "sounds\\alertTick.wav";
    se_p[6].filename = "sounds\\endofpage.wav";
    se_p[7].filename = "sounds\\boom.wav";
    se_p[8].filename = "sounds\\boom.wav";
    se_p[9].filename = "sounds\\boom.wav";
    se_p[10].filename = "sounds\\boom.wav";
    se_p[11].filename = "sounds\\boom.wav";
    se_p[12].filename = "sounds\\boom.wav";
    se_p[13].filename = "sounds\\boom.wav";
    se_p[14].filename = "sounds\\ioerror.wav";
    se_p[15].filename = "sounds\\pass.wav";
    se_p[16].filename = "sounds\\tick.wav";
    se_p[17].filename = "sounds\\loading.wav";
    //se_p[6].filename = "sounds\\ui_christmas_pop_switch.wav";
    //getFiles();
    //文件句柄，win10用long long，win7用long就可以了
    long long hFile = 0;
    //文件信息 

    struct _finddata_t fileinfo;
    string p;
    if ((hFile = _findfirst(p.assign(".\\music").append("\\*.ogg").c_str(), &fileinfo)) != -1)
    {
        do
        {
         

            if (!(fileinfo.attrib & _A_SUBDIR))
            {
                musicnum1++;
                stage1[musicnum1 - 1].name = ".\\music\\";
                stage1[musicnum1 - 1].name += fileinfo.name;
                if (musicnum1 >= 16)
                {
                    break;
                }
                //files.push_back(p.assign(path).append("\\").append(fileinfo.name));
                //if (endsWith(fileinfo.name, ".mp3"))
                //{
                //string buffp = "./bgm/" + musicsource[musicnum];
                ////buffp = "C:/Users/zxc/source/repos/Project5/x64/Debug/bgm/Arcaea_Lowiro_pragmatism-resurrection-_Laur.mp3";
                //musiclength[musicnum] = GetLong(buffp.c_str());

                //}
                //names.push_back(fileinfo.name);
            }
        } while (_findnext(hFile, &fileinfo) == 0);
        _findclose(hFile);

    }

    writelog("found "+to_string(musicnum1)+" external music file.");
    /*for (int i = 0; i < musicnum1; i++)
    {
        writelog(stage1[i].name);
    }*/
    ALogg[1].name = sys[1].name;
    ALogg[2].name = sys[2].name;

    
    int curnum = 1;
    int mainpagemusic, historypagemusic;

    if (musicnum1 > 0)
    {
        
        for (int i = 0; i < musicnum1; i++)
        {
            ALogg[3+i].name= stage1[i].name;
        }
        mainpagemusic = rand() % musicnum1+3;
        sys[3].name = ALogg[mainpagemusic].name;
        historypagemusic = mainpagemusic;
        curnum = mainpagemusic;
    }
    else
    {
        no_exmusic = 1;
        mainpagemusic = -1;
        historypagemusic = mainpagemusic;
        curnum = mainpagemusic;
    }
    
    device = alcOpenDevice(0);
    context = alcCreateContext(device, 0);
    ALboolean initStatus = alcMakeContextCurrent(context);
    if (alGetError() != AL_NO_ERROR)
    {
        writelog("openal init failed!");
    }
    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
    //writelog("in load 2");
    for (int i = 0; i < 22; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            // Create sound buffer and source
            alGenBuffers(1, &se_p[i].ALwav[j].bufferID);
            alGenSources(1, &se_p[i].ALwav[j].sourceID);
            if (alGetError() != AL_NO_ERROR)
            {
                writelog("alGenSources error in load wav!");
            }
            // Set the source and listener to the same location

            alSource3f(se_p[i].ALwav[j].sourceID, AL_POSITION, 0.0f, 0.0f, 0.0f);
        }
        

        
    }
  
    
    // Load the OGG file into memory
    //writelog("try to load ogg");
    //string logcontent = "";
    
    int oggnum = 1;
    
    thread_smusic_response = 0;
    HANDLE hThreadsAudio;
    
    //thread_smusic_response = 1;
    while (md5_result != 1)
    {
        Sleep(1);
        if (md5_result == -1)
        {
            writelog("Audio thread stopped.(verification error)");
            return 0;
        }
    }
    //if (!no_exmusic)
    //{
    //    hThreadsAudio = (HANDLE)_beginthreadex(NULL, 0, ThreadPlaySingleMusic, NULL, 0, NULL);	//创建线程
    //    if (hThreadsAudio == 0)
    //    {
    //        writelog("Failed to create sAudio thread!");
    //        //threadfailed++;

    //        thread_smusic_response = 0;
    //    }
    //}
    
    
  
    


    // Load the WAVE file into memory

    writelog("try to load wav.");
    //Sleep(100);
    for (int i = 0; i < 18; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (se_p[i].filename == "")
            {
                continue;
            }
            if (!loadWavFile(se_p[i].filename, &se_p[i].ALwav[j].bufferID, &se_p[i].ALwav[j].size2, &se_p[i].ALwav[j].freq, &se_p[i].ALwav[j].format))
            {
                writelog("load wavefile failed");
                Sleep(200);
                continue;
            }
            alSourcei(se_p[i].ALwav[j].sourceID, AL_BUFFER, se_p[i].ALwav[j].bufferID);
            alSourcef(se_p[i].ALwav[j].sourceID, AL_GAIN, (float)set3[0].se_volume / 100.0F);  //音量
        }
        
    }
    for (int i = 7; i < 14; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            alSourcef(se_p[i].ALwav[j].sourceID, AL_PITCH, 0.5f+(i-7)*0.2f);
        }
    }
    

    
    writelog("load wav completed.");
    //hThreadsAudio = (HANDLE)_beginthreadex(NULL, 0, ThreadPlaySingleMusic, NULL, 0, NULL);	//创建线程
    //if (hThreadsAudio == 0)
    //{
    //    writelog("Failed to create sAudio thread!");

    //    thread_smusic_response = 0;
    //}
    
    //alGenBuffers(1, &ALogg.bufferID);
    ////writelog(to_string(alGetError()));
    //alGenSources(1, &ALogg.sourceID);
    //if (alGetError() != AL_NO_ERROR)
    //{
    //    writelog("alGenSources error in load ogg!");
    //}
    //// Set the source and listener to the same location

    //alSource3f(ALogg.sourceID, AL_POSITION, 0.0f, 0.0f, 0.0f);
    //if (alGetError() != AL_NO_ERROR)
    //{
    //    writelog("AL_POSITION error in load ogg!");
    //    //writelog(to_string(ALogg.bufferID));
    //    Sleep(1000);
    //}
    //else
    //{
    //    writelog("AL_POSITION pass");
    //    //writelog(to_string(ALogg.bufferID));
    //    Sleep(1000);
    //}
    //if (!LoadOGG(musicpathbuf.c_str(), &ALogg.bufferID,  &ALogg.freq,&ALogg.format, &ALogg.size2))
    //{
    //    writelog("load oggfile failed");
    //}
    //writelog("ogg loaded.");
    ////writelog(to_string(ALogg[curnum].freq));
    //writelog(to_string(ALogg.format));
    //writelog(to_string(ALogg.freq));
    //writelog(to_string(ALogg.sourceID));
    //writelog(to_string(ALogg.state));
    //writelog(to_string(ALogg.bufferID));
    ////writelog(to_string(ALogg[curnum].bufferData.size()));
    //

    //if (ALogg.size2 > 1000)
    //{

    //    writelog("try to link.");
    //    Sleep(1000);
    //    //Sleep(10000);
    //    /*writelog(to_string(alGetError()));
    //    Sleep(1000);*/
    //    //alBufferData(ALogg.bufferID, ALogg.format, &ALogg.bufferData, static_cast<ALsizei>(ALogg.bufferData.size()), ALogg.freq);

    //    /*if (alGetError() != AL_NO_ERROR)
    //    {
    //        writelog("alBufferData error in load ogg!");
    //        writelog(to_string(ALogg.bufferID));
    //        Sleep(1000);
    //    }
    //    else
    //    {
    //        writelog("alBufferData pass");
    //        writelog(to_string(ALogg.bufferID));
    //        Sleep(1000);
    //    }*/
    //    // Attach sound buffer to source
    //    //Sleep(1000);
    //    alSourcei(ALogg.sourceID, AL_BUFFER, ALogg.bufferID);
    //    //writelog("ogg 2");
    //    alSourcef(ALogg.sourceID, AL_GAIN, (float)set3[0].music_volume / 100.0F);  //音量
    //    if (isloop)
    //    {
    //        alSourcei(ALogg.sourceID, AL_LOOPING, AL_TRUE);
    //    }
    //    else
    //    {
    //        alSourcei(ALogg.sourceID, AL_LOOPING, AL_FALSE);
    //    }

    //    alSourcePlay(ALogg.sourceID);
    //    //curnum = mainpagemusic;
    //    writelog("playing");
    //}
    //// Finally, play the sound!!!
    //Sleep(100);
    //alSourcePlay(se_p[0].ALwav[0].sourceID);
    
    int  current_playing_wav = 0;
    //bool dbg_wav_wait = 0;
    int dbgk = 233, threadfailed=0;
    while (1)   //循环播放
    {
        if (g_output && set3[0].thread_method == 0)
        {

            g_output->WaitForVBlank();
        }
        else
        {
            Sleep(0);
        }
        //continue;
        if (normal_quit || thread_Audio_quit_single||quit_single)
        {
            thread_smusic_response = -1;
            for (int i = 0; i < 24; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    alSourceStop(se_p[i].ALwav[j].sourceID);
                    alDeleteBuffers(1, &se_p[i].ALwav[j].bufferID);
                    alDeleteSources(1, &se_p[i].ALwav[j].sourceID);
                }
            }
            /*for (int i = 0; i < 10; i++)
            {

                alSourceStop(ALogg[i].sourceID);
                alDeleteBuffers(1, &ALogg[i].bufferID);
                alDeleteSources(1, &ALogg[i].sourceID);

            }*/
            
            alcDestroyContext(context);
            alcCloseDevice(device);
            return 0;
        }
        
        //if (dbg_wav_wait)//debug
        //{
        //    if (!rightclick)
        //    {
        //        dbg_wav_wait = 0;
        //    }
        //}
        //else
        //{
        //    
        //    if (rightclick)
        //    {
        //        dbg_wav_wait = 1;
        //        for (int i = 0; i < 4; i++)
        //        {
        //            alGetSourcei(se_p[current_playing_wav].ALwav[i].sourceID, AL_SOURCE_STATE, &se_p[current_playing_wav].ALwav[i].state);
        //            if (se_p[current_playing_wav].ALwav[i].state == AL_STOPPED|| se_p[current_playing_wav].ALwav[i].state == AL_INITIAL)
        //            {
        //                alSourcePlay(se_p[current_playing_wav].ALwav[i].sourceID);
        //                break;
        //            }

        //        }
        //        
        //        current_playing_wav++;
        //        if (current_playing_wav > 2)
        //        {
        //            current_playing_wav = 2;
        //        }
        //    }
        //}
        if (thread_Audio_volume_changed)
        {
            for (int i = 0; i < 24; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    
                    //alSourcei(se_p[i].ALwav[j].sourceID, AL_BUFFER, se_p[i].ALwav[j].bufferID);
                    alSourcef(se_p[i].ALwav[j].sourceID, AL_GAIN, (float)set3[0].se_volume / 100.0F);  //音量
                }

            }
            //for (int i = 0; i < 10; i++)
            //{
            //    alSourcef(ALogg[i].sourceID, AL_GAIN, (float)set3[0].music_volume / 100.0F);  //音量
            //}
            
            thread_Audio_volume_changed = 0;
        }
        for (int i = 0; i < 256; i++)
        {

            if (se[i] != 0)
            {
                for (int j = 0; j < 4; j++)
                {
                    alGetSourcei(se_p[se[i]-1].ALwav[j].sourceID, AL_SOURCE_STATE, &se_p[se[i]-1].ALwav[j].state);
                    if (se_p[se[i]-1].ALwav[j].state == AL_STOPPED||se_p[se[i] - 1].ALwav[j].state == AL_INITIAL)
                    {
                        alSourcePlay(se_p[se[i] - 1].ALwav[j].sourceID);
                        
                        break;
                    }
                    if (j == 3)
                    {
                        alSourcePlay(se_p[se[i] - 1].ALwav[0].sourceID);
                    }
                }

                se[i] = 0;
            }
        }
        
        
            // Query the state of the souce
        if (threadfailed == 8)
        {
            writelog("stop trying to create audio thread!");
            threadfailed++;
        }
            if (thread_smusic_response==0&& threadfailed<8)
            {
                
                
                //writelog("try to switch music");
                
                
                //alGenSources(1, &ALogg[curnum].sourceID);
                if (thread_Audio_target_music == 1)//in game
                {
                    if (!no_exmusic)
                    {
                        isloop = 0;

                        oggnum = rand() % musicnum1;
                        while (oggnum == current_playing_music && musicnum1 > 1)
                        {
                            oggnum = rand() % musicnum1;
                        }
                        //musicpathbuf = stage1[oggnum].name;

                        curnum = oggnum + 3;
                    }
                    else
                    {
                        curnum = -1;
                    }
                    
                    
                }
                else if (thread_Audio_target_music == 2)//mainpage
                {
                    isloop = 1;
                    curnum = mainpagemusic;
                    //musicpathbuf = sys[1].name;
                }
                else if (thread_Audio_target_music == 3)//historypage
                {
                    isloop = 1;
                curnum = historypagemusic;
                //musicpathbuf = sys[1].name;
                }
                //else if (thread_Audio_target_music == 4)//gameover
                //{
                //    isloop = 1;
                //
                //    
                //    //musicpathbuf = sys[7].name;
                //    curnum = 7;
                //}
                else if (thread_Audio_target_music == 5)//ed
                {
                    isloop = 0;
                curnum = 1;
                //musicpathbuf = sys[8].name;
                }
                else if (thread_Audio_target_music == 6)//op
                {
                    isloop = 1;
                    curnum = 2;
                    //musicpathbuf = sys[9].name;
                
                }
                current_playing_music = curnum;
                
                if (curnum!=-1)
                {
                    thread_smusic_response = 1;
                    hThreadsAudio = (HANDLE)_beginthreadex(NULL, 0, ThreadPlaySingleMusic, NULL, 0, NULL);	//创建线程
                    if (hThreadsAudio == 0)
                    {
                        writelog("Failed to create sAudio thread!");
                        threadfailed++;

                        thread_smusic_response = 0;
                    }
                }
                
                
                
            }
            continue;
        

    }

    // Clean up sound buffer and source
    

    return 0;
}


//音频线程部分结束

//主线程
//底层控件部分

float dursec = 0;
float scale =1;
float to_screen(float ori)
{
    return ori * scale;
}
//string settings_description[16] = { "" };
string tips[16] = { "" };

//载入文字说明
void init_string()
{
    /*settings_description[0] = "每隔1分钟自动保存进度";
    settings_description[1] = "设定每步动画所用时间，单位:s";
    settings_description[2] = "在加载过程中显示tips";
    settings_description[3] = "接受长按鼠标左键后的手势输入";
    settings_description[4] = "渲染所有效果";
    settings_description[5] = "垂直同步(重启程序后生效)";
    settings_description[6] = "4x MSAA抗锯齿(重启程序后生效)";
    settings_description[7] = "在右上角显示实时帧率";
    settings_description[8] = "音乐音量(延迟生效)";
    settings_description[9] = "音效音量";
    settings_description[10] = "在声音播放出现延迟过大问题时，可尝试修改此项";
    settings_description[11] = "允许使用自定义音频源(跳过音频文件校验)";
    settings_description[12] = "";
    settings_description[13] = "";*/


    tips[0] = "高级模式下的地雷密度达到了0.2";
    tips[1] = "猜雷比拼的既有运气也有概率的计算能力";
    tips[2] = "这款非由引擎制作的游戏充分体现了游戏引擎的重要性";
    tips[3] = "内存泄露的问题一直悬而未决，干脆鸽了";
    tips[4] = "其实初始化实际所用的时间相当短，不过不放一个加载页面总觉得缺了点什么";
    tips[5] = "初级模式下的地雷密度仅有0.1";
    tips[6] = "中级模式下的地雷密度为0.156";
    tips[7] = "地雷的生成偏向于集群式";
    tips[8] = "在作业还剩一大堆的时候，开发效率会格外的高";
    tips[9] = "对于益智类游戏，思路是关键所在";
    tips[10] = "如果加入了用户和统计功能，试图破解前请先备份存档";
    tips[11] = "由于技术原因没能实现窗口全屏，要想使用全屏显示请按 Alt+Enter";
    tips[12] = "tips绝对是夹带私货的绝佳之处...";
    tips[13] = "曾经一直对c语言抱有非常敬仰的态度，直到用它来写GUI对象";
    tips[14] = "此程序存在内存泄露，持续游玩时推荐每隔1个小时重启一次程序";
    tips[15] = "可以尝试双击、右键已打开的格子";
}
int mouseX = 0, mouseY = 0;
bool clicking = 0,right;
POINT cpos;
struct box  //点击区域
{
    bool active = 0;
    float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    bool focus = 0;
    bool clicked = 0;
    bool click_status = 0;
    bool rightclick_status = 0;
    bool right_clicked = 0;
    float clicked_time = 0;
    bool doubleclick_status = 0;
    bool double_clicked = 0;
};
box boxes[1024];
int SetBox(float x1, float x2, float y1, float y2)
{
    int k = 0;
    for (int i = 0; i < 1024; i++)
    {
        if (!boxes[i].active)
        {
            k = i;
            break;
        }
        
    }
    boxes[k].focus = 0;
    boxes[k].clicked = 0;
    boxes[k].click_status = 0;
    boxes[k].right_clicked = 0;
    boxes[k].rightclick_status = 0;
    boxes[k].clicked_time = 0;
    boxes[k].double_clicked = 0;
    boxes[k].doubleclick_status = 0;

    boxes[k].x1 = to_screen(x1);
    boxes[k].x2 = to_screen(x2);
    boxes[k].y1 = to_screen(y1);
    boxes[k].y2 = to_screen(y2);
    boxes[k].active = 1;
    return k;
}

void CheckBoxes()   //响应鼠标对应的按钮控件交互
{
    for (int i = 0; i < 1024; i++)
    {
        if (!boxes[i].active)
        {
            continue;
        }
        if (boxes[i].doubleclick_status)
        {
            boxes[i].clicked_time -= dursec;
            if (boxes[i].clicked_time < 0)
            {
                boxes[i].doubleclick_status = 0;
            }
        }
        if (cpos.x > boxes[i].x1 && cpos.x < boxes[i].x2 && cpos.y > boxes[i].y1 && cpos.y < boxes[i].y2)
        {
            boxes[i].focus = 1;

            if (boxes[i].click_status == 0 && clicking)
            {
                boxes[i].click_status = 1;
                if (page_index != 2)
                {
                    playeffectsound(1);
                }
                
            }
            else if (boxes[i].click_status == 1 && !clicking)
            {
                boxes[i].click_status = 0;
                boxes[i].clicked = 1;
                if (page_index != 2) 
                {
                    playeffectsound(2);
                }
                
                if (boxes[i].doubleclick_status)
                {
                    //writelog("double clicked.");
                    boxes[i].double_clicked = 1;
                }
                else
                {
                    boxes[i].clicked_time = 0.3;
                }
                boxes[i].doubleclick_status = !boxes[i].doubleclick_status;
                if (page_index == 1)
                {
                    //writelog(to_string(i));
                    thread_IO_request_save_config = 1;
                }
                //boxes[i].active = 0;    //防止多次激活
            }
            if (boxes[i].rightclick_status == 0 && rightclick)
            {
                boxes[i].rightclick_status = 1;
            }
            else if (boxes[i].rightclick_status == 1 && !rightclick)
            {
                boxes[i].rightclick_status = 0;
                boxes[i].right_clicked = 1;
            }
        }
        else
        {
            boxes[i].focus = 0;
            boxes[i].click_status = 0;
            boxes[i].rightclick_status = 0;
            boxes[i].doubleclick_status = 0;
        }

    }
    return;
}
ID2D1SolidColorBrush* g_pBrushWhite = NULL;
//MESSAGE_BOX -> button -> box
struct lable
{
    bool active = 0;
    float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    string text = "";
    int TextFormat=0;
    int box_index = 0;
    ID2D1SolidColorBrush* Brush1 = NULL;   //外框画笔
    ID2D1SolidColorBrush* Brush2 = NULL;   //内框画笔
    ID2D1SolidColorBrush* Brush3 = NULL;   //文字画笔
    ID2D1SolidColorBrush* Brush2_ori= NULL;
    ID2D1Bitmap* Bitmap = NULL;
};
lable buttons[128];
lable lables[128];


struct effect
{
    bool active = 0;
    int index = 0;
    float time = 0;
    float posx = 0, posy = 0;
};
effect alleffects[4096];
void cleanEffects()
{
    for (int i = 0; i < 4096; i++)
    {
        alleffects[i].active = 0;
        //alleffects[i].time = 0;
    }
    return;
}
void createEffect(int index,float posx,float posy)
{
    for (int i = 0; i < 4096; i++)
    {
        if (alleffects[i].active)
        {
            continue;
        }
        if (index == 1)
        {
            alleffects[i].time = 0;
        }
        else if(index == 2)
        {
            alleffects[i].time = 0.1f;
        }
        else if (index == 3)
        {
            alleffects[i].time = 1.0f;
        }
        
        alleffects[i].index = index;
        if (index > 1)
        {
            alleffects[i].posx = posx;
            alleffects[i].posy = posy;
        }
        alleffects[i].active = 1;
    

        break;
    }
    return;
}
void cleanAnimation();
//重置页面状态、重置按钮
void InitPage(int index)
{
    for (int i = 0; i < 128; i++)
    {
        buttons[i].active = 0;
        lables[i].active = 0;
        
    }
    for (int i = 0; i < 1024; i++)
    {
        boxes[i].active = 0;
        boxes[i].clicked = 0;
        boxes[i].right_clicked = 0;
    }
    
    switch (index)
    {
    case 0: //开始页面
        cleanAnimation();
        thread_Audio_target_music = 2;
        thread_Audio_switch_immediately = 1;
        break;
    case 1: //设置页面
        break;
    case 2:
        thread_Audio_target_music = 1;
        thread_Audio_switch_immediately = 1;
        break;
    case 3:
        
        break;
    case 4:
        
        break;
    case 5:
        thread_Audio_target_music = 3;
        //thread_Audio_switch_immediately = 1;
        break;
    case 6:
        cleanAnimation();
        break;
    case 3001:
        thread_Audio_target_music = 6;
        thread_Audio_switch_immediately = 1;
        break;
    case 3002:
        thread_Audio_target_music = 5;
        thread_Audio_switch_immediately = 1;
        break;
    default:
        break;
    }
    return;
}


void CreateLable(float x1, float y1, float x2, float y2, string text,
    ID2D1SolidColorBrush* Brush1,
    ID2D1SolidColorBrush* Brush2,
    ID2D1SolidColorBrush* Brush3,
    ID2D1Bitmap* Bitmap,
    int TextFormat)
{
    int k = 0;
    for (int i = 0; i < 128; i++)
    {
        if (lables[i].active)
        {
            continue;
        }
        k = i;
        break;
    }
    lables[k].x1 = x1;
    lables[k].x2 = x2;
    lables[k].y1 = y1;
    lables[k].y2 = y2;
    lables[k].text = text;
    lables[k].Brush1 = Brush1;
    lables[k].Brush2 = Brush2;
    lables[k].Brush3 = Brush3;
    lables[k].TextFormat = TextFormat;
    lables[k].Bitmap = Bitmap;
    //lables[k].box_index=SetBox(x1, x2, y1, y2);
    lables[k].active = 1;
    return;
}
void CreateButton(float x1, float y1, float x2, float y2, string text,
    ID2D1SolidColorBrush* Brush1,
    ID2D1SolidColorBrush* Brush2,
    ID2D1SolidColorBrush* Brush3,
    ID2D1Bitmap* Bitmap)
{
    int k = 0;
    for (int i = 0; i < 128; i++)
    {
        if (buttons[i].active)
        {
            continue;
        }
        k = i;
        break;
    }
    buttons[k].x1 = x1;
    buttons[k].x2 = x2;
    buttons[k].y1 = y1;
    buttons[k].y2 = y2;
    buttons[k].text = text;
    buttons[k].Brush1 = Brush1;
    buttons[k].Brush2 = Brush2;
    buttons[k].Brush3 = Brush3;
    buttons[k].Brush2_ori = Brush2;
    buttons[k].Bitmap = Bitmap;
    buttons[k].box_index = SetBox(x1, x2, y1, y2);
    buttons[k].active = 1;
    return;
}



//底层控件部分结束








//渲染逻辑部分

#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)  //用来检测按键的点击事件
typedef std::chrono::high_resolution_clock Clock;


ID2D1Factory1* g_pD2DFactory = NULL;	// Direct2D factory
//ID2D1HwndRenderTarget* g_pRenderTarget = NULL;	// Render target
ID2D1SolidColorBrush* g_pBrushBlack = NULL;
ID2D1SolidColorBrush*  g_pBrushQuit = NULL;
ID2D1SolidColorBrush* g_pBrushGray = NULL;
ID2D1SolidColorBrush* g_pBrushLightGray = NULL;
ID2D1SolidColorBrush* g_pBrushBlue = NULL;	// A brush, reflect the line color
ID2D1SolidColorBrush* g_pBrushDarkBlue = NULL;
ID2D1SolidColorBrush* g_pBrushLightBlue = NULL;
ID2D1SolidColorBrush* g_pBrushYellow = NULL;
ID2D1SolidColorBrush* g_pBrushLightYellow = NULL;
ID2D1SolidColorBrush* g_pBrushGreen = NULL;
ID2D1SolidColorBrush* g_pBrushLightGreen = NULL;
ID2D1SolidColorBrush* g_pBrushtext = NULL;
//ID2D1SolidColorBrush* g_pBrushtank = NULL;
ID2D1SolidColorBrush* g_pBrushRed = NULL;
ID2D1SolidColorBrush* g_pBrushPurple = NULL; 
ID2D1SolidColorBrush* g_pBrushBrown = NULL;
ID2D1SolidColorBrush* BrushRand[8] = { NULL };
ID2D1SolidColorBrush* g_pBrushBGSelect = NULL;
ID2D1SolidColorBrush* g_pBrushLight = NULL;
ID2D1SolidColorBrush* g_pBrushDark = NULL;
ID2D1SolidColorBrush* g_pBrushPink = NULL;
ID2D1LinearGradientBrush* g_pLinearGradientBrush = NULL;
ID2D1RadialGradientBrush* g_pRadialGradientBrush = NULL;


ID2D1BitmapBrush1* g_pBitmapBrushUI[32] = { NULL };
ID2D1BitmapBrush1* g_pBitmapBrushSetting[16] = { NULL };
//ID2D1BitmapBrush1** g_pBitmapBrushTankusing = NULL;
//ID2D1BitmapBrush1** g_pBrushbulletusing = NULL;
//ID2D1SolidColorBrush** g_pBrushbullettype = NULL;
IDWriteFactory* g_pDWriteFactory = NULL;
IDWriteTextFormat* g_pTextFormat = NULL;
IDWriteTextFormat* g_pTextFormatLarge = NULL;
IDWriteTextFormat* g_pTextFormatL = NULL;
IDWriteTextFormat* g_pTextFormatmini = NULL;
IDWriteTextFormat* g_pTextFormatminiL = NULL;
IDWriteTextFormat* g_pTextFormat2 = NULL;
IDWriteTextFormat* g_pTextFormatNormal = NULL;
ID2D1Bitmap* ppBitmap1 = NULL;
ID2D1Bitmap* ppBitmap2 = NULL;
//ID2D1Bitmap* ppBitmaptank[32] = { NULL };
//ID2D1Bitmap* ppBitmapanimeusing = NULL;
ID2D1Bitmap1* g_pD2DTargetBimtap = NULL;
ID2D1Bitmap* g_pD2DBimtapUI[32] = { nullptr };
IWICImagingFactory* pIWICFactory = NULL;
ID2D1Device* g_pD2DDevice = NULL;
ID2D1DeviceContext* g_pD2DDeviceContext;

DXGI_ADAPTER_DESC Adapter;

D3D_FEATURE_LEVEL m_featureLevel;
ID3D11Device* pD3DDevice = NULL;
ID3D11DeviceContext* pD3DDeviceImmediateContext = NULL;
//ID3D11RenderTargetView* mRenderTargetView = NULL;
IDXGIDevice1* pDxgiDevice = NULL;
DXGI_SWAP_CHAIN_DESC1 sd;
IDXGIAdapter* pDxgiAdapter = NULL;
IDXGIFactory2* g_pDxgiFactory = NULL;
IDXGISwapChain1* g_pSwapChain = NULL;
//ID3D11Texture2D
IDXGISurface* pDxgiBackBuffer = nullptr;
//IDXGISurface* pDxgiFrontBuffer = nullptr;
//IDWriteFontFile* fontFile[8];
int DrawCallNum = 0;

Clock::time_point current;
Clock::time_point start, end, startb, endb;


DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc;
D2D1_BITMAP_PROPERTIES1 targetbitmapProperties = D2D1::BitmapProperties1(
    D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
    D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)

);
D2D1_POINT_2F point1 = D2D1::Point2F(100.0F, 100.0F);
D2D_RECT_F nullrect = D2D1::RectF();
RECT rc;
D2D1_RECT_F textLayoutRect, fpsRect, textrbRect;

//D2D1_RECT_F picrect = D2D1::RectF(400.0F, 50.0F, 1400.0F, 50.0F);
RECT windowrect;
int width, height, frame = 0, rectnum = 1, fps = 0;
int dur20, cframe = 0, nframe = 0, durnanosec = 0;

int nWidth = GetSystemMetrics(SM_CXSCREEN);  //屏幕宽度    
int nHeight = GetSystemMetrics(SM_CYSCREEN); //屏幕高度
bool  isloadfiles = 0, isloading = 0, Vsync = 1, mEnable4xMsaa = 1, fullSrcState = 0;
bool keyw = 0, keya = 0, keys = 0, keyd = 0, keyspace = 0;
int mouse_move_direction = 1, mousex_pre = 0, mousey_pre = 0;
float mouse_move_duration = 0, mouse_wait_duration=0;
//string dbgstring = "";
HWND hWnd, FocusWindow = NULL;
//int gsampleCountOut, gmaxQualityLevel;
float checkfocustime = 0, expand_input_time=0.1,esc_input_time=0;
bool input_expand = 0;
char expand_input_key = NULL;
void input()
{
    
    if (checkfocustime > 0.1)
    {
        FocusWindow = GetFocus();
        checkfocustime = 0;
    }
    else
    {
        checkfocustime += dursec;
    }
    esc_input_time += dursec;
    keyw = 0, keya = 0, keys = 0, keyd = 0, keyspace = 0;
    clicking = 0, rightclick = 0;
    if (hWnd == FocusWindow)
    {
        if (KEY_DOWN(27)&&page_index!=0)
        {
            if (esc_input_time > 0.5)
            {
                boxes[buttons[0].box_index].clicked = 1;
                esc_input_time = 0;

            }
            
        }
        if (KEY_DOWN(VK_LBUTTON))
        {
            clicking = 1;
        }
        if (KEY_DOWN(VK_RBUTTON))
        {
            rightclick = 1;
        }
        if (KEY_DOWN(VK_SPACE))
        {
            keyspace = 1;
        }
        if (KEY_DOWN(87) || KEY_DOWN(VK_UP))
        {
            keyw = 1;
        }
        if (KEY_DOWN(65)|| KEY_DOWN(VK_LEFT))
        {
            keya = 1;
        }
        if (KEY_DOWN(83) || KEY_DOWN(VK_DOWN))
        {
            keys = 1;
        }
        if (KEY_DOWN(68) || KEY_DOWN(VK_RIGHT))
        {
            keyd = 1;
        }
        GetCursorPos(&cpos);                       //获取鼠标在屏幕上的位置
        ScreenToClient(hWnd, &cpos);
        /*cpos.x = (float)cpos.x / scale;
        cpos.y = (float)cpos.y / scale;*/

        //if (set1[0].mouse_input&&page_index==2)
        //{
        //    mouse_wait_duration += dursec;
        //    if (mouse_wait_duration > 0.04)
        //    {
        //        
        //        if (mouse_move_direction == 1)
        //        {
        //            if ((cpos.x - mousex_pre) / mouse_wait_duration < -400 * scale)
        //            {
        //                mouse_move_duration += mouse_wait_duration;
        //            }
        //            else
        //            {
        //                mouse_move_duration = 0;
        //                if ((cpos.x - mousex_pre) / mouse_wait_duration > 400 * scale)
        //                {
        //                    mouse_move_direction = 2;
        //                }
        //                else if ((cpos.y - mousey_pre) / mouse_wait_duration < -400 * scale)
        //                {
        //                    mouse_move_direction = 3;
        //                }
        //                else if ((cpos.y - mousey_pre) / mouse_wait_duration > 400 * scale)
        //                {
        //                    mouse_move_direction = 4;
        //                }
        //            }
        //        }
        //        else if (mouse_move_direction == 2)
        //        {

        //            if ((cpos.x - mousex_pre) / mouse_wait_duration > 400 * scale)
        //            {
        //                mouse_move_duration += mouse_wait_duration;
        //            }
        //            else
        //            {
        //                mouse_move_duration = 0;
        //                if ((cpos.x - mousex_pre) / mouse_wait_duration < -400 * scale)
        //                {
        //                    mouse_move_direction = 1;
        //                }
        //                else if ((cpos.y - mousey_pre) / mouse_wait_duration < -400 * scale)
        //                {
        //                    mouse_move_direction = 3;
        //                }
        //                else if ((cpos.y - mousey_pre) / mouse_wait_duration > 400 * scale)
        //                {
        //                    mouse_move_direction = 4;
        //                }
        //            }
        //        }
        //        else if (mouse_move_direction == 3)
        //        {

        //            if ((cpos.y - mousey_pre) / mouse_wait_duration < -400 * scale)
        //            {
        //                mouse_move_duration += mouse_wait_duration;
        //            }
        //            else
        //            {
        //                mouse_move_duration = 0;
        //                if ((cpos.x - mousex_pre) / mouse_wait_duration > 400 * scale)
        //                {
        //                    mouse_move_direction = 2;
        //                }
        //                else if ((cpos.x - mousex_pre) / mouse_wait_duration < -400 * scale)
        //                {
        //                    mouse_move_direction = 1;
        //                }
        //                else if ((cpos.y - mousey_pre) / mouse_wait_duration > 400 * scale)
        //                {
        //                    mouse_move_direction = 4;
        //                }
        //            }
        //        }
        //        else if (mouse_move_direction == 4)
        //        {
        //            if ((cpos.y - mousey_pre) / mouse_wait_duration > 400 * scale)
        //            {
        //                mouse_move_duration += mouse_wait_duration;
        //            }
        //            else
        //            {
        //                mouse_move_duration = 0;
        //                if ((cpos.x - mousex_pre) / mouse_wait_duration > 400 * scale)
        //                {
        //                    mouse_move_direction = 2;
        //                }
        //                else if ((cpos.y - mousey_pre) / mouse_wait_duration < -400 * scale)
        //                {
        //                    mouse_move_direction = 3;
        //                }
        //                else if ((cpos.x - mousex_pre) / mouse_wait_duration < -400 * scale)
        //                {
        //                    mouse_move_direction = 1;
        //                }
        //            }
        //        }
        //        mousex_pre = cpos.x;
        //        mousey_pre = cpos.y;
        //        mouse_wait_duration = 0;
        //    }
        //    
        //    
        //    //dbgstring = getTimeDigit((float)(cpos.x - mousex_pre));
        //    if (mouse_move_duration > 0.11)
        //    {
        //        mouse_move_duration = 0;
        //        if (mouse_move_direction == 1)
        //        {
        //            keya = 1;
        //        }
        //        else if (mouse_move_direction == 2)
        //        {
        //            keyd = 1;
        //        }
        //        else if (mouse_move_direction == 3)
        //        {
        //            keyw = 1;
        //        }
        //        else if (mouse_move_direction == 4)
        //        {
        //            keys = 1;
        //        }
        //    }
        //    
        //}
        if (input_expand)
        {
            expand_input_key = NULL;
            if (expand_input_time < 0)
            {
                expand_input_time = 0.1;
                expand_input_key = NULL;
                for (int i = 48; i < 91; i++)
                {
                    if (KEY_DOWN(i))
                    {
                        if (i < 58)
                        {
                            expand_input_key = i;
                        }
                        else if (GetKeyState(VK_CAPITAL))
                        {
                            expand_input_key = i;
                        }
                        else
                        {
                            expand_input_key = i + 32;
                        }


                    }
                }
                if (KEY_DOWN(8))//backspace
                {
                    expand_input_key = 35;
                }
                if (KEY_DOWN(13))//enter
                {
                    expand_input_key = 36;
                }
            }
            else
            {
                expand_input_time -= dursec;
            }
            
        }
    }
    
    return;
}

UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
UINT sampleCountOut = 1;

void CreateD3DResource(HWND hWnd)
{
    HRESULT hr;
    D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1
    };

    // 创建设备
    hr = D3D11CreateDevice(
        // 设为空指针选择默认设备
        NULL,
        // 强行指定硬件渲染
        D3D_DRIVER_TYPE_HARDWARE,
        // 没有软件接口
        nullptr,
        // 创建flag
        creationFlags,
        // 欲使用的特性等级列表
        featureLevels,
        // 特性等级列表长度
        ARRAYSIZE(featureLevels),
        // SDK 版本
        D3D11_SDK_VERSION,
        // 返回的D3D11设备指针
        &pD3DDevice,
        // 返回的特性等级
        &m_featureLevel,
        // 返回的D3D11设备上下文指针
        &pD3DDeviceImmediateContext
    );
    if (FAILED(hr))
    {
        writelog("D3D11CreateDevice Failed.");
        //return  false;
    }
    ZeroMemory(&sd, sizeof(sd));
    // 是否使用4X MSAA?
    if (set2[0].MSAA)
    {
        UINT m4xMsaaQuality = 0;
        for (UINT sampleCount = 1; sampleCount <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; sampleCount++)
        {
            UINT maxQualityLevel = 0;
            HRESULT hr = pD3DDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, sampleCount, &maxQualityLevel);


            //ERROR_HANDLE(hr == S_OK, L"CheckMultisampleQualityLevels failed.", MOD_GRAPHIC);

            if (hr == S_OK && maxQualityLevel > 0) {
                //LOG(logDEBUG1, "MSAA " << sampleCount << "X supported with " << maxQualityLevel << " quality levels.", MOD_GRAPHIC);
                sampleCountOut = sampleCount;
                m4xMsaaQuality = 0;
            }

        }
        sd.SampleDesc.Count = sampleCountOut;
        // m4xMsaaQuality是通过CheckMultisampleQualityLevels()方法获得的
        sd.SampleDesc.Quality = m4xMsaaQuality;
    }
    // NoMSAA
    else
    {
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
    }
    //sampleCountOut = 4;
    //gsampleCountOut = sampleCountOut;
    //gmaxQualityLevel = m4xMsaaQuality;
    //sampleCountOut = 4;
    //hr= pD3DDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality);
    //assert(m4xMsaaQuality > 0);
    if (FAILED(hr))
    {
        writelog("CheckMultisampleQualityLevels Failed.");
        //return  false;
    }
    hr = pD3DDevice->QueryInterface(__uuidof(IDXGIDevice1), (void**)&pDxgiDevice);
    if (FAILED(hr))
    {
        writelog("Query D3DDevice Interface Failed.");
        //return  false;
    }
    hr = pDxgiDevice->GetAdapter(&pDxgiAdapter);
    if (FAILED(hr))
    {
        writelog("GetAdapter Failed.");
        //return  false;
    }
    hr = pDxgiDevice->SetGPUThreadPriority(4);
    if (FAILED(hr))
    {
        writelog("SetGPUThreadPriority Failed.");
        //return  false;
    }
    hr = pDxgiAdapter->GetDesc(&Adapter);
    if (FAILED(hr))
    {
        writelog("GetDesc Failed.");
        //return  false;
    }
    hr = pDxgiAdapter->GetParent(IID_PPV_ARGS(&g_pDxgiFactory));
    if (FAILED(hr))
    {
        writelog("GetParent Failed.");
        //return  false;
    }
    

    //sd.Width = rc.right-rc.left;    // 使用窗口客户区宽度
    //sd.Height = rc.bottom-rc.top;
    //sd.RefreshRate.Numerator = 0;
    //sd.RefreshRate.Denominator = 1;
    //sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    //sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    //sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.Width = rc.right - rc.left;    // 使用窗口客户区宽度
    sd.Height = rc.bottom - rc.top;
    sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.Stereo = FALSE;

    //sd.SampleDesc.Count = 1;
    //sd.SampleDesc.Quality = 0;
    //sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    
    //sd.Width = 2560;
    //sd.Height = 1440;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 1;
    //sd.OutputWindow = hWnd;
    //sd.Windowed = true;
    //sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    //sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    //sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferCount = 1;
    sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    sd.Scaling = DXGI_SCALING_STRETCH;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    ZeroMemory(&fullscreenDesc, sizeof(fullscreenDesc));
    fullscreenDesc.RefreshRate.Denominator = 1;
    fullscreenDesc.RefreshRate.Numerator = 300;
    fullscreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    fullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
    fullscreenDesc.Windowed = 1;

    if (pD3DDevice == NULL)
    {
        writelog("pD3DDevice==0");
        return;
    }
    hr = g_pDxgiFactory->CreateSwapChainForHwnd(
        pD3DDevice,
        hWnd,
        &sd,
        &fullscreenDesc,
        //false ? &fullscreenDesc : nullptr, 
        nullptr,
        &g_pSwapChain
    );
    if (FAILED(hr))
    {
        writelog("CreateSwapChainForHwnd Failed.");
        return;
    }
    hr = pDxgiDevice->SetMaximumFrameLatency(1);
    hr = g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pDxgiBackBuffer));
    if (FAILED(hr))
    {
        writelog( "GetBuffer Failed.");
        return ;
    }
    hr = g_pSwapChain->GetContainingOutput(&g_output);

    if (FAILED(hr))
    {
        writelog("GetContainingOutput Failed.");
        return ;
    }




    //hr = pDxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pDxgiAdapter);
    //if (FAILED(hr))
    //{
    //    MessageBox(NULL, "GetParent Failed.", 0, 0);
    //    //return  false;
    //}


    //pD3DDevice->CreateRenderTargetView(pDxgiBackBuffer, 0, &mRenderTargetView);

    //MessageBox(hWnd, "CreateD3DResource succeed", "1", 0);
}


HRESULT LoadBitmapFromFile(
    ID2D1RenderTarget* pRenderTarget,
    IWICImagingFactory* pIWICFactory,
    PCWSTR uri,
    UINT destinationWidth,
    UINT destinationHeight,
    ID2D1Bitmap** ppBitmap
    //ID2D1BitmapBrush* g_pBitmapBrush
)
{
    HRESULT hr = S_OK;

    IWICBitmapDecoder* pDecoder = NULL;
    IWICBitmapFrameDecode* pSource = NULL;
    IWICStream* pStream = NULL;
    IWICFormatConverter* pConverter = NULL;
    IWICBitmapScaler* pScaler = NULL;
    if (pIWICFactory == NULL)
    {
        writelog("pIWICFactory ==0");
        //MessageBox(NULL, "pIWICFactory ==0", "Error", 0);
    }
    //MessageBox(NULL, uri, "Error", 0);
    hr = pIWICFactory->CreateDecoderFromFilename(
        uri,
        NULL,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &pDecoder
    );
    if (SUCCEEDED(hr))
    {

        // Create the initial frame.
        hr = pDecoder->GetFrame(0, &pSource);
    }
    else
    {
        writelog("Load pDecoder failed!");
        //MessageBox(hWnd, "Load pDecoder failed!\ntest.jpg->1", to_string(hr).c_str(), 0);
        return hr;
    }
    if (SUCCEEDED(hr))
    {
        hr = pIWICFactory->CreateFormatConverter(&pConverter);
    }
    else
    {
        writelog("Load pSource failed!");
        //MessageBox(hWnd, "Load pSource failed!\ntest.jpg->1", "Error", 0);
        return hr;
    }
    // If a new width or height was specified, create an
// IWICBitmapScaler and use it to resize the image.
    UINT originalWidth, originalHeight;
    hr = pSource->GetSize(&originalWidth, &originalHeight);
    if (destinationWidth != 0 || destinationHeight != 0)
    {

        if (SUCCEEDED(hr))
        {
            if (destinationWidth == 0)
            {
                FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(originalHeight);
                destinationWidth = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
            }
            else if (destinationHeight == 0)
            {
                FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(originalWidth);
                destinationHeight = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
            }


        }
    }
    else
    {
        destinationWidth = originalWidth;
        destinationHeight = originalHeight;
    }
    hr = pIWICFactory->CreateBitmapScaler(&pScaler);
    if (SUCCEEDED(hr))
    {
        hr = pScaler->Initialize(
            pSource,
            destinationWidth,
            destinationHeight,
            WICBitmapInterpolationModeCubic
        );
    }
    if (SUCCEEDED(hr))
    {
        hr = pConverter->Initialize(
            pScaler,
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone,
            NULL,
            0.f,
            WICBitmapPaletteTypeMedianCut
        );
    }
    if (SUCCEEDED(hr))
    {
        // Create a Direct2D bitmap from the WIC bitmap.
        hr = g_pD2DDeviceContext->CreateBitmapFromWicBitmap(
            pConverter,
            NULL,
            ppBitmap
        );
    }


    SAFE_RELEASE(pDecoder);
    SAFE_RELEASE(pSource);
    SAFE_RELEASE(pStream);
    SAFE_RELEASE(pConverter);
    SAFE_RELEASE(pScaler);
    //MessageBox(NULL, "Load Bitmap From File succeed", "4", 0);
    return hr;
}





void CreateD2DResource(HWND hWnd)
{
    if (!g_pD2DDeviceContext)
    {
        HRESULT hr;
        //创建工厂
        hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
            __uuidof(ID2D1Factory1),
            reinterpret_cast<void**>(&g_pD2DFactory));
        if (FAILED(hr))
        {
            writelog("Create D2D factory failed!");
            return;
        }
        hr = g_pD2DFactory->CreateDevice(pDxgiDevice, &g_pD2DDevice);
        if (FAILED(hr))
        {
            writelog("Create D2D Device failed!");
            return;
        }
        hr = g_pD2DDevice->CreateDeviceContext(
            D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
            &g_pD2DDeviceContext
        );
        if (FAILED(hr))
        {
            writelog("Create D2D DeviceContext failed!");
            return;
        }

        hr = g_pD2DDeviceContext->CreateBitmapFromDxgiSurface(
            pDxgiBackBuffer,
            &targetbitmapProperties,
            &g_pD2DTargetBimtap
        );
        if (FAILED(hr))
        {
            writelog("CreateBitmapFromDxgiSurface Failed.");
            return;
        }
        g_pD2DDeviceContext->SetTarget(g_pD2DTargetBimtap);
        if (FAILED(hr))
        {
            writelog("SetTarget Failed.");
            //return  false;
        }
        g_pD2DDeviceContext->SetUnitMode(D2D1_UNIT_MODE_PIXELS);
        if (FAILED(hr))
        {
            writelog("SetUnitMode Failed.");
            return;
        }
        writelog("d2d stage1 passed.");
        // Obtain the size of the drawing area
        RECT rc;
        GetClientRect(hWnd, &rc);

        // Create a Direct2D render target
        //通过工厂创建RenderTarget渲染对象
        /*
        hr = g_pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(
                hWnd,
                D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)
            ),
            &g_pRenderTarget
        );
        if (FAILED(hr))
        {
            MessageBox(hWnd, "Create render target failed!", "Error", 0);
            return;
        }
        */


        // Create a brush
        //通过渲染对象 创建一个固定颜色的画刷
        hr = g_pD2DDeviceContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::Black),
            &g_pBrushBlack
        );
        hr = g_pD2DDeviceContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::Black),
            &g_pBrushQuit
        );
        
        hr = g_pD2DDeviceContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::White),
            &g_pBrushWhite
        );
        hr = g_pD2DDeviceContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::Gray),
            &g_pBrushGray
        ); 
        hr = g_pD2DDeviceContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::LightGray),
            &g_pBrushLightGray
        );

        hr = g_pD2DDeviceContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::SkyBlue),
            &g_pBrushBlue
        );
        hr = g_pD2DDeviceContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::DarkBlue),
            &g_pBrushDarkBlue
        );
        
        hr = g_pD2DDeviceContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::LightGoldenrodYellow),
            &g_pBrushLightYellow
        );
        hr = g_pD2DDeviceContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::LightSkyBlue),
            &g_pBrushLightBlue
        );
        hr = g_pD2DDeviceContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::MediumPurple),
            &g_pBrushPurple
        );
        hr = g_pD2DDeviceContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::Yellow),
            &g_pBrushYellow
        );
        hr = g_pD2DDeviceContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::White),
            &g_pBrushLight
        );
        hr = g_pD2DDeviceContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::Black),
            &g_pBrushDark
        );
        hr = g_pD2DDeviceContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::RosyBrown),
            &g_pBrushBrown
        );
        hr = g_pD2DDeviceContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::LightPink),
            &g_pBrushPink
        );
        g_pBrushLight->SetOpacity(0.5);
        g_pBrushDark->SetOpacity(0.5);
        hr = g_pD2DDeviceContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::ForestGreen),
            &g_pBrushGreen
        );
        hr = g_pD2DDeviceContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::LightGreen),
            &g_pBrushLightGreen
        );
        hr = g_pD2DDeviceContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::Red),
            &g_pBrushRed
        );
        hr = g_pD2DDeviceContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::Black),
            &g_pBrushtext
        );
        /*hr = g_pD2DDeviceContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::Black),
            &g_pBrushtank
        );*/
        BrushRand[0] = g_pBrushGreen;
        BrushRand[1] = g_pBrushRed;
        BrushRand[2] = g_pBrushPurple;
        BrushRand[3] = g_pBrushYellow;
        BrushRand[4] = g_pBrushLightGreen;
        BrushRand[5] = g_pBrushBlue;
        BrushRand[6] = g_pBrushGreen;
        BrushRand[7] = g_pBrushGray;

        if (FAILED(hr))
        {
            writelog("Create brush failed!");
            return;
        }
        D2D1_GRADIENT_STOP gradientStops[2];
        gradientStops[0].color = D2D1::ColorF(D2D1::ColorF::White);
        gradientStops[0].position = 0.0f; 
        gradientStops[1].color = D2D1::ColorF(D2D1::ColorF::Blue);
        gradientStops[1].position = 1.0f;
        ID2D1GradientStopCollection* pGradientStops = NULL;
        // Create gradient stops collection
        hr = g_pD2DDeviceContext->CreateGradientStopCollection(
            gradientStops,
            2,
            D2D1_GAMMA_2_2,
            D2D1_EXTEND_MODE_CLAMP,
            &pGradientStops
        );
        if (FAILED(hr))
        {
            writelog("Create gradient stops collection failed!");
            return;
        }
        
        
        // Create a linear gradient brush to fill in the rectangle
        hr = g_pD2DDeviceContext->CreateLinearGradientBrush(
            D2D1::LinearGradientBrushProperties(
                D2D1::Point2F(0, 0),
                D2D1::Point2F(to_screen(1600), to_screen(900))),
            pGradientStops,
            &g_pLinearGradientBrush
        );

        if (FAILED(hr))
        {
            writelog("Create Linear gradient brush failed!");
            return;
        }
        g_pLinearGradientBrush->SetOpacity(0.4);
        gradientStops[0].color = D2D1::ColorF(D2D1::ColorF::LightBlue);
        gradientStops[1].color = D2D1::ColorF(D2D1::ColorF::SteelBlue);
        hr = g_pD2DDeviceContext->CreateGradientStopCollection(
            gradientStops,
            2,
            D2D1_GAMMA_2_2,
            D2D1_EXTEND_MODE_CLAMP,
            &pGradientStops
        );
        if (FAILED(hr))
        {
            writelog("Create gradient stops collection failed!");
            return;
        }
        hr = g_pD2DDeviceContext->CreateRadialGradientBrush(
            D2D1::RadialGradientBrushProperties(
                D2D1::Point2F(0, 0),
                D2D1::Point2F(0, to_screen(-5)),
                to_screen(50), to_screen(50)),
            pGradientStops,
            &g_pRadialGradientBrush
        );
        
        if (FAILED(hr))
        {
            writelog("Create Radial gradient brush failed!");
            return;
        }
        g_pRadialGradientBrush->SetOpacity(1.0);
        /*if (SUCCEEDED(hr))
        {
            ID2D1GradientStopCollection* pGradientStops = NULL;

            static const D2D1_GRADIENT_STOP gradientStops[] =
            {
                {   0.f,  D2D1::ColorF(D2D1::ColorF::Black, 1.0f)  },
                {   1.f,  D2D1::ColorF(D2D1::ColorF::White, 0.0f)  },
            };

            hr = g_pD2DDeviceContext->CreateGradientStopCollection(
                gradientStops,
                2,
                &pGradientStops);




            if (SUCCEEDED(hr))
            {
                hr = g_pD2DDeviceContext->CreateRadialGradientBrush(
                    D2D1::RadialGradientBrushProperties(
                        D2D1::Point2F(75, 75),
                        D2D1::Point2F(0, 0),
                        75,
                        75),
                    pGradientStops,
                    &m_pRadialGradientBrush);
            }
            pGradientStops->Release();
        }*/
        SAFE_RELEASE(pGradientStops);
        writelog("d2d stage2 passed.");

        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(&g_pDWriteFactory)
        );
        if (FAILED(hr))
        {
            writelog("Create DWrite Factory failed!");
            normal_quit = 1;
            return;
        }

        //IDWriteFontFile* pFontFile;

        //hr = g_pDWriteFactory->CreateFontFileReference(L".\\fonts\\Quarlow.ttf", nullptr, &fontFile[0]);
        //BOOL isSupported;
        //DWRITE_FONT_FILE_TYPE fileType;
        //UINT32 numberOfFonts;
        //hr = fontFile[0]->Analyze(&isSupported, &fileType, /* face type */ nullptr, &numberOfFonts);
        ////判断字体文件是否可用
        ////...

        //IDWriteFontCollection* fCollection;
        //IDWriteTextFormat* pTextFormat;
        //

        /*for (uint32_t fontIndex = 0; fontIndex < numberOfFonts; fontIndex++)
        {
            IDWriteFontFaceReference* pFontFaceReference;
            hr = pDWriteFactory->CreateFontFaceReference(pFontFile, fontIndex, DWRITE_FONT_SIMULATIONS_NONE, &pFontFaceReference);

            if (SUCCEEDED(hr))
            {
                hr = pFontSetBuilder->AddFontFaceReference(pFontFaceReference);
            }
        }*/
        int tt = AddFontResourceA(".\\fonts\\SmileySans-Oblique.ttf");  //添加字体文件

        //MessageBox(NULL, to_string(tt).c_str(), "Error", 0);
        if ( tt== 0)
        {
            writelog("Load Font Resource failed!");
            writelog(".\\fonts\\SmileySans-Oblique.ttf");
        }
        tt = AddFontResourceA(".\\fonts\\Pacifico.ttf");
        if (tt == 0)
        {
            writelog("Load Font Resource failed!");
            writelog(".\\fonts\\Pacifico.ttf");
        }
        tt = AddFontResourceA(".\\fonts\\Quarlow.ttf");
        if (tt == 0)
        {
            writelog("Load Font Resource failed!");
            writelog(".\\fonts\\Quarlow.ttf");
        }
        tt = AddFontResourceA(".\\fonts\\PublicSans-Regular.ttf");
        if (tt == 0)
        {
            writelog("Load Font Resource failed!");
            writelog(".\\fonts\\PublicSans-Regular.ttf");
        }

        hr = g_pDWriteFactory->CreateTextFormat(
            L"得意黑",                   // Font family name
            NULL,                          // Font collection(NULL sets it to the system font collection)
            DWRITE_FONT_WEIGHT_REGULAR,    // Weight
            DWRITE_FONT_STYLE_NORMAL,      // Style
            DWRITE_FONT_STRETCH_NORMAL,    // Stretch
            to_screen(36.0f),                         // Size    
            L"en-us",                      // Local
            &g_pTextFormat                 // Pointer to recieve the created object
        );
        if (FAILED(hr))
        {
            writelog("Create IDWriteTextFormat failed!");
            normal_quit = 1;
            return;
        }
        hr = g_pDWriteFactory->CreateTextFormat(
            L"Pacifico",                   // Font family name
            NULL,                          // Font collection(NULL sets it to the system font collection)
            DWRITE_FONT_WEIGHT_REGULAR,    // Weight
            DWRITE_FONT_STYLE_NORMAL,      // Style
            DWRITE_FONT_STRETCH_NORMAL,    // Stretch
            to_screen(192.0f),                         // Size    
            L"en-us",                      // Local
            &g_pTextFormatLarge                 // Pointer to recieve the created object
        );
        
        hr = g_pDWriteFactory->CreateTextFormat(    //0
            L"得意黑",                   // Font family name
            NULL,                          // Font collection(NULL sets it to the system font collection)
            DWRITE_FONT_WEIGHT_REGULAR,    // Weight
            DWRITE_FONT_STYLE_NORMAL,      // Style
            DWRITE_FONT_STRETCH_NORMAL,    // Stretch
            to_screen(32.0f),                         // Size    
            L"en-us",                      // Local
            &g_pTextFormatL                 // Pointer to recieve the created object
        );
        hr = g_pDWriteFactory->CreateTextFormat(    //2
            L"Pacifico",                   // Font family name
            NULL,                          // Font collection(NULL sets it to the system font collection)
            DWRITE_FONT_WEIGHT_REGULAR,    // Weight
            DWRITE_FONT_STYLE_NORMAL,      // Style
            DWRITE_FONT_STRETCH_NORMAL,    // Stretch
            to_screen(30.0f),                         // Size    
            L"en-us",                      // Local
            &g_pTextFormat2                 // Pointer to recieve the created object
        );
        hr = g_pDWriteFactory->CreateTextFormat(    //1
            L"Quarlow",
            //L"NSimSun",
            //L"Gabriola",                   // Font family name
            NULL,                          // Font collection(NULL sets it to the system font collection)
            DWRITE_FONT_WEIGHT_REGULAR,    // Weight
            DWRITE_FONT_STYLE_NORMAL,      // Style
            DWRITE_FONT_STRETCH_NORMAL,    // Stretch
            to_screen(40.0),                         // Size    
            L"en-us",                      // Local
            &g_pTextFormatmini                 // Pointer to recieve the created object
        );
        hr = g_pDWriteFactory->CreateTextFormat(    //1
            L"Quarlow",
                               // Font family name
            NULL,                          // Font collection(NULL sets it to the system font collection)
            DWRITE_FONT_WEIGHT_REGULAR,    // Weight
            DWRITE_FONT_STYLE_NORMAL,      // Style
            DWRITE_FONT_STRETCH_NORMAL,    // Stretch
            to_screen(40.0f),                         // Size    
            L"en-us",                      // Local
            &g_pTextFormatminiL                 // Pointer to recieve the created object
        );
        hr = g_pDWriteFactory->CreateTextFormat(    //1
            L"Public Sans",
            //L"NSimSun",
            //L"Gabriola",                   // Font family name
            NULL,                          // Font collection(NULL sets it to the system font collection)
            DWRITE_FONT_WEIGHT_REGULAR,    // Weight
            DWRITE_FONT_STYLE_NORMAL,      // Style
            DWRITE_FONT_STRETCH_NORMAL,    // Stretch
            to_screen(50.0f),                         // Size    
            L"en-us",                      // Local
            &g_pTextFormatNormal                 // Pointer to recieve the created object
        );
        
        g_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);           //水平居中
        g_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER); //段落居中
        g_pTextFormatL->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER); //段落居中
        g_pTextFormatmini->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);           //水平居中
        g_pTextFormatmini->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER); //段落居中
        g_pTextFormatminiL ->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER); //段落居中
        g_pTextFormat2->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER); //段落居中
        g_pTextFormat2->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);           //水平居中
        g_pTextFormatNormal->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);           //水平居中
        g_pTextFormatNormal->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER); //段落居中
        g_pTextFormatLarge->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);           //水平居中
        g_pTextFormatLarge->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER); //段落居中
        
        // Initialize Image Factory
        hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, __uuidof(pIWICFactory), (LPVOID*)&pIWICFactory);
        if (FAILED(hr))
        {
            writelog("Create WICImagingFactory failed!");
            normal_quit = 1;
            return;
        }

        g_pD2DDeviceContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        g_pD2DDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);

    }
    writelog("DirectX was built.");
    //MessageBox(hWnd, "CreateD2DResource succeed", "2", 0);
    return;
}


void DrawBitmap_1(ID2D1Bitmap* ppBitmap, D2D_RECT_F dest,  float opacity)
{
    HRESULT hr = S_OK;
    D2D_RECT_F NDEST;
    D2D1_SIZE_F size = ppBitmap->GetSize();
    UINT originalWidth, originalHeight, destinationHeight = dest.bottom - dest.top, destinationWidth = dest.right - dest.left;
    originalWidth = size.width;
    originalHeight = size.height;
    //D2D1_POINT_2F upperLeftCorner = D2D1::Point2F(0.f, 0.f);
    if (destinationHeight != 0 || destinationWidth != 0)    //等比例缩放
    {

        if (SUCCEEDED(hr))
        {
            if (destinationWidth == 0)
            {
                FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(originalHeight);
                destinationWidth = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
            }
            else if (destinationHeight == 0)
            {
                FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(originalWidth);
                destinationHeight = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
            }


        }
    }
    else
    {
        destinationWidth = originalWidth;
        destinationHeight = originalHeight;
    }
    dest.bottom = dest.top + destinationHeight;
    dest.right = dest.left + destinationWidth;
    //dest.top = to_screen(dest.top);
    //dest.bottom = to_screen(dest.bottom);
    //dest.right = to_screen(dest.right);
    //dest.left = to_screen(dest.left);
    // Draw bitmap
    NDEST.top = to_screen(dest.top);
    NDEST.bottom = to_screen(dest.bottom);
    NDEST.right = to_screen(dest.right);
    NDEST.left = to_screen(dest.left);
    g_pD2DDeviceContext->DrawBitmap(
        ppBitmap,
        NDEST,
        opacity,
        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
    );
    DrawCallNum++;

}

void DrawBitmap_1(ID2D1Bitmap* ppBitmap, D2D_RECT_F dest, D2D_RECT_F src,float opacity)
{
    HRESULT hr = S_OK;
    D2D_RECT_F NDEST;
    D2D1_SIZE_F size = ppBitmap->GetSize();
    UINT originalWidth, originalHeight, destinationHeight = dest.bottom - dest.top, destinationWidth = dest.right - dest.left;
    originalWidth = size.width;
    originalHeight = size.height;
    //D2D1_POINT_2F upperLeftCorner = D2D1::Point2F(0.f, 0.f);
    if (destinationHeight != 0 || destinationWidth != 0)    //等比例缩放
    {

        if (SUCCEEDED(hr))
        {
            if (destinationWidth == 0)
            {
                FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(originalHeight);
                destinationWidth = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
            }
            else if (destinationHeight == 0)
            {
                FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(originalWidth);
                destinationHeight = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
            }


        }
    }
    else
    {
        destinationWidth = originalWidth;
        destinationHeight = originalHeight;
    }

    dest.bottom = dest.top + destinationHeight;
    dest.right = dest.left + destinationWidth;
    NDEST.top = to_screen(dest.top);
    NDEST.bottom = to_screen(dest.bottom);
    NDEST.right = to_screen(dest.right);
    NDEST.left = to_screen(dest.left);
    // Draw bitmap

    g_pD2DDeviceContext->DrawBitmap(
        ppBitmap,
        NDEST,
        opacity,
        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
        src
    );
    DrawCallNum++;

}

struct animation
{
    short type = 0;
    short curnum = 0;
    short num = 0;
    bool active = 0;
    short destl = 0;
    short destw = 0;
    short length = 0;
    short width = 0;
    short destx = 0;
    short desty = 0;
    float stime = 0.16;
    float ctime = 0;
}anime[128];

void cleanAnimation()
{
    for (int i = 0; i < 128; i++)
    {
        anime[i].active = 0;
    }
    return;
}
float cubeMap_width = 0;
void CreateAnimation(short type, short destx, short desty)
{
    int k = 0;
    for (int i = 0; i < 128; i++)
    {
        if (!anime[i].active)
        {
            k = i;
            break;
        }
    }
    switch (type)
    {
    case 1: //1-1 
    case 2:
    case 3:
    case 4:
        anime[k].ctime = 0;
        anime[k].stime = 0.1;
        anime[k].length = 512;
        anime[k].width = 512;
        anime[k].num = 11;
        anime[k].destl = 400;
        anime[k].destw = 400;
        break;
    case 5:
        anime[k].ctime = 0;
        anime[k].stime = 0.016;
        anime[k].length = 270;
        anime[k].width = 270;
        anime[k].num = 84;
        anime[k].destl = cubeMap_width;
        anime[k].destw = cubeMap_width;
        break;
    case 6:
        anime[k].ctime = 0;
        anime[k].stime = 0.022;
        anime[k].length = 270;
        anime[k].width = 270;
        anime[k].num = 68;
        anime[k].destl = cubeMap_width;
        anime[k].destw = cubeMap_width;
        break;
    case 7:
        anime[k].ctime = 0;
        anime[k].stime = 0.023;
        anime[k].length = 270;
        anime[k].width = 270;
        anime[k].num = 64;
        anime[k].destl = cubeMap_width;
        anime[k].destw = cubeMap_width;
        break;
    case 8:
        anime[k].ctime = 0;
        anime[k].stime = 0.02;
        anime[k].length = 270;
        anime[k].width = 270;
        anime[k].num = 74;
        anime[k].destl = cubeMap_width;
        anime[k].destw = cubeMap_width;
        break;
    default:
        break;
    }
    anime[k].type = type;
    
    anime[k].destx = destx;
    anime[k].desty = desty;
    anime[k].curnum = 1;//start from 1
    anime[k].active = 1;

    return;
}
void DrawSpriteSheet()
{
    
    ID2D1Bitmap* ppBitmap;
    D2D_RECT_F src,dest;
    int num, x, y, slength, swidth, dlength, dwidth,t=0;
    for (int i = 0; i < 128; i++)
    {
        if (!anime[i].active)
        {
            continue;
        }
        /*if (anime[i].ctime < -0.1)
        {
            continue;
        }*/
        if (anime[i].destl == 0)
        {
            dlength = 1600;
            dwidth = 900;
        }
        x = anime[i].destx;
        y = anime[i].desty;
        dlength = anime[i].destl;
        dwidth = anime[i].destw;
        slength = anime[i].length;
        swidth = anime[i].width;
        num = anime[i].curnum;
        switch (anime[i].type)
        {
        case 1:
            ppBitmap = g_pD2DBimtapUI[22];
            break;
        case 2:
            ppBitmap = g_pD2DBimtapUI[23];
            break;
        case 3:
            ppBitmap = g_pD2DBimtapUI[24];
            break;
        case 4:
            ppBitmap = g_pD2DBimtapUI[25];
            break;
        case 5:
            ppBitmap = g_pD2DBimtapUI[26];
            break;
        case 6:
            ppBitmap = g_pD2DBimtapUI[27];
            break;
        case 7:
            ppBitmap = g_pD2DBimtapUI[28];
            break;
        case 8:
            ppBitmap = g_pD2DBimtapUI[29];
            break;
        default:
            ppBitmap = g_pD2DBimtapUI[25];
            return;
            
        }
        //src = D2D1::RectF(((anime[i].curnum - 1) % 10) * anime[i].length, ((anime[i].curnum - 1) / 10) * anime[i].width, ((anime[i].curnum - 1) % 10) * anime[i].length + anime[i].length, ((anime[i].curnum - 1) / 10) * anime[i].width + anime[i].width);
        src = D2D1::RectF(((num - 1) % 10) * slength, (num - 1) / 10 * swidth, ((num - 1) % 10) * slength+ slength, (num - 1) / 10 * swidth+ swidth);
        dest = D2D1::RectF((float)x, (float)y, (float)(x + dlength), (float)(y + dwidth));
        DrawBitmap_1(ppBitmap, dest, src, 1.0);
        anime[i].ctime += dursec;
        if (anime[i].ctime > anime[i].stime)
        {
            t = anime[i].ctime / anime[i].stime;
            anime[i].curnum += t;

            anime[i].ctime -= t * anime[i].stime;
            if (anime[i].curnum > anime[i].num)
            {
                if (anime[i].type < 5)
                {
                    anime[i].curnum = 1;
                }
                else
                {
                    anime[i].active = 0;
                }
                
            }
        }
        
        
    }
    
    
    //DrawCallNum++;
    return;
}




wchar_t* multiByteToWideChar(const string& pKey)
{
    const char* pCStrKey = pKey.c_str();
    //第一次调用返回转换后的字符串长度，用于确认为wchar_t*开辟多大的内存空间
    int pSize = MultiByteToWideChar(CP_OEMCP, 0, pCStrKey, strlen(pCStrKey) + 1, NULL, 0);
    wchar_t* pWCStrKey = new wchar_t[pSize];
    //第二次调用将单字节字符串转换成双字节字符串
    MultiByteToWideChar(CP_OEMCP, 0, pCStrKey, strlen(pCStrKey) + 1, pWCStrKey, pSize);
    return pWCStrKey;
}





void update()
{
    
    CheckBoxes();
    //update button
    for (int i = 0; i < 128; i++)
    {
        if (!buttons[i].active)
        {
            continue;
        }

        if (boxes[buttons[i].box_index].click_status)
        {
            buttons[i].Brush2 = g_pBrushGray;
        }
        else if (boxes[buttons[i].box_index].focus)
        {
            buttons[i].Brush2 = g_pBrushLightGray;
        }
        else
        {
            buttons[i].Brush2 = buttons[i].Brush2_ori;
        }
    }

    return;
}

void rendButton()
{
    for (int i = 0; i < 128; i++)
    {
        if (!buttons[i].active)
        {
            continue;
        }
        float pic_opacity = 0.8F;
        float x1 = to_screen(buttons[i].x1),x2= to_screen(buttons[i].x2),y1= to_screen(buttons[i].y1),y2= to_screen(buttons[i].y2);
        float opacity_buf = 0;
        if (buttons[i].Brush2 != NULL)
        {
            opacity_buf = buttons[i].Brush2->GetOpacity();
            buttons[i].Brush2->SetOpacity(0.65f);
            g_pD2DDeviceContext->FillRoundedRectangle(
                D2D1::RoundedRect(
                    D2D1::RectF(x1, y1, x2, y2),
                    15.0f,
                    25.0f),
                buttons[i].Brush2
            );
            buttons[i].Brush2->SetOpacity(opacity_buf);
            DrawCallNum++;
        }
        
        if (buttons[i].Bitmap != NULL)
        {
            //opacity_buf = buttons[i].Brush3->GetOpacity();
            /*if (boxes[buttons[i].box_index].focus)
            {
                pic_opacity = 0.2;
                buttons[i].Brush3->SetOpacity(0.9F);

            }
            else
            {
                buttons[i].Brush3->SetOpacity(0.3F);

            }*/
            DrawBitmap_1(
                    buttons[i].Bitmap,
                    D2D1::RectF((buttons[i].x1+ buttons[i].x2+ buttons[i].y1- buttons[i].y2)/2, buttons[i].y1, (buttons[i].x1 + buttons[i].x2 - buttons[i].y1 + buttons[i].y2) / 2, buttons[i].y2),
                pic_opacity
                
            );
            //buttons[i].Brush3->SetOpacity(opacity_buf);
            DrawCallNum++;
        }
        
        if (buttons[i].Brush1 != NULL)
        {
            //buttons[i].Brush3->SetOpacity(1.0F);
            g_pD2DDeviceContext->DrawRoundedRectangle(
                D2D1::RoundedRect(
                    D2D1::RectF(x1, y1, x2, y2),
                    15.0f,
                    25.0f),
                buttons[i].Brush1,
                to_screen(3.0f)
            );
            DrawCallNum++;
        }
        
        if (buttons[i].text != ""&& buttons[i].Brush3 != NULL)
        {
            g_pD2DDeviceContext->DrawTextA(
                multiByteToWideChar(buttons[i].text),           // Text to render  
                wcslen(multiByteToWideChar(buttons[i].text)),       // Text length
                g_pTextFormat,     // Text format
                D2D1::RectF(
                    static_cast<FLOAT>(to_screen(buttons[i].x1)),
                    static_cast<FLOAT>(to_screen(buttons[i].y1)),
                    static_cast<FLOAT>(to_screen(buttons[i].x2)),
                    static_cast<FLOAT>(to_screen(buttons[i].y2))
                ),    // The region of the window where the text will be rendered
                buttons[i].Brush3      // The brush used to draw the text
            );
            DrawCallNum++;
        }
        
        
    }
}
void rendLable()
{
    for (int i = 0; i < 128; i++)
    {
        if (!lables[i].active)
        {
            continue;
        }
        //float pic_opacity = 0.8F;
        float x1 = to_screen(lables[i].x1), x2 = to_screen(lables[i].x2), y1 = to_screen(lables[i].y1), y2 = to_screen(lables[i].y2);

        
        if (lables[i].Brush2 != NULL)
        {
            if (page_index == 5)
            {
                lables[i].Brush2->SetOpacity(0.4);
            }
            else
            {
                lables[i].Brush2->SetOpacity(0.6);
            }
            
            g_pD2DDeviceContext->FillRoundedRectangle(
                D2D1::RoundedRect(
                    D2D1::RectF(x1, y1, x2, y2),
                    15.0f,
                    25.0f),
                lables[i].Brush2
            );
            lables[i].Brush2->SetOpacity(1.0);
            DrawCallNum++;
        }
        
        if (lables[i].Bitmap != NULL)
        {
            //if (boxes[lables[i].box_index].focus)
            //{
            //    //pic_opacity = 0.2;
            //    lables[i].Brush3->SetOpacity(0.9F);

            //}
            //else
            //{
            //    lables[i].Brush3->SetOpacity(0.3F);

            //}
            lables[i].Brush3->SetOpacity(0.4F);
            DrawBitmap_1(
                lables[i].Bitmap,
                D2D1::RectF((lables[i].x1 + lables[i].x2 + lables[i].y1 - lables[i].y2) / 2, lables[i].y1, (lables[i].x1 + lables[i].x2 - lables[i].y1 + lables[i].y2) / 2, lables[i].y2),
                0.8f

            );
            
            DrawCallNum++;
        }
        /*else
        {
            lables[i].Brush3->SetOpacity(1.0F);
        }*/
        if (lables[i].Brush1 != NULL)
        {
            g_pD2DDeviceContext->DrawRoundedRectangle(
                D2D1::RoundedRect(
                    D2D1::RectF(x1, y1, x2, y2),
                    15.0f,
                    25.0f),
                lables[i].Brush1,
                to_screen(3.0f)
            );
            DrawCallNum++;
        }
        IDWriteTextFormat* g_pTextFormat_T = g_pTextFormat;
        if (lables[i].TextFormat == 0)
        {
            g_pTextFormat_T = g_pTextFormatL;
        }
        else if (lables[i].TextFormat == 1)
        {
            g_pTextFormat_T = g_pTextFormatminiL;
        }
        else if (lables[i].TextFormat == 2)
        {
            g_pTextFormat_T = g_pTextFormat2;
        }
        else if (lables[i].TextFormat == 3)
        {
            g_pTextFormat_T = g_pTextFormatNormal;
        }
        else if (lables[i].TextFormat == 4)
        {
            g_pTextFormat_T = g_pTextFormatmini;
        }
        /*else if (lables[i].TextFormat == 5)
        {
            g_pTextFormat_T = g_pTextFormatL;
            g_pBitmapBrushUI
        }*/
        if (lables[i].text != ""&& lables[i].Brush3 != NULL)
        {
            if (lables[i].TextFormat == 5)
            {
                g_pTextFormat_T = g_pTextFormat;
                g_pD2DDeviceContext->DrawTextA(
                    multiByteToWideChar(lables[i].text),           // Text to render  
                    wcslen(multiByteToWideChar(lables[i].text)),       // Text length
                    g_pTextFormat_T,     // Text format
                    D2D1::RectF(
                        x1 * 0.9 + x2 * 0.1,
                        y1,
                        x2,
                        y2
                    ),    // The region of the window where the text will be rendered
                    g_pRadialGradientBrush//g_pBitmapBrushUI[12]      // The brush used to draw the text
                );
                g_pD2DDeviceContext->DrawTextA(
                    multiByteToWideChar(lables[i].text),           // Text to render  
                    wcslen(multiByteToWideChar(lables[i].text)),       // Text length
                    g_pTextFormat_T,     // Text format
                    D2D1::RectF(
                        x1 * 0.9 + x2 * 0.1,
                        y1,
                        x2,
                        y2
                    ),    // The region of the window where the text will be rendered
                    g_pBrushDark      // The brush used to draw the text
                );
                DrawCallNum+=2;
            }
            else
            {
                g_pD2DDeviceContext->DrawTextA(
                    multiByteToWideChar(lables[i].text),           // Text to render  
                    wcslen(multiByteToWideChar(lables[i].text)),       // Text length
                    g_pTextFormat_T,     // Text format
                    D2D1::RectF(
                        x1 * 0.9 + x2 * 0.1,
                        y1,
                        x2,
                        y2
                    ),    // The region of the window where the text will be rendered
                    lables[i].Brush3      // The brush used to draw the text
                );
            }
            
            lables[i].Brush3->SetOpacity(1.0F);
            DrawCallNum++;
        }
        if (lables[i].Brush3 != NULL)
        {
            lables[i].Brush3->SetOpacity(1.0F);
        }
        
    }
    return;
}



//扫雷逻辑部分
#define centerx 880
#define centery 400
//int cubes[5][5] = { 0 };    //取值0-11,模拟4*4网格
struct cube_map
{
    int generate_value = 0;
    int status = 0; //1-未开启 2-已开启 3-被标记为雷 4-被聚焦
    bool isMine = 0;
    int value = 0;
    int gui_pointer = 0;
    float posx = 0, posy = 0;   //中点坐标
};
cube_map cubes[32][32];
struct gui_cubes
{
    bool active = 0;
    bool isMine = 0;
    int value = 0;
    int status = 0; //1-未开启 2-已开启 3-被标记为雷 4-被聚焦
    int act = 0;    //1-  2-  3-generating animation
    int theme = 0;
    int x = 0, y = 0;
    float posx = 0, posy = 0;
    bool marked = 0;
    
    float opacity = 1;
    //float zoom = 1;
    bool opening = 0;
    float open_time = 0;
    int mine_type = 0;
    int box_index = 0;
    
};
gui_cubes cubes_g[32][32];    //渲染相关参数








int last_page_index = 0;
int saveDeleted = 0,recorded = 0;

int  cubenum_w = 0, cubenum_h = 0, mine_totle_num = 0, opened_cubenum = 0;
//int last_step_buf[6][6] = { 0 };

//void RefreshWaitLine();
//void generate_cube();
//int generate_gui_cube();
//void reflex();
void game_main();
void claenStage();
//void step(int direction);
//bool cubes_g_update(int direction);
//void rendSingleCube(int index);
//void rendCube();
void rendMap();
void initMS();
//void save2048(int index);
void saveMS(int index);
void loadMS();
//void page_game_read();
string getTimeDigit(float time);
bool record_once();
void page_game_load();
void rend_page_history();
void rend_page_highscore();


void newMap();


void initMS()
{
    opened_cubenum = 0;
    if (set1[0].map_size == 1)//小 9*9-10
    {
        cubenum_w = 9;
        cubenum_h = 9;
        mine_totle_num = 10;
        cubeMap_width = 80;

    }
    else if (set1[0].map_size == 2)//中 16*16-40
    {
        cubenum_w = 16;
        cubenum_h = 16;
        mine_totle_num = 40;
        cubeMap_width = 50;
    }
    else if (set1[0].map_size == 3)//大 30*16-99
    {
        cubenum_w = 30;
        cubenum_h = 16;
        mine_totle_num = 99;
        cubeMap_width = 45;
    }
    /*string dbgbufx = "", dbgbufy="";*/
    for (int i = 1; i <= cubenum_w; i++)
    {
        /*dbgbufx = "", dbgbufy = "";*/
        for (int j = 1; j <= cubenum_h; j++)
        {
            cubes_g[i][j].posx = centerx + (((float)i - (float)cubenum_w / 2) - 0.5f) * cubeMap_width;
            cubes_g[i][j].posy = centery + (((float)j - (float)cubenum_h / 2) - 0.5f) * cubeMap_width;
            /*dbgbufx += to_string(cubes_g[i][j].posx);
            dbgbufx += " ";
            dbgbufy += to_string(cubes_g[i][j].posy);
            dbgbufy += " ";*/
            cubes_g[i][j].status = 1;
            cubes_g[i][j].opacity = 1;
            cubes_g[i][j].x = i;
            cubes_g[i][j].y = j;
            cubes_g[i][j].open_time = 0;
            cubes_g[i][j].opening = 0;
        }
        /*writelog("x: " + dbgbufx);
        writelog("y: " + dbgbufy);*/
    }


    for (int i = 0; i <= cubenum_w + 1; i++)
    {
        for (int j = 0; j <= cubenum_h + 1; j++)
        {
            cubes[i][j].isMine = 0;
            cubes_g[i][j].isMine = 0;
            cubes_g[i][j].marked = 0;
            if (boxes[cubes_g[i][j].box_index].active && cubes_g[i][j].box_index>4/*需调整*/)
            {
                boxes[cubes_g[i][j].box_index].active = 0;
                boxes[cubes_g[i][j].box_index].clicked = 0;
                boxes[cubes_g[i][j].box_index].click_status = 0;
                boxes[cubes_g[i][j].box_index].rightclick_status = 0;
                boxes[cubes_g[i][j].box_index].right_clicked = 0;
            }
            
        }
    }
    return;
}

void newMap()   //生成地图
{
    int totle_value = 0, rand_temp = 0,created_mine_num=0;
    bool break_single = 0;
    currentSave = 0;
    initMS();
    //for (int i = 1/*需更换*/; i < 1024; i++)
    //{
    //    boxes[i].active = 0;
    //    boxes[i].clicked = 0;
    //    boxes[i].right_clicked = 0;
    //}
    
    while (created_mine_num < mine_totle_num)
    {
        for (int i = 1; i <= cubenum_w; i++)//初始化权重
        {
            for (int j = 1; j <= cubenum_h; j++)
            {
                cubes[i][j].generate_value = 2;
            }
        }
        for (int i = 1; i <= cubenum_w; i++)//刷新权重
        {
            for (int j = 1; j <= cubenum_h; j++)
            {
                if (cubes[i][j].isMine)
                {
                    cubes[i - 1][j-1].generate_value++;
                    cubes[i - 1][j].generate_value++;
                    cubes[i - 1][j+1].generate_value++;
                    cubes[i + 1][j].generate_value++;
                    cubes[i+1][j + 1].generate_value++;
                    cubes[i+1][j - 1].generate_value++;
                    cubes[i][j+1].generate_value++;
                    cubes[i][j-1].generate_value++;
                }
                
            }
        }
        totle_value = 0;
        break_single = 0;
        for (int i = 1; i <= cubenum_w; i++)//收集权重
        {
            for (int j = 1; j <= cubenum_h; j++)
            {
                if (!cubes[i][j].isMine)
                {
                    totle_value += cubes[i][j].generate_value;
                }
                
            }
        }

        rand_temp = rand() % totle_value;
        for (int i = 1; i <= cubenum_w; i++)    //添置地雷
        {
            for (int j = 1; j <= cubenum_h; j++)
            {
                /*if (i == cubenum_w && j == cubenum_h)
                {
                    writelog("last cube.");
                }*/
                if (cubes[i][j].isMine)
                {
                    continue;
                }
                rand_temp -= cubes[i][j].generate_value;
                if (rand_temp <= 0)
                {
                    cubes[i][j].isMine = 1;
                    cubes_g[i][j].isMine = 1;
                    cubes_g[i][j].mine_type = rand() % 3;
                    created_mine_num++;
                    break_single = 1;
                    break;
                }
                
            }
            if (break_single)
            {
                break;
            }
        }
    }
    for (int i = 1; i <= cubenum_w; i++)
    {
        for (int j = 1; j <= cubenum_h; j++)
        {
            cubes[i][j].value=0;
            cubes_g[i][j].value = 0;
            
        }
    }
    for (int i = 1; i <= cubenum_w; i++)//刷新空块的数字
    {
        for (int j = 1; j <= cubenum_h; j++)
        {
            if (cubes[i][j].isMine)
            {
                cubes_g[i - 1][j - 1].value++;
                cubes_g[i - 1][j].value++;
                cubes_g[i - 1][j + 1].value++;
                cubes_g[i + 1][j].value++;
                cubes_g[i + 1][j + 1].value++;
                cubes_g[i + 1][j - 1].value++;
                cubes_g[i][j + 1].value++;
                cubes_g[i][j - 1].value++;
            }
            cubes_g[i][j].box_index = SetBox(cubes_g[i][j].posx - cubeMap_width / 2, cubes_g[i][j].posx + cubeMap_width / 2, cubes_g[i][j].posy - cubeMap_width / 2, cubes_g[i][j].posy + cubeMap_width / 2);
        }
    }
    /*string dbgbuf = "";
    for (int i = 1; i <= cubenum_w; i++)
    {
        for (int j = 1; j <= cubenum_h; j++)
        {
            dbgbuf+=to_string()
        }
    }*/
    writelog("newmap finished.");
    return;
}


void saveMS(int index)
{
    if (thread_IO_request_save_gameMS)
    {
        writelog("request_save busy.");
        return;
    }
    int counter = 0;
    
    timebuf = time_used;
    flagbuf = mine_totle_num - mine_remain;
    opened_cubenum_buf = opened_cubenum;
    for (int i = 1; i <= cubenum_w; i++)
    {
        for (int j = 1; j <= cubenum_h; j++)
        {
            if (cubes_g[i][j].isMine)
            {
                savebuf[counter] = 1;
            }
            else
            {
                savebuf[counter] = 0;
            }
            

            counter++;
        }
    }
    thread_IO_request_save_gameMS = index;
    return;
}
//void autosave()
//{
//    save2048(currentSave);
//    return;
//}
void loadMS()
{
    
    if (thread_IO_request_load_gameMS)
    {
        writelog("request_load busy.");
        return;
    }
    int counter = 0;

    mine_remain = mine_totle_num;
    opened_cubenum = 0;
    time_used = 0;
    
    int count = 0;
    //read savebuf->cubes_g

    for (int i = 1; i <= cubenum_w; i++)
    {
        for (int j = 1; j <= cubenum_h; j++)
        {
            cubes[i][j].value = 0;
            cubes_g[i][j].value = 0;
            /*cubes_g[i][j].marked = 0;
            cubes_g[i][j].status = 1;
            cubes_g[i][j].opening = 0;*/
            if (savebuf[count] == 1)
            {
                counter++;
            }
            cubes_g[i][j].isMine = savebuf[count];
            count++;
        }

    }
    //writelog("loaded mine num from savebuf " + to_string(counter));
    count = 0;
    for (int i = 1; i <= cubenum_w; i++)//刷新空块的数字
    {
        for (int j = 1; j <= cubenum_h; j++)
        {
            if (cubes_g[i][j].isMine)
            {
                count++;
                cubes_g[i - 1][j - 1].value++;
                cubes_g[i - 1][j].value++;
                cubes_g[i - 1][j + 1].value++;
                cubes_g[i + 1][j].value++;
                cubes_g[i + 1][j + 1].value++;
                cubes_g[i + 1][j - 1].value++;
                cubes_g[i][j + 1].value++;
                cubes_g[i][j - 1].value++;
            }
            //cubes_g[i][j].box_index = SetBox(cubes_g[i][j].posx - cubeMap_width / 2, cubes_g[i][j].posx + cubeMap_width / 2, cubes_g[i][j].posy - cubeMap_width / 2, cubes_g[i][j].posy + cubeMap_width / 2);
        }
    }
    //刷新地图value
    writelog("game loaded.");
    return;

}







void claenStage()
{
    
    for (int i = 0; i <= cubenum_w; i++)
    {
        for (int j = 0; j <= cubenum_h; j++)
        {
            cubes[i][j].value = 0;
        }
    }
    //score = 0, score_bar = 0, timeForScore = 0, timeForScore_required = 0, step_used = 0; 
    if (set2[0].visual_effect)
    {
        createEffect(1, 0, 0);
        
    }
    time_used = 0;
    return;
}





void game_write_single_save()
{
    string writebuf[64] = { "" }, readbuf[64] = { "" };
    string pathname = "./save/", fullfilename = "";
    pathname += usernameC;
    int flag = 0, pointer = 0,lines=0;
    
    
    
    

    fullfilename = pathname;
    fullfilename += "/save";
    fullfilename += to_string(thread_IO_request_save_gameMS);
    fullfilename += ".dat";
    flag = _access(fullfilename.c_str(), 6);
    if (flag != 0)
    {
        writelog("create new save.");
        writelog(fullfilename.c_str());

    }


    writebuf[0] = "[user]";
    writebuf[1] = usernameC;
    writebuf[2] = "[index]";
    writebuf[3] = to_string(thread_IO_request_save_gameMS);
    writebuf[4] = "[mapsize]";
    writebuf[5] = to_string(set1[0].map_size);
    writebuf[6] = "[flag]";
    writebuf[7] = to_string(flagbuf);
    writebuf[8] = "[opened_cube]";
    writebuf[9] = to_string(opened_cubenum_buf);
    writebuf[10] = "[time]";
    writebuf[11] = to_string(timebuf);
    writebuf[12] = "[gamedata]";
    
    
    for (int i = 0; i < cubenum_h*cubenum_w; i++)
    {
            writebuf[13] += to_string(savebuf[i]);
    }
    
    
    WriteFile(fullfilename.c_str(), writebuf);
    
    certfile(fullfilename);
    currentSave = thread_IO_request_save_gameMS;
    writelog("save succeed.");
    thread_IO_request_save_gameMS = 0;
    return;
}
void game_read_single_save()
{
    string readbuf[64] = { "" };
    string pathname = "./save/", fullfilename = "";
    pathname += usernameC;
    int flag = 0, lines = 0, k = 0, m = 0;
    bool check1 = 0;
    
    k = thread_IO_request_load_gameMS - 1;
    fullfilename = pathname;
    fullfilename += "/save";
    fullfilename += to_string(thread_IO_request_load_gameMS);
    fullfilename += ".dat";
    
    if (!file_private_verify(fullfilename))
    {
        writelog("cert file failed!  "+ fullfilename);
        
        thread_IO_request_load_gameMS = -1;
        playeffectsound(6);
        return;
    }



    
       

        lines = ReadFile(fullfilename.c_str(), readbuf);
        if (lines > 0)
        {
            for (int j = 0; j < 64; j++)
            {
                if (readbuf[j] == "[user]")
                {
                    if (readbuf[j + 1] != usernameC)
                    {
                        playeffectsound(6);
                        thread_IO_request_load_gameMS = -1;
                        writelog("save cert failed!(username)    "+ fullfilename);
                        
                        return;
                    }
                }
                else if (readbuf[j] == "[index]")
                {
                    if (readbuf[j + 1] != to_string(thread_IO_request_load_gameMS))
                    {
                        playeffectsound(6);
                        thread_IO_request_load_gameMS = -1;
                        writelog("save cert failed!(index)    "+ fullfilename);
                        
                        return;
                    }
                }
                else if (readbuf[j] == "[mapsize]")
                {
                    //read_infos[k].score = readbuf[j + 1];
                    set1[0].map_size = stoi(readbuf[j + 1]);
                    initMS();
                }
                else if (readbuf[j] == "[flag]")
                {
                    //read_infos[k].step = readbuf[j + 1];
                    flagbuf = stoi(readbuf[j + 1]);
                }
                else if (readbuf[j] == "[opened_cube]")
                {
                    opened_cubenum_buf= stoi(readbuf[j + 1]);
                }
                else if (readbuf[j] == "[time]")
                {
                    
                    timebuf = stof(readbuf[j + 1]);
                }
                else if (readbuf[j] == "[gamedata]")
                {

                    
                    for (int i = 0; i < cubenum_h * cubenum_w; i++)
                    {
                        if (stoi(readbuf[j + 1].substr(i, 1)) == 1)
                        {
                            m++;
                        }
                        savebuf[i] = stoi(readbuf[j+1].substr(i,1));
                    }
                    //writelog("loaded mine num to savebuf "+to_string(m));
                }
                /*else if (readbuf[j] == "[waitline]")
                {
                    m = 1;
                    for (int i = 0; i < 8; i++)
                    {
                        if (!AllisNum(readbuf[j + m]))
                        {
                            writelog("corrupted file!");
                            writelog(fullfilename.c_str());
                            playeffectsound(6);
                            thread_IO_request_load_game2048 = -1;
                            return;
                        }
                        waitlinebuf[i] = stoi(readbuf[j + m]);
                        m++;
                    }
                    wl_pointerbuf = stoi(readbuf[j + m]);
                }*/

            }
        }
        else
        {
            writelog("corrupted file!");
            writelog(fullfilename.c_str());
            playeffectsound(6);
            thread_IO_request_load_gameMS = -1;
            return;
        }
    

    currentSave = thread_IO_request_load_gameMS;
    thread_IO_request_load_gameMS = 0;
    return;
}



//渲染背景

void rendMap()
{
    ID2D1SolidColorBrush* g_pBrushTextbuf = NULL;
    float x1, x2, y1, y2;
    ID2D1Bitmap* ppBitmap = NULL;
    g_pLinearGradientBrush->SetTransform(D2D1::Matrix3x2F::Translation(D2D1::SizeF(to_screen(centerx - (float)cubenum_w / 2 * cubeMap_width-500), to_screen(centery - (float)cubenum_h / 2 * cubeMap_width-200))));
    g_pD2DDeviceContext->FillRectangle(

        D2D1::RectF(to_screen(centerx -(float)cubenum_w/2*cubeMap_width), to_screen(centery - (float)cubenum_h / 2 * cubeMap_width), to_screen(centerx + (float)cubenum_w / 2 * cubeMap_width), to_screen(centery + (float)cubenum_h / 2 * cubeMap_width)),
        g_pBrushLightBlue
    );
    g_pBrushLight->SetOpacity(0.7);
    g_pD2DDeviceContext->FillRectangle(

        D2D1::RectF(to_screen(centerx - (float)cubenum_w / 2 * cubeMap_width), to_screen(centery - (float)cubenum_h / 2 * cubeMap_width), to_screen(centerx + (float)cubenum_w / 2 * cubeMap_width), to_screen(centery + (float)cubenum_h / 2 * cubeMap_width)),
        g_pBrushLight
    );
    g_pBrushLight->SetOpacity(0.5);
    DrawCallNum += 2;
    for (int i = 1; i <= cubenum_w; i++)
    {
        for (int j = 1; j <= cubenum_h; j++)    //rendSingleCube
        {
            x1 = cubes_g[i][j].posx - cubeMap_width / 2;
            x2 = cubes_g[i][j].posx + cubeMap_width / 2;
            y1 = cubes_g[i][j].posy - cubeMap_width / 2;
            y2 = cubes_g[i][j].posy + cubeMap_width / 2;
            if (cubes_g[i][j].status == 1)
            {
                
                g_pRadialGradientBrush->SetTransform(D2D1::Matrix3x2F::Translation(D2D1::SizeF(to_screen(x1 + cubeMap_width / 15), to_screen(y1 + cubeMap_width / 15))));
                g_pRadialGradientBrush->SetRadiusX(to_screen(cubeMap_width));
                g_pRadialGradientBrush->SetRadiusY(to_screen(cubeMap_width));
                //g_pRadialGradientBrush->SetCenter(D2D1::Point2F(cubes_g[i][j].posx * scale, cubes_g[i][j].posy * scale));

                g_pD2DDeviceContext->FillRoundedRectangle(
                    D2D1::RoundedRect(
                        D2D1::RectF(to_screen(x1 + cubeMap_width/15), to_screen(y1 + cubeMap_width / 15), to_screen(x2 - cubeMap_width / 15), to_screen(y2 - cubeMap_width / 15)),
                    to_screen(cubeMap_width/8),
                    to_screen(cubeMap_width / 8)),
                    g_pRadialGradientBrush

                );
                g_pBrushLight->SetOpacity(0.3);
                g_pD2DDeviceContext->FillRoundedRectangle(
                    D2D1::RoundedRect(
                        D2D1::RectF(to_screen(x1 + cubeMap_width / 8), to_screen(y1 + cubeMap_width / 8), to_screen(x2 - cubeMap_width / 8), to_screen(y2 - cubeMap_width / 8)),
                        to_screen(cubeMap_width / 8),
                        to_screen(cubeMap_width / 8)),
                    g_pBrushLight

                );
                g_pD2DDeviceContext->FillRectangle(

                    D2D1::RectF(to_screen(x1 ), to_screen(y1 ), to_screen(x2 ), to_screen(y2 )),
                    g_pLinearGradientBrush
                );
                if (cubes_g[i][j].marked)
                {
                    /*g_pD2DDeviceContext->FillEllipse(
                        D2D1::Ellipse(D2D1::Point2F(to_screen((x1 + x2) / 2), to_screen((y1 + y2) / 2)), to_screen(10), to_screen(10)),
                        g_pBrushRed);*/
                    DrawCallNum++;
                    DrawBitmap_1(g_pD2DBimtapUI[21],
                        D2D1::RectF(x1, y1 ,x2 , y2 ),
                        0.8);
                }
                g_pBrushLight->SetOpacity(1 - cubes_g[i][j].opacity);
                g_pD2DDeviceContext->FillRectangle(

                    D2D1::RectF(to_screen(x1 + 3), to_screen(y1 + 3), to_screen(x2 - 3), to_screen(y2 - 3)),
                    g_pBrushLight
                );
                g_pBrushLight->SetOpacity(0.5);
                g_pD2DDeviceContext->DrawRectangle(

                    D2D1::RectF(to_screen(x1), to_screen(y1), to_screen(x2), to_screen(y2)),
                    g_pBrushBlack,
                    to_screen(2.0f)
                );
                DrawCallNum += 5;
            }
            else if (cubes_g[i][j].status == 2)
            {
                if (cubes_g[i][j].isMine)
                {
                    if (cubes_g[i][j].mine_type < 3)
                    {
                        g_pRadialGradientBrush->SetTransform(D2D1::Matrix3x2F::Translation(D2D1::SizeF(to_screen(x1 + cubeMap_width / 15), to_screen(y1 + cubeMap_width / 15))));
                        g_pRadialGradientBrush->SetRadiusX(to_screen(cubeMap_width));
                        g_pRadialGradientBrush->SetRadiusY(to_screen(cubeMap_width));
                        g_pD2DDeviceContext->FillRoundedRectangle(
                            D2D1::RoundedRect(
                                D2D1::RectF(to_screen(x1 + cubeMap_width / 15), to_screen(y1 + cubeMap_width / 15), to_screen(x2 - cubeMap_width / 15), to_screen(y2 - cubeMap_width / 15)),
                                to_screen(cubeMap_width / 8),
                                to_screen(cubeMap_width / 8)),
                            g_pRadialGradientBrush

                        );
                        /*g_pBrushLight->SetOpacity(0.3);
                        g_pD2DDeviceContext->FillRoundedRectangle(
                            D2D1::RoundedRect(
                                D2D1::RectF(to_screen(x1 + cubeMap_width / 8), to_screen(y1 + cubeMap_width / 8), to_screen(x2 - cubeMap_width / 8), to_screen(y2 - cubeMap_width / 8)),
                                to_screen(cubeMap_width / 8),
                                to_screen(cubeMap_width / 8)),
                            g_pBrushLight

                        );*/
                        g_pD2DDeviceContext->FillRectangle(

                            D2D1::RectF(to_screen(x1), to_screen(y1), to_screen(x2), to_screen(y2)),
                            g_pLinearGradientBrush
                        );
                        DrawCallNum += 3;
                    }
                    
                    

                    
                    DrawBitmap_1(g_pD2DBimtapUI[15+ cubes_g[i][j].mine_type],
                        D2D1::RectF(x1, y1, x2, y2),
                        0.8);
                    
                    DrawCallNum++;
                }
                
                    
                
                if (cubes_g[i][j].value != 0&& !cubes_g[i][j].isMine)
                {
                    switch (cubes_g[i][j].value)
                    {
                    case 1:
                        g_pBrushTextbuf = g_pBrushBlue;
                        break;
                    case 2:
                        g_pBrushTextbuf = g_pBrushGreen;
                        break;
                    case 3:
                        g_pBrushTextbuf = g_pBrushRed;
                        break;
                    case 4:
                        g_pBrushTextbuf = g_pBrushLightGreen;
                        break;
                    case 5:
                        g_pBrushTextbuf = g_pBrushYellow;
                        break;
                    case 6:
                        g_pBrushTextbuf = g_pBrushPurple;
                        break;
                    case 7:
                        g_pBrushTextbuf = g_pBrushBrown;
                        break;
                    case 8:
                        g_pBrushTextbuf = g_pBrushGray;
                        break;
                    default:
                        g_pBrushTextbuf = g_pBrushBlack;
                        break;
                    }
                    g_pD2DDeviceContext->DrawText(
                        multiByteToWideChar(to_string(cubes_g[i][j].value)),           // Text to render
                        wcslen(multiByteToWideChar(to_string(cubes_g[i][j].value))),       // Text length
                        g_pTextFormatNormal,     // Text format
                        D2D1::RectF(to_screen(x1 + 3), to_screen(y1 + 3), to_screen(x2 - 3), to_screen(y2 - 3)),    // The region of the window where the text will be rendered
                        g_pBrushTextbuf      // The brush used to draw the text
                    );
                    DrawCallNum++;
                }
                
                g_pD2DDeviceContext->DrawRectangle(

                    D2D1::RectF(to_screen(x1), to_screen(y1), to_screen(x2), to_screen(y2)),
                        g_pBrushBlack,
                    to_screen(2.0f)
                );
                DrawCallNum++;
            }
            
        }
    }
    
    return;
}



string getTimeDigit(float time)
{
    string result = "";
    if (time < 100)
    {
        result = to_string(time).substr(0, 4);
    }
    else if (100 <= time && time < 1000)
    {
        result = to_string(time).substr(0, 5);
    }
    else if (1000 <= time && time < 10000)
    {
        result = to_string(time).substr(0, 4);
    }
    else
    {
        result = to_string(time);
    }
    result += "s";
    return result;
}
void open_cube(int i,int j,int delay)
{
    if (cubes_g[i][j].status == 2|| cubes_g[i][j].opening)
    {
        return;
    }
    
    
    cubes_g[i][j].opening = 1;
    if (cubes_g[i][j].isMine)
    {
        cubes_g[i][j].status = 2;
        if (delay < 3)
        {
            cubes_g[i][j].mine_type += 3;
        }
        //anime
        if (page_status==2)
        {
            CreateAnimation(rand() % 4 + 5, cubes_g[i][j].posx - cubeMap_width / 2, cubes_g[i][j].posy - cubeMap_width / 2);
            page_status = 102;
            playeffectsound(rand() % 7 + 8);
        }
        
        return;
    }
    if (delay == 1)
    {
        cubes_g[i][j].open_time = 0.1;

    }
    else
    {
        cubes_g[i][j].open_time = 0;
        cubes_g[i][j].status = 2;
        opened_cubenum++;
        if (opened_cubenum == cubenum_w * cubenum_h - mine_totle_num)
        {
            lables[0].text = "0";
            page_status = 103;
            playeffectsound(16);
        }
    }
    if (cubes_g[i][j].value == 0&&i>0&&i<=cubenum_w&&j>0&&j<= cubenum_h&&delay!=1)
    {
        playeffectsound(17);
        if (!cubes_g[i][j - 1].opening)
        {
            open_cube(i, j - 1,1);
        }
        if (!cubes_g[i][j + 1].opening)
        {
            open_cube(i, j + 1,1);
        }
        if (!cubes_g[i-1][j - 1].opening)
        {
            open_cube(i - 1, j - 1,1);
        }
        if (!cubes_g[i-1][j ].opening)
        {
            open_cube(i - 1, j,1);
        }
        if (!cubes_g[i-1][j + 1].opening)
        {
            open_cube(i - 1, j + 1,1);
        }
        if (!cubes_g[i+1][j - 1].opening)
        {
            open_cube(i + 1, j - 1,1);
        }
        if (!cubes_g[i + 1][j].opening)
        {
            open_cube(i + 1, j,1);
        }
        if (!cubes_g[i + 1][j + 1].opening)
        {
            open_cube(i + 1, j + 1,1);
        }

    }

}
void doubleClick_openCube(int i,int j)
{
    int marknum = 0;
    if (cubes_g[i][j-1].marked&& cubes_g[i][j - 1].status==1)
    {
        marknum++;
    }
    if (cubes_g[i][j+1].marked && cubes_g[i][j + 1].status == 1)
    {
        marknum++;
    }
    if (cubes_g[i-1][j-1].marked && cubes_g[i-1][j - 1].status == 1)
    {
        marknum++;
    }
    if (cubes_g[i-1][j].marked && cubes_g[i-1][j ].status == 1)
    {
        marknum++;
    }
    if (cubes_g[i-1][j+1].marked && cubes_g[i-1][j + 1].status == 1)
    {
        marknum++;
    }
    if (cubes_g[i+1][j-1].marked && cubes_g[i+1][j - 1].status == 1)
    {
        marknum++;
    }
    if (cubes_g[i+1][j].marked && cubes_g[i+1][j].status == 1)
    {
        marknum++;
    }
    if (cubes_g[i+1][j+1].marked && cubes_g[i+1][j + 1].status == 1)
    {
        marknum++;
    }
    if (marknum == cubes_g[i][j].value)
    {
        if (!cubes_g[i][j - 1].marked && cubes_g[i][j - 1].status == 1)
        {
            open_cube(i, j - 1, 2);
        }
        if (!cubes_g[i][j + 1].marked && cubes_g[i][j + 1].status == 1)
        {
            open_cube(i, j + 1, 2);
        }
        if (!cubes_g[i - 1][j - 1].marked && cubes_g[i - 1][j - 1].status == 1)
        {
            open_cube(i - 1, j - 1, 2);
        }
        if (!cubes_g[i - 1][j].marked && cubes_g[i - 1][j].status == 1)
        {
            open_cube(i - 1, j, 2);
        }
        if (!cubes_g[i - 1][j + 1].marked && cubes_g[i - 1][j + 1].status == 1)
        {
            open_cube(i - 1, j + 1, 2);
        }
        if (!cubes_g[i + 1][j - 1].marked && cubes_g[i + 1][j - 1].status == 1)
        {
            open_cube(i + 1, j - 1, 2);
        }
        if (!cubes_g[i + 1][j].marked && cubes_g[i + 1][j].status == 1)
        {
            open_cube(i + 1, j, 2);
        }
        if (!cubes_g[i + 1][j + 1].marked && cubes_g[i + 1][j + 1].status == 1)
        {
            open_cube(i + 1, j + 1, 2);
        }
  
    }
    else
    {
        //animation
        //se
    }
    return;
}



void game_main()
{
    
    static bool first_click, show_more;
    static int game_enter_anime_status,bomb_num, restart_confirm=0;
    static float bomb_time, light_flow_y, light_flow_delta_time, restart_confirm_time;
    int randtemp;
    bool break_single = 0;
    g_pD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::LightBlue));
    if (page_status == 0)   //初始化
    {
        //writelog("in page_status 0.");
        InitPage(2);
        page_status = 1;
        last_page_index = 2;
        first_click = 1;
        bomb_num = 1;
        if (set1[0].step_time_level)
        {
            bomb_time = 0.6;
        }
        else
        {
            bomb_time = 0.4;
        }
        
        light_flow_y = 900;
        light_flow_delta_time = 0;
        restart_confirm = 1;
        restart_confirm_time = 5;
        show_more = 1;
        writelog("game start.");
    }
    else if (page_status == 1)
    {
        //writelog("in page_status 1.");
        CreateButton(30, 50, 180, 100, ""/*lan[0].str_return*/, g_pBrushYellow, g_pBrushBlue, g_pBrushGreen, g_pD2DBimtapUI[10]);//0
        CreateButton(40, 400, 200, 450, "Save", g_pBrushYellow, g_pBrushBlue, g_pBrushGreen, NULL);//1
        CreateButton(40, 480, 200, 530, "Load", g_pBrushYellow, g_pBrushBlue, g_pBrushGreen, NULL);//2
        CreateButton(40, 600, 200, 650, "", g_pBrushBlue, g_pBrushYellow, g_pBrushGreen, g_pD2DBimtapUI[11]);//3
        CreateButton(40, 670, 200, 720, "more", g_pBrushYellow, g_pBrushLightBlue, g_pBrushGreen, NULL);//4
        CreateLable(420, 810, 570, 870,"0", NULL, g_pBrushDarkBlue, g_pBrushWhite, NULL, 3);//0
        CreateLable(1120, 810, 1270, 870, "0", NULL, g_pBrushDarkBlue, g_pBrushWhite, NULL, 3);//1
        if (last_page_index < 3)
        {
            if (game_mode == 0)
            {
                newMap();

            }
            else if (game_mode == 1)
            {

                int count = 0;
                //read savebuf->cubes_g

                for (int i = 1; i <= cubenum_w; i++)
                {
                    for (int j = 1; j <= cubenum_h; j++)
                    {
                        cubes[i][j].value = 0;
                        cubes_g[i][j].value = 0;
                        cubes_g[i][j].marked = 0;
                        cubes_g[i][j].status = 1;
                        cubes_g[i][j].opening = 0;

                        cubes_g[i][j].isMine = savebuf[count];
                        count++;
                    }
                }
                count = 0;
                for (int i = 1; i <= cubenum_w; i++)//刷新空块的数字
                {
                    for (int j = 1; j <= cubenum_h; j++)
                    {
                        if (cubes_g[i][j].isMine)
                        {
                            count++;
                            cubes_g[i - 1][j - 1].value++;
                            cubes_g[i - 1][j].value++;
                            cubes_g[i - 1][j + 1].value++;
                            cubes_g[i + 1][j].value++;
                            cubes_g[i + 1][j + 1].value++;
                            cubes_g[i + 1][j - 1].value++;
                            cubes_g[i][j + 1].value++;
                            cubes_g[i][j - 1].value++;
                        }
                        cubes_g[i][j].box_index = SetBox(cubes_g[i][j].posx - cubeMap_width / 2, cubes_g[i][j].posx + cubeMap_width / 2, cubes_g[i][j].posy - cubeMap_width / 2, cubes_g[i][j].posy + cubeMap_width / 2);
                    }
                }
                //writelog("dbg game_main " + to_string(count));
            }
            
            if (/*game_mode != 2*/1)
            {
                time_used = 0;
                mine_remain = mine_totle_num;
                opened_cubenum = 0;
            }
            if (set2[0].visual_effect)
            {
                game_enter_anime_status = 0;
                for (int i = 1; i <= cubenum_w; i++)
                {
                    for (int j = 1; j <= cubenum_h; j++)
                    {
                        cubes_g[i][j].opacity = 0;
                        cubes_g[i][j].act = 0;
                    }
                }

                for (int i = 1; i <= cubenum_w; i++)
                {
                    cubes_g[i][rand() % cubenum_h + 1].act = 1;
                }
                page_status = 101;
            }
            else
            {
                page_status = 2;
            }
            
        }
        else
        {
            for (int i = 1; i <= cubenum_w; i++)
            {
                for (int j = 1; j <= cubenum_h; j++)
                {
                    cubes_g[i][j].box_index = SetBox(cubes_g[i][j].posx - cubeMap_width / 2, cubes_g[i][j].posx + cubeMap_width / 2, cubes_g[i][j].posy - cubeMap_width / 2, cubes_g[i][j].posy + cubeMap_width / 2);
                }
            }
            page_status = 2;
        }
        
        last_page_index = 2;
        
        
        lables[0].text = to_string(mine_remain);
        
        //page_status = 2;
        recorded = 0;
        
        
    }
    else if (page_status == 101)
    {
        
        for (int i = 1; i <= cubenum_w; i++)
        {
            for (int j = 1; j <= cubenum_h; j++)
            {
                if (cubes_g[i][j].act == 1)
                {
                    cubes_g[i][j].opacity += dursec *300/ cubenum_h;
                    
                }
            }
        }
        for (int k = 1; k <= cubenum_h; k++)
        {
            if (cubes_g[1][k].act == 1&& cubes_g[1][k].opacity>=1)
            {
                //writelog("in page_status 101.");
                game_enter_anime_status++;
                if (game_enter_anime_status == cubenum_h)
                {
                    page_status = 2;
                }
                for (int i = 1; i <= cubenum_w; i++)
                {
                    for (int j = 1; j <= cubenum_h; j++)
                    {
                        if (cubes_g[i][j].act == 1)
                        {
                            cubes_g[i][j].act = 2;
                        }
                        
                    }
                }
                
                for (int i = 1; i <= cubenum_w; i++)
                {
                    if (cubenum_h == game_enter_anime_status)
                    {
                        randtemp = 0;
                    }
                    else
                    {
                        randtemp = rand() % (cubenum_h - game_enter_anime_status);
                    }
                    
                    for (int j = 1; j <= cubenum_h; j++)
                    {
                        
                        if (cubes_g[i][j].act == 0)
                        {
                            if (randtemp == 0)
                            {
                                cubes_g[i][j].act = 1;
                                break;
                            }
                            randtemp--;
                        }
                    }
                }
                break;
            }
            
        }
        /*for (int i = 1; i <= cubenum_w; i++)
        {
            if (cubes_g[1][1].opacity >= 1)
            {
                continue;
            }
            
            for (int j = 1; j <= cubenum_h; j++)
            {
                cubes_g[i][j].opacity += dursec * cubenum_w;
            }
            if (cubes_g[cubenum_w][1].opacity >= 1)
            {
                page_status = 2;
            }
            break;
        }*/
        if (boxes[buttons[0].box_index].clicked)
        {
            page_index = 0;
            page_status = 0;
        }
        rendMap();
    }
    else if (page_status == 2)
    {

        if (!first_click)
        {
            time_used += dursec;
        }
        
        lables[1].text = to_string((int)time_used);
        
        if (boxes[buttons[0].box_index].clicked)
        {
            boxes[buttons[0].box_index].clicked = 0;
            if (restart_confirm)
            {
                restart_confirm_time = 5;
                restart_confirm = 0;
                buttons[0].Brush1 = g_pBrushRed;
                buttons[3].Brush2_ori = g_pBrushRed;
                buttons[3].Bitmap = g_pD2DBimtapUI[12];
            }
            else
            {
                page_index = 0;
                page_status = 0;
                restart_confirm = 1;
            }
        }
        else if (boxes[buttons[1].box_index].clicked)
        {
            page_index = 3;
            page_status = 0;
        }
        else if (boxes[buttons[2].box_index].clicked)
        {
            page_index = 4;
            page_status = 0;
        }
        else if (boxes[buttons[3].box_index].clicked)
        {
                boxes[buttons[3].box_index].clicked = 0;
                if (restart_confirm)
                {
                    restart_confirm_time = 5;
                    restart_confirm = 0;
                    buttons[3].Brush2_ori = g_pBrushRed;
                    buttons[3].Bitmap = g_pD2DBimtapUI[12];
                    buttons[0].Brush1 = g_pBrushRed;
                }
                else
                {
                    page_status = 0;
                    restart_confirm = 1;
                }
            
        }
        else if (boxes[buttons[4].box_index].clicked)
        {
            boxes[buttons[4].box_index].clicked = 0;
            if (show_more)
            {
                buttons[4].Brush2_ori = g_pBrushLightBlue;
                buttons[4].Brush3 = g_pBrushGreen;
                show_more = 0;
                for (int i = 0; i < 4; i++)
                {
                    boxes[buttons[i].box_index].active = 0;
                    buttons[i].active = 0;
                }
            }
            else
            {
                buttons[4].Brush2_ori = g_pBrushLight;
                buttons[4].Brush3 = g_pBrushGreen;
                show_more = 1;
                for (int i = 0; i < 4; i++)
                {
                    boxes[buttons[i].box_index].active = 1;
                    buttons[i].active = 1;
                }
            }
        }
        if (!restart_confirm)
        {
            restart_confirm_time -= dursec;
            if (restart_confirm_time < 0)
            {
                restart_confirm = 1;
                buttons[3].Brush2_ori = g_pBrushYellow;
                buttons[0].Brush1 = g_pBrushYellow;
                buttons[3].Bitmap = g_pD2DBimtapUI[11];
            }
        }
        for (int i = 1; i <= cubenum_w; i++)
        {
            for (int j = 1; j <= cubenum_h; j++)
            {
                if (cubes_g[i][j].status == 1)
                {
                    if (boxes[cubes_g[i][j].box_index].focus)
                    {
                        if (set2[0].visual_effect)
                        {
                            if (cubes_g[i][j].opacity > 0.95f)
                            {
                                cubes_g[i][j].opacity = 0.85f;
                            }
                            else if (cubes_g[i][j].opacity > 0.7f)
                            {
                                cubes_g[i][j].opacity -= dursec;
                            }
                        }
                        else
                        {
                            cubes_g[i][j].opacity = 0.7;
                        }
                        
                        if (boxes[cubes_g[i][j].box_index].click_status)
                        {
                            cubes_g[i][j].opacity = 0.5;
                        }
                    }
                    else
                    {
                        if (set2[0].visual_effect)
                        {
                            if (cubes_g[i][j].opacity < 1.0f)
                            {
                                cubes_g[i][j].opacity += dursec;
                            }
                        }
                        else
                        {
                            cubes_g[i][j].opacity = 1.0;
                        }
                        
                    }
                    if (boxes[cubes_g[i][j].box_index].clicked)
                    {
                        boxes[cubes_g[i][j].box_index].clicked = 0;
                        if (first_click)
                        {
                            if (game_mode == 0)
                            {
                                while (cubes_g[i][j].value != 0 || cubes_g[i][j].isMine)
                                {
                                    newMap();
                                }
                            }
                            writelog("first click.");
                            first_click = 0;
                        }
                        if (cubes_g[i][j].marked == 0)
                        {
                            open_cube(i, j, 0);
                        }
                        else
                        {
                            playeffectsound(7);
                        }
                        
                    }
                    if (boxes[cubes_g[i][j].box_index].right_clicked)
                    {
                        boxes[cubes_g[i][j].box_index].right_clicked = 0;
                        if (first_click == 0)
                        {
                            cubes_g[i][j].marked = !cubes_g[i][j].marked;
                            if (!cubes_g[i][j].marked)
                            {
                                mine_remain++;
                            }
                            else
                            {
                                mine_remain--;
                            }
                            lables[0].text = to_string(mine_remain);
                        }
                        
                    }
                    if (cubes_g[i][j].opening)
                    {
                        cubes_g[i][j].open_time -= dursec;
                        if (cubes_g[i][j].open_time < 0)
                        {
                            cubes_g[i][j].opening = 0;
                            open_cube(i, j,0);
                        }
                    }
                    
                }
                else
                {
                    if (boxes[cubes_g[i][j].box_index].double_clicked)
                    {
                        boxes[cubes_g[i][j].box_index].double_clicked = 0;
                        doubleClick_openCube(i, j);
                    }
                    if (boxes[cubes_g[i][j].box_index].rightclick_status)
                    {
                        //boxes[cubes_g[i][j].box_index].right_clicked = 0;
                        cubes_g[i][j-1].opacity = 0.7;
                        cubes_g[i][j + 1].opacity = 0.7;
                        cubes_g[i-1][j - 1].opacity = 0.7;
                        cubes_g[i-1][j ].opacity = 0.7;
                        cubes_g[i-1][j + 1].opacity = 0.7;
                        cubes_g[i+1][j - 1].opacity = 0.7;
                        cubes_g[i+1][j ].opacity = 0.7;
                        cubes_g[i+1][j + 1].opacity = 0.7;
                    }
                }
                
            }
        }
        rendMap();
    }
    else if (page_status == 102)    //lose
    {
        //writelog("in page_status 102.");
        if (recorded == 0&&game_mode==0)
        {
            if (record_once())
            {
                recorded = 1;
            }
        }
        bomb_time -= dursec;
        if (bomb_time<0)
        {
            bomb_num++;
            if (set1[0].step_time_level)
            {
                bomb_time = 1.0 / bomb_num;
            }
            else
            {
                bomb_time = 0.5 / bomb_num;
            }
            
            if (bomb_num == mine_totle_num)
            {
                randtemp = 0;
            }
            else
            {
                randtemp = rand() % (mine_totle_num - bomb_num);
            }
            for (int i = 1; i <= cubenum_w; i++)
            {
                
                for (int j = 1; j <= cubenum_h; j++)
                {
                    if (cubes_g[i][j].isMine&& cubes_g[i][j].status==1)
                    {
                        if (randtemp == 0)
                        {
                            open_cube(i, j, 3);
                            createEffect(3, cubes_g[i][j].posx - cubeMap_width / 2, cubes_g[i][j].posy - cubeMap_width / 2);
                            CreateAnimation(rand() % 4 + 5, cubes_g[i][j].posx-cubeMap_width/2, cubes_g[i][j].posy - cubeMap_width / 2);
                            playeffectsound(rand() % 7 + 8);
                            break_single = 1;
                            break;
                        }
                        else
                        {
                            randtemp--;
                        }
                    }
                }
                if (break_single)
                {
                    break;
                }
            }
        }
        rendMap();
        if (bomb_num == mine_totle_num)
        {
            page_status = 3;
        }
    }
    else if (page_status == 103)    //win
    {
        //writelog("in page_status 103.");
        mine_remain = 0;
        if (recorded==0 && game_mode == 0)
        {
            if (record_once())
            {
                recorded = 1;
            }
        }
        light_flow_delta_time += dursec;
        if (light_flow_delta_time > 0.02)
        {
            createEffect(2, centerx - (float)cubenum_w / 2 * cubeMap_width, light_flow_y);
            if (set1[0].step_time_level)
            {
                light_flow_y -= light_flow_delta_time * 300;
            }
            else
            {
                light_flow_y -= light_flow_delta_time * 600;
            }
            
            if (light_flow_y < centery - (float)cubenum_h / 2 * cubeMap_width)
            {
                page_status = 3;
            }
            light_flow_delta_time = 0;
        }
        rendMap();
        
    }
    else if (page_status == 3)  
    {
        //writelog("in page_status 3.");
        if (boxes[buttons[0].box_index].clicked)
        {
            page_index = 0;
            page_status = 0;
        }
        if (boxes[buttons[1].box_index].clicked)
        {
            page_index = 3;
            page_status = 0;
        }
        if (boxes[buttons[2].box_index].clicked)
        {
            page_index = 4;
            page_status = 0;
        }
        if (boxes[buttons[3].box_index].clicked)
        {
            page_index = 2;
            page_status = 0;
        }
        rendMap();
        if (recorded == -1)
        {
            g_pD2DDeviceContext->DrawText(
                multiByteToWideChar("Record failed!"),           // Text to render
                wcslen(multiByteToWideChar("Record failed!")),       // Text length
                g_pTextFormat,     // Text format
                textrbRect,    // The region of the window where the text will be rendered
                g_pBrushRed      // The brush used to draw the text
            );
            DrawCallNum++;
        }
    }
    if (page_status > 1)
    {
        g_pBrushLight->SetOpacity(0.6);
        g_pD2DDeviceContext->FillEllipse(D2D1::Ellipse(D2D1::Point2(to_screen(365), to_screen(835)), to_screen(40), to_screen(40)), g_pBrushLight);
        g_pD2DDeviceContext->FillEllipse(D2D1::Ellipse(D2D1::Point2(to_screen(1065), to_screen(835)), to_screen(40), to_screen(40)), g_pBrushLight);
        DrawBitmap_1(g_pD2DBimtapUI[15], D2D1::RectF(330, 800, 400, 870), 1.0);
        DrawBitmap_1(g_pD2DBimtapUI[30], D2D1::RectF(1030, 800, 1100, 870), 1.0);
        DrawCallNum += 4;
        if (game_mode > 0)
        {
            g_pD2DDeviceContext->DrawText(
                multiByteToWideChar("Review Mode"),           // Text to render
                wcslen(multiByteToWideChar("Review Mode")),       // Text length
                g_pTextFormat,     // Text format
                textrbRect,    // The region of the window where the text will be rendered
                g_pBrushGreen      // The brush used to draw the text
            );
            DrawCallNum++;
        }
    }
    return;
}

bool record_once()
{
    static bool recording = 0;
    int counter = 0,count=0;
    
    //更新highscore
    
    if (thread_IO_request_record_game == 0&&!recording)
    {
        writelog("in record_once()");

        timebuf = time_used;
        flagbuf = mine_totle_num - mine_remain;
        opened_cubenum_buf = opened_cubenum;
        for (int i = 1; i <= cubenum_w; i++)
        {
            for (int j = 1; j <= cubenum_h; j++)
            {
                
                    if (cubes_g[i][j].isMine)
                    {
                        savebuf[counter] = 1;
                        count++;
                    }
                    else
                    {
                        savebuf[counter] = 0;
                    }
                

                counter++;
            }
        }
        //writelog("dbg record_once " + to_string(count));
        thread_IO_request_record_game = 1;
        recording = 1;
        return 0;
    }
    /*else if (thread_IO_request_record_game == 1)
    {
        return 0;
    }*/
    else if (thread_IO_request_record_game == 0 && recording)
    {
        recording = 0;
        return 1;
    }
    else if (thread_IO_request_record_game < 0)
    {
        recording = 0;
        recorded = -1;
        return 0;
    }
    /*else if (thread_IO_request_delete_game == -1)
    {
        playeffectsound(6);
        thread_IO_request_record_game = -1;
        writelog("skip record.");
        return 1;
    }
    else
    {
        g_pD2DDeviceContext->DrawText(
            multiByteToWideChar("Deleting..."),           // Text to render
            wcslen(multiByteToWideChar("Deleting...")),       // Text length
            g_pTextFormat,     // Text format
            textrbRect,    // The region of the window where the text will be rendered
            g_pBrushBlack      // The brush used to draw the text
        );
        DrawCallNum++;
        return 0;
    }*/
    
    
    //更新history
    //GetLocalTime(&st);

    return 0;
}
//逻辑部分结束

//page_index=0
int homepage_status = 0;
float transform_x = 0, transform_y = 0, bg_transform_x=0, bg_transform_y=0;
struct letter
{
    float posx = 0;
    float posy = 0;
    float target_posx = 0;
    float target_posy = 0;
    float speedx = 0;
    float speedy = 0;
    float opacity = 0.4;
    bool effect = 0;//0 for dark,1 for light
    float effect_level = 0;
    bool effect_stage = 0;
    float effect_wait_time = 0;
    string sletter = "";
};
letter letters[11];
void rendLetter()
{
    for (int i = 0; i < 11; i++)
    {
        g_pBitmapBrushUI[13]->SetOpacity(letters[i].opacity);
        g_pD2DDeviceContext->DrawText(
            multiByteToWideChar(letters[i].sletter),           // Text to render
            wcslen(multiByteToWideChar(letters[i].sletter)),       // Text length
            g_pTextFormatLarge,     // Text format
            D2D1::RectF(to_screen(letters[i].posx-30), to_screen(letters[i].posy - 30), to_screen(letters[i].posx + 30), to_screen(letters[i].posy + 30)),    // The region of the window where the text will be rendered
            g_pBitmapBrushUI[13]      // The brush used to draw the text
        );
        g_pBitmapBrushUI[13]->SetOpacity(1.0f);
        if (!long_credit)
        {
            if (letters[i].effect)
            {
                g_pBrushLight->SetOpacity(letters[i].effect_level);
                g_pD2DDeviceContext->DrawText(
                    multiByteToWideChar(letters[i].sletter),           // Text to render
                    wcslen(multiByteToWideChar(letters[i].sletter)),       // Text length
                    g_pTextFormatLarge,     // Text format
                    D2D1::RectF(to_screen(letters[i].posx - 30), to_screen(letters[i].posy - 30), to_screen(letters[i].posx + 30), to_screen(letters[i].posy + 30)),    // The region of the window where the text will be rendered
                    g_pBrushLight      // The brush used to draw the text
                );
                g_pBrushLight->SetOpacity(0.6);
            }
            else
            {
                g_pBrushDark->SetOpacity(letters[i].effect_level);
                g_pD2DDeviceContext->DrawText(
                    multiByteToWideChar(letters[i].sletter),           // Text to render
                    wcslen(multiByteToWideChar(letters[i].sletter)),       // Text length
                    g_pTextFormatLarge,     // Text format
                    D2D1::RectF(to_screen(letters[i].posx - 30), to_screen(letters[i].posy - 30), to_screen(letters[i].posx + 30), to_screen(letters[i].posy + 30)),    // The region of the window where the text will be rendered
                    g_pBrushDark      // The brush used to draw the text
                );
                g_pBrushDark->SetOpacity(0.6);
            }
            DrawCallNum++;
        }
        
        DrawCallNum ++;
    }
    
    return;
}
void rend_home_page()
{
    if (page_status == 0)
    {
        InitPage(0);
        page_status = 1;
        last_page_index = 0;
    }
    else if (page_status == 1)
    {
        CreateButton(280, 700, 470, 800, "Start",g_pBrushYellow ,g_pBrushBlue , g_pBrushGreen, NULL);//0
        CreateButton(900, 670, 1050, 750, ""/*lan[0].quit*/, g_pBrushYellow, g_pBrushBlue, g_pBrushGreen, g_pD2DBimtapUI[1]);//1 
        CreateButton(600, 680, 750, 750, "", g_pBrushYellow, g_pBrushBlue, g_pBrushGreen, g_pD2DBimtapUI[0]);//2
        CreateButton(300, 500, 450, 580, "Load", g_pBrushYellow, g_pBrushBlue, g_pBrushGreen, NULL);//3
        CreateButton(300, 370, 450, 450, "History", g_pBrushYellow, g_pBrushBlue, g_pBrushGreen, NULL);//4
        page_status = 2;
        letters[0].sletter = "M";
        letters[1].sletter = "i";
        letters[2].sletter = "n";
        letters[3].sletter = "e";
        letters[4].sletter = "S";
        letters[5].sletter = "w";
        letters[6].sletter = "e";
        letters[7].sletter = "e";
        letters[8].sletter = "p";
        letters[9].sletter = "e";
        letters[10].sletter = "r";
        
        transform_x = rand() % 4000;
        transform_y = rand() % 4000;
        bg_transform_x = rand() % 4000;
        bg_transform_y = rand() % 4000;
        for (int i = 0; i < 11; i++)
        {
            letters[i].posx = rand() % 1500+50;
            letters[i].posy = rand() % 800+50;
            letters[i].opacity = 0.4;
            letters[i].target_posx = 400+i*100;
            letters[i].target_posy = 200 + rand() % 4*20;
            letters[i].speedx= (letters[i].target_posx- letters[i].posx)/0.8f;
            letters[i].speedy = (letters[i].target_posy - letters[i].posy) / 0.8f;
            letters[i].effect = rand() % 2;
            letters[i].effect_stage = 0;
            letters[i].effect_wait_time = i+1;
            letters[i].effect_level = 0;
        }
        homepage_status = 0;
    }
    else if (page_status == 2)
    {
        g_pD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::Lavender));
        if (!long_credit)
        {
            g_pBitmapBrushUI[12]->SetTransform(D2D1::Matrix3x2F::Translation(D2D1::SizeF(bg_transform_x * scale, bg_transform_y * scale)));
            g_pD2DDeviceContext->FillRectangle(

                D2D1::RectF(0, 0, to_screen(1600), to_screen(900)),
                g_pBitmapBrushUI[12]
            );
            g_pBrushLight->SetOpacity(0.5);
            g_pD2DDeviceContext->FillRectangle(

                D2D1::RectF(0, 0, to_screen(1600), to_screen(900)),
                g_pBrushLight
            );
            DrawCallNum += 2;
        }
        if (homepage_status == 0)
        {
            for (int i = 0; i < 11; i++)
            {
                letters[i].posx += letters[i].speedx * dursec;
                letters[i].posy += letters[i].speedy * dursec;
                letters[i].opacity += 0.75f * dursec;
                if (letters[i].opacity > 1)
                {
                    homepage_status = 1;
                }
            }
            
            
        }
        else if (homepage_status == 1)
        {
            
            transform_x += dursec * 50;
            transform_y += dursec * 50;
            bg_transform_x += dursec * 50;
            bg_transform_y += dursec * 50;
            g_pBitmapBrushUI[13]->SetTransform(D2D1::Matrix3x2F::Translation(D2D1::SizeF(transform_x * scale, transform_y * scale)));
            for (int i = 0; i < 11; i++)
            {
                if (letters[i].effect_wait_time < 0)
                {
                    if (letters[i].effect_stage == 0)
                    {
                        letters[i].effect_level += dursec;
                        if (letters[i].effect_level > 0.6)
                        {
                            letters[i].effect_stage = 1;

                        }
                    }
                    else
                    {
                        letters[i].effect_level -= dursec;
                        if (letters[i].effect_level < 0)
                        {
                            letters[i].effect_level = 0;
                            letters[i].effect_stage = 0;
                            letters[i].effect = rand() % 2;
                            letters[i].effect_wait_time = 4+rand()%5;
                        }
                    }
                }
                else
                {
                    letters[i].effect_wait_time -=dursec;
                }
                
            }
                
            

        }
        
        if (boxes[buttons[1].box_index].clicked)
        {
            normal_quit = 1;
        }
        else if (boxes[buttons[2].box_index].clicked)
        {
            page_index = 1;
            page_status = 0;
        }
        else if (boxes[buttons[0].box_index].clicked)
        {
            game_mode = 0;
            page_index = 2;
            page_status = 0;
            createEffect(1, 0.0f, 0.0f);
        }
        else if (boxes[buttons[3].box_index].clicked)
        {
            last_page_index = 0;
            page_index = 4;
            page_status = 0;
        }
        else if (boxes[buttons[4].box_index].clicked)
        {
            
            page_index = 5;
            page_status = 0;
        }
        rendLetter();
        return;
    }
    return;
}
int setting_page_index = 1;
string setting_descreption = "";
int button_drag = 0;
bool graphic_changed = 0;
void rend_setting_page()
{
    g_pD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::LightBlue));
    if (page_status == 0)   //初始化
    {
        InitPage(1);
        page_status = 1;
        
        //setting_page_index = 1;
    }
    else if (page_status == 1)  //初始化
    {
        CreateButton(100,50,250,100, ""/*lan[0].str_return*/, g_pBrushYellow, g_pBrushBlue, g_pBrushGreen, g_pD2DBimtapUI[10]);//0
        CreateLable(100, 790, 1500, 880, "", NULL, g_pBrushPurple, g_pBrushBlack, NULL,0);//0
        if (setting_page_index == 1)
        {

            CreateButton(400, 150, 550, 220, lan[0].general,g_pBrushRed , g_pBrushBlue, g_pBrushGreen, NULL);//1
            CreateButton(725, 150, 875, 220, lan[0].graphics, g_pBrushYellow, g_pBrushBlue, g_pBrushGreen, NULL);//2
            CreateButton(1050, 150, 1200, 220, lan[0].audio, g_pBrushYellow, g_pBrushBlue, g_pBrushGreen, NULL);//3

            CreateButton(1250, 300, 1350, 380, lan[0].on, g_pBrushYellow, g_pBrushBlue, g_pBrushBlack, NULL);//4
            CreateButton(1250, 400, 1350, 480, lan[0].on, g_pBrushYellow, g_pBrushBlue, g_pBrushBlack, NULL);//5
            CreateButton(1250, 500, 1350, 580, lan[0].mode1, g_pBrushYellow, g_pBrushBlue, g_pBrushBlack, NULL);//6
            CreateButton(1250, 600, 1350, 680, lan[0].on, g_pBrushYellow, g_pBrushBlue, g_pBrushBlack, NULL);//7
            //CreateButton(1250, 700, 1350, 780, ""/*lan[0].on*/, g_pBrushYellow, g_pBrushBlue, g_pBrushBlack, NULL);//8

            if (set1[0].language_translation == 0)
            {
                buttons[4].Brush2_ori = g_pBrushRed;
                buttons[4].text = lan[0].off;
            }
            else
            {
                buttons[4].Brush2_ori = g_pBrushLightGreen;
                buttons[4].text = lan[0].on;
            }
            if (set1[0].map_size == 1)
            {
                //buttons[5].Brush2_ori = g_pBrushRed;
                buttons[5].text = lan[0].easy;
            }
            else if (set1[0].map_size == 2)
            {
                //buttons[5].Brush2_ori = g_pBrushLightGreen;
                buttons[5].text = lan[0].normal;
            }
            else
            {
                buttons[5].text = lan[0].hard;
            }
            if (set1[0].step_time_level == 0)
            {
                
                buttons[6].text = lan[0].mode1;
            }
            else
            {
                
                buttons[6].text = lan[0].mode2;
            }
            if (set1[0].tips == 0)
            {
                buttons[7].Brush2_ori = g_pBrushRed;
                buttons[7].text = lan[0].off;
            }
            else
            {
                buttons[7].Brush2_ori = g_pBrushLightGreen;
                buttons[7].text = lan[0].on;
            }
            /*if (set1[0].mouse_input == 0)
            {
                buttons[8].Brush2_ori = g_pBrushRed;
                buttons[8].text = lan[0].off;
            }
            else
            {
                buttons[8].Brush2_ori = g_pBrushLightGreen;
                buttons[8].text = lan[0].on;
            }*/

            CreateLable(100, 300, 900, 380, lan[0].language_translation, g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL,1);//1
            CreateLable(100, 400, 900, 480, lan[0].map_size, g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL,1);//2
            CreateLable(100, 500, 900, 580, lan[0].step_time_level, g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL,1);//3
            CreateLable(100, 600, 900, 680, lan[0].tips, g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL,1);//4
            //CreateLable(100, 700, 900, 780, lan[0].mouse_input, g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL,1);//5

        }
        else if (setting_page_index == 2)
        {
            lables[0].text = "Supported by DirectX 11.0";
            CreateButton(400, 150, 550, 220, lan[0].general, g_pBrushYellow, g_pBrushBlue, g_pBrushGreen, NULL);//1
            CreateButton(725, 150, 875, 220, lan[0].graphics, g_pBrushRed, g_pBrushBlue, g_pBrushGreen, NULL);//2
            CreateButton(1050, 150, 1200, 220, lan[0].audio, g_pBrushYellow, g_pBrushBlue, g_pBrushGreen, NULL);//3

            CreateButton(1250, 300, 1350, 380, lan[0].on, g_pBrushYellow, g_pBrushBlue, g_pBrushBlack, NULL);//4
            CreateButton(1250, 400, 1350, 480, lan[0].on, g_pBrushYellow, g_pBrushBlue, g_pBrushBlack, NULL);//5
            CreateButton(1250, 500, 1350, 580, lan[0].on, g_pBrushYellow, g_pBrushBlue, g_pBrushBlack, NULL);//6
            CreateButton(1250, 600, 1350, 680, lan[0].on, g_pBrushYellow, g_pBrushBlue, g_pBrushBlack, NULL);//7
            CreateButton(1250, 700, 1350, 780, lan[0].on, g_pBrushYellow, g_pBrushBlue, g_pBrushBlack, NULL);//8
            if (set2[0].visual_effect == 0)
            {
                buttons[4].Brush2_ori = g_pBrushRed;
                buttons[4].text = lan[0].off;
            }
            else
            {
                buttons[4].Brush2_ori = g_pBrushLightGreen;
                buttons[4].text = lan[0].on;
            }
            if (set2[0].vsync == 0)
            {
                buttons[5].Brush2_ori = g_pBrushRed;
                buttons[5].text = lan[0].off;
            }
            else
            {
                buttons[5].Brush2_ori = g_pBrushLightGreen;
                buttons[5].text = lan[0].on;
            }
            if (set2[0].MSAA == 0)
            {
                buttons[6].Brush2_ori = g_pBrushRed;
                buttons[6].text = lan[0].off;
            }
            else
            {
                buttons[6].Brush2_ori = g_pBrushLightGreen;
                buttons[6].text = lan[0].on;
            }
            switch (set2[0].resolution)
            {
            case 1:
                buttons[7].text = "450P";
                break;
            case 2:
                buttons[7].text = "720P";
                break;
            case 3:
                buttons[7].text = "900P";
                break;
            case 4:
                buttons[7].text = "1080P";
                break;
            case 5:
                buttons[7].text = "2K";
                break;
            case 6:
                buttons[7].text = "4K";
                break;
            default:
                break;
            }
            /*if (set2[0].theme == 1)
            {
                
                buttons[7].text = lan[0].mode1;
            }
            else
            {
                
                buttons[7].text = lan[0].mode2;
            }*/
            if (set2[0].show_framerate == 0)
            {
                buttons[8].Brush2_ori = g_pBrushRed;
                buttons[8].text = lan[0].off;
            }
            else
            {
                buttons[8].Brush2_ori = g_pBrushLightGreen;
                buttons[8].text = lan[0].on;
            }
            CreateLable(100, 300, 900, 380, lan[0].visual_effect, g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL,1);//1
            CreateLable(100, 400, 900, 480, lan[0].vsync, g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL,1);//2
            CreateLable(100, 500, 900, 580, lan[0].MSAA, g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL,1);//3
            CreateLable(100, 600, 900, 680, lan[0].resolution, g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL,1);//4
            CreateLable(100, 700, 900, 780, lan[0].show_framerate, g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL,1);//5
        }
        else if (setting_page_index == 3)
        {
            lables[0].text = "Supported by OpenAL";
            CreateButton(400, 150, 550, 220, lan[0].general, g_pBrushYellow, g_pBrushBlue, g_pBrushGreen, NULL);//1
            CreateButton(725, 150, 875, 220, lan[0].graphics, g_pBrushYellow,g_pBrushBlue , g_pBrushGreen, NULL);//2
            CreateButton(1050, 150, 1200, 220, lan[0].audio, g_pBrushRed, g_pBrushBlue, g_pBrushGreen, NULL);//3

            
            CreateButton(1250, 600, 1350, 680, lan[0].mode1, g_pBrushYellow, g_pBrushBlue, g_pBrushBlack, NULL);//4
            CreateButton(600, 350, 650, 380, "", g_pBrushYellow, g_pBrushYellow, g_pBrushBlack, NULL);//5
            CreateButton(600, 450, 650, 480, "", g_pBrushYellow, g_pBrushYellow, g_pBrushBlack, NULL);//6
            buttons[5].x1 = 400 + 9.0 * set3[0].music_volume-25;
            buttons[5].x2 = 400 + 9.0 * set3[0].music_volume + 25;
            buttons[6].x1 = 400 + 9.0 * set3[0].se_volume - 25;
            buttons[6].x2 = 400 + 9.0 * set3[0].se_volume + 25;
            boxes[buttons[5].box_index].x1 = to_screen(buttons[5].x1);
            boxes[buttons[5].box_index].x2 = to_screen(buttons[5].x2);
            boxes[buttons[6].box_index].x1 = to_screen(buttons[6].x1);
            boxes[buttons[6].box_index].x2 = to_screen(buttons[6].x2);
            if (set3[0].thread_method == 0)
            {
                
                buttons[4].text = lan[0].mode1;
            }
            else
            {
                
                buttons[4].text = lan[0].mode2;
            }
            CreateLable(100, 600, 900, 680, lan[0].thread_method, g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL, 1);//1
            CreateLable(400, 360, 1300, 370, "", NULL, g_pBrushPurple, g_pBrushBlack, NULL, 1);//2
            CreateLable(400, 460, 1300, 470, "", NULL, g_pBrushPurple, g_pBrushBlack, NULL, 1);//3
            CreateLable(1400, 280, 1550, 380, to_string(set3[0].music_volume), g_pBrushYellow, g_pBrushBlue, g_pBrushGreen, NULL, 1);//4
            CreateLable(1400, 420, 1550, 520, to_string(set3[0].se_volume), g_pBrushYellow, g_pBrushBlue, g_pBrushGreen, NULL, 1);//5
            CreateLable(100, 280, 360, 380, lan[0].music_volume, g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL, 1);//6
            CreateLable(100, 420, 360, 520, lan[0].se_volume, g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL, 1);//7

        }




        page_status = 2;
    }
    else if (page_status == 2)   //就绪
    {
    
        if (setting_page_index == 1)
        {
            DrawBitmap_1(
                g_pD2DBimtapUI[2],
                D2D1::RectF(600,
                    250,
                    1000,
                    660),
                0.5

            );
            if (boxes[buttons[4].box_index].focus)
            {
                lables[0].text = lan[0].language_translation_description;
            }
            else if (boxes[buttons[5].box_index].focus)
            {
                lables[0].text = lan[0].mapsize_description;
            }
            else if (boxes[buttons[6].box_index].focus)
            {
                lables[0].text = lan[0].step_time_level_description;
            }
            else if (boxes[buttons[7].box_index].focus)
            {
                lables[0].text = lan[0].tips_description;
            }
            else if (boxes[buttons[8].box_index].focus)
            {
                //lables[0].text = lan[0].mouse_input_description;
            }

            if (boxes[buttons[4].box_index].clicked)
            {
                boxes[buttons[4].box_index].clicked = 0;
                if (set1[0].language_translation == 0)
                {
                    lan[0] = lan[2];
                    set1[0].language_translation = 1;
                    buttons[4].Brush2_ori = g_pBrushLightGreen;
                    buttons[4].text = lan[0].on;

                }
                else
                {
                    lan[0] = lan[1];
                    set1[0].language_translation = 0;
                    buttons[4].Brush2_ori = g_pBrushRed;
                    buttons[4].text = lan[0].off;
                }
                
                
            }
            if (boxes[buttons[5].box_index].clicked)
            {
                boxes[buttons[5].box_index].clicked = 0;
                if (set1[0].map_size == 1)
                {
                    set1[0].map_size = 2;
                    
                    buttons[5].text = lan[0].normal;
                }
                else if (set1[0].map_size == 2)
                {
                    set1[0].map_size = 3;
                    
                    buttons[5].text = lan[0].hard;
                }
                else if (set1[0].map_size == 3)
                {
                    set1[0].map_size = 1;

                    buttons[5].text = lan[0].easy;
                }
            }
            if (boxes[buttons[6].box_index].clicked)
            {
                boxes[buttons[6].box_index].clicked = 0;
                if (set1[0].step_time_level == 0)
                {
                    set1[0].step_time_level = 1;

                    buttons[6].text = lan[0].mode2;
                }
                else
                {
                    set1[0].step_time_level = 0;

                    buttons[6].text = lan[0].mode1;
                }
            }
            if (boxes[buttons[7].box_index].clicked)
            {
                boxes[buttons[7].box_index].clicked = 0;
                if (set1[0].tips == 0)
                {
                    set1[0].tips = 1;
                    buttons[7].Brush2_ori = g_pBrushLightGreen;
                    buttons[7].text = lan[0].on;
                }
                else
                {
                    set1[0].tips = 0;
                    buttons[7].Brush2_ori = g_pBrushRed;
                    buttons[7].text = lan[0].off;
                }
            }
            //if (boxes[buttons[8].box_index].clicked)
            //{
            //    boxes[buttons[8].box_index].clicked = 0;
            //    /*if (set1[0].mouse_input == 0)
            //    {
            //        set1[0].mouse_input = 1;
            //        buttons[8].Brush2_ori = g_pBrushLightGreen;
            //        buttons[8].text = lan[0].on;
            //    }
            //    else
            //    {
            //        set1[0].mouse_input = 0;
            //        buttons[8].Brush2_ori = g_pBrushRed;
            //        buttons[8].text = lan[0].off;
            //    }*/
            //}
        }
        else if (setting_page_index == 2)
        {
        if (graphic_changed)
        {
            DrawBitmap_1(
                g_pD2DBimtapUI[4],
                D2D1::RectF(550,
                    200,
                    1050,
                    700),
                0.5

            );
        }
        else
        {
            DrawBitmap_1(
                g_pD2DBimtapUI[3],
                D2D1::RectF(600,
                    250,
                    1000,
                    660),
                0.5

            );
        }
            

            if (boxes[buttons[4].box_index].focus)
            {
                lables[0].text = lan[0].visual_effect_description;
            }
            else if (boxes[buttons[5].box_index].focus)
            {
                lables[0].text = lan[0].vsync_description;
            }
            else if (boxes[buttons[6].box_index].focus)
            {
                lables[0].text = lan[0].MSAA_description;
                lables[0].text += "\n";
                lables[0].text += lan[0].sampleCount;
                lables[0].text += to_string(sampleCountOut);
                lables[0].text += "x";
            }
            else if (boxes[buttons[7].box_index].focus)
            {
                lables[0].text = lan[0].resolution_description;
            }
            else if (boxes[buttons[8].box_index].focus)
            {
                lables[0].text = lan[0].show_framerate_description;
            }

            if (boxes[buttons[4].box_index].clicked)
            {
                boxes[buttons[4].box_index].clicked = 0;
                cleanEffects();
                if (set2[0].visual_effect == 0)
                {
                    set2[0].visual_effect = 1;
                    buttons[4].Brush2_ori = g_pBrushLightGreen;
                    buttons[4].text = lan[0].on;
                    
                }
                else
                {
                    set2[0].visual_effect = 0;
                    buttons[4].Brush2_ori = g_pBrushRed;
                    buttons[4].text = lan[0].off;
                    
                }


            }
            if (boxes[buttons[5].box_index].clicked)
            {
                boxes[buttons[5].box_index].clicked = 0;
                if (set2[0].vsync == 0)
                {
                    set2[0].vsync = 1;
                    buttons[5].Brush2_ori = g_pBrushLightGreen;
                    buttons[5].text = lan[0].on;
                }
                else
                {
                    set2[0].vsync = 0;
                    buttons[5].Brush2_ori = g_pBrushRed;
                    buttons[5].text = lan[0].off;
                }
            }
            if (boxes[buttons[6].box_index].clicked)
            {
                boxes[buttons[6].box_index].clicked = 0;
                graphic_changed = 1;
                if (set2[0].MSAA == 0)
                {
                    set2[0].MSAA = 1;
                    buttons[6].Brush2_ori = g_pBrushLightGreen;
                    buttons[6].text = lan[0].on;
                }
                else
                {
                    set2[0].MSAA = 0;
                    buttons[6].Brush2_ori = g_pBrushRed;
                    buttons[6].text = lan[0].off;
                }
            }
            if (boxes[buttons[7].box_index].clicked)
            {
                graphic_changed = 1;
                boxes[buttons[7].box_index].clicked = 0;
                set2[0].resolution++;
                if (set2[0].resolution > resolution_allow)
                {
                    set2[0].resolution = 1;
                }
                switch (set2[0].resolution)
                {
                case 1:
                    buttons[7].text = "450P";
                    break;
                case 2:
                    buttons[7].text = "720P";
                    break;
                case 3:
                    buttons[7].text = "900P";
                    break;
                case 4:
                    buttons[7].text = "1080P";
                    break;
                case 5:
                    buttons[7].text = "2K";
                    break;
                case 6:
                    buttons[7].text = "4K";
                    break;
                default:
                    break;
                }
                /*if (set2[0].theme == 1)
                {
                    set2[0].theme = 2;
                    
                    buttons[7].text = lan[0].mode2;
                }
                else
                {
                    set2[0].theme = 1;
                    
                    buttons[7].text = lan[0].mode1;
                }*/
            }
            if (boxes[buttons[8].box_index].clicked)
            {
                boxes[buttons[8].box_index].clicked = 0;
                if (set2[0].show_framerate == 0)
                {
                    set2[0].show_framerate = 1;
                    buttons[8].Brush2_ori = g_pBrushLightGreen;
                    buttons[8].text = lan[0].on;
                }
                else
                {
                    set2[0].show_framerate = 0;
                    buttons[8].Brush2_ori = g_pBrushRed;
                    buttons[8].text = lan[0].off;
                }
            }
        }
        else if (setting_page_index == 3)
        {
        if (set3[0].thread_method)
        {
            DrawBitmap_1(
                g_pD2DBimtapUI[6],
                D2D1::RectF(600,
                    250,
                    1000,
                    660),
                0.5

            );
        }
        else
        {
            DrawBitmap_1(
                g_pD2DBimtapUI[5],
                D2D1::RectF(600,
                    250,
                    1000,
                    660),
                0.5

            );
        }
            

            if (boxes[buttons[4].box_index].focus)
            {
                lables[0].text = lan[0].thread_method_description;
            }

            if (boxes[buttons[4].box_index].clicked)
            {
                boxes[buttons[4].box_index].clicked = 0;
                if (set3[0].thread_method == 0)
                {
                    set3[0].thread_method = 1;
                    
                    buttons[4].text = lan[0].mode2;
                }
                else
                {
                    set3[0].thread_method = 0;
                    
                    buttons[4].text = lan[0].mode1;
                }


            }
            if (boxes[buttons[5].box_index].click_status && button_drag == 0 || button_drag == 1)
            {
                if (boxes[buttons[5].box_index].click_status)
                {
                    button_drag = 1;
                }
                if (!clicking)
                {
                    button_drag = 0;
                    thread_Audio_volume_changed_music = 1;

                }
                
                set3[0].music_volume = (cpos.x/scale - 400) / 9;
                if (set3[0].music_volume < 0)
                {
                    set3[0].music_volume = 0;
                }
                else if (set3[0].music_volume > 100)
                {
                    set3[0].music_volume = 100;
                }
                lables[4].text = to_string(set3[0].music_volume);
                buttons[5].x1 = 400 + 9.0 * set3[0].music_volume - 25;
                buttons[5].x2 = 400 + 9.0 * set3[0].music_volume + 25;
                boxes[buttons[5].box_index].x1 = to_screen(buttons[5].x1);
                boxes[buttons[5].box_index].x2 = to_screen(buttons[5].x2);
                

            }
            if (boxes[buttons[6].box_index].click_status && button_drag == 0 || button_drag == 2)
            {
                if (boxes[buttons[6].box_index].click_status)
                {
                    button_drag = 2;
                }
                if (!clicking)
                {
                    button_drag = 0;
                    thread_Audio_volume_changed = 1;
                }
                set3[0].se_volume = (cpos.x/scale - 400) / 9;
                if (set3[0].se_volume < 0)
                {
                    set3[0].se_volume = 0;
                }
                else if (set3[0].se_volume > 100)
                {
                    set3[0].se_volume = 100;
                }
                lables[5].text = to_string(set3[0].se_volume);
                buttons[6].x1 = 400 + 9.0 * set3[0].se_volume - 25;
                buttons[6].x2 = 400 + 9.0 * set3[0].se_volume + 25;
                
                boxes[buttons[6].box_index].x1 = to_screen(buttons[6].x1);
                boxes[buttons[6].box_index].x2 = to_screen(buttons[6].x2);

            }
            
        }
        DrawCallNum++;

        

        if (boxes[buttons[0].box_index].clicked)
        {
            
            boxes[buttons[0].box_index].clicked = 0;
            page_index = 0;
            page_status = 0;
            return;
        }
        else if (boxes[buttons[1].box_index].clicked)
        {
            //writelog("dbg setting 1  "+to_string(buttons[0].box_index));
            setting_page_index = 1;
            page_status = 0;
        }
        else if (boxes[buttons[2].box_index].clicked)
        {
            
            setting_page_index = 2;
            page_status = 0;
        }
        else if (boxes[buttons[3].box_index].clicked)
        {
            
            setting_page_index = 3;
            page_status = 0;
        }
        return;
    }
    return;
}
void rend_start();
float quit_page_count = 0;
void rend_quit()
{
    if (!normal_quit)
    {
        return;
    }
    quit_page_count+= dursec;
    if (quit_page_count >= 1.0f)
    {
        quit_single = 1;
    }
    g_pBrushQuit->SetOpacity(quit_page_count);
    g_pD2DDeviceContext->FillRectangle(
        
            D2D1::RectF(0, 0, to_screen(1660), to_screen(960)),
        g_pBrushQuit
    );
    return;
}
float start_page_count = 0.2;
int start_page_stage = 0;
int tip_used_num = 0;
void rend_start()
{
    g_pD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::White));
    if (page_index == 1000)
    {
        //string str_start = "C/C++ X DirectX Project\n2D application framework";
        g_pD2DDeviceContext->DrawText(
            multiByteToWideChar("Z"),           // Text to render
            wcslen(multiByteToWideChar("Z")),       // Text length
            g_pTextFormatNormal,     // Text format
            D2D1::RectF(to_screen(690), to_screen(490), to_screen(890), to_screen(690)),    // The region of the window where the text will be rendered
            g_pBrushRed      // The brush used to draw the text
        );
        g_pD2DDeviceContext->DrawText(
            multiByteToWideChar("Z"),           // Text to render
            wcslen(multiByteToWideChar("Z")),       // Text length
            g_pTextFormatNormal,     // Text format
            D2D1::RectF(to_screen(700), to_screen(500), to_screen(900), to_screen(700)),    // The region of the window where the text will be rendered
            g_pBrushGreen      // The brush used to draw the text
        );
        g_pD2DDeviceContext->DrawText(
            multiByteToWideChar("Z"),           // Text to render
            wcslen(multiByteToWideChar("Z")),       // Text length
            g_pTextFormatNormal,     // Text format
            D2D1::RectF(to_screen(710), to_screen(510), to_screen(910), to_screen(710)),    // The region of the window where the text will be rendered
            g_pBrushLightBlue      // The brush used to draw the text
        );
        g_pD2DDeviceContext->DrawText(
            multiByteToWideChar("C/C++ X DirectX Project\n2D application framework"),           // Text to render
            wcslen(multiByteToWideChar("C/C++ X DirectX Project\n2D application framework")),       // Text length
            g_pTextFormat,     // Text format
            textLayoutRect,    // The region of the window where the text will be rendered
            g_pBrushBlue      // The brush used to draw the text
        );
    }
    else if (page_index == 1001)
    {
        if (start_page_stage == 0)
        {
            thread_IO_request_read_config = 1;
            thread_IO_request_read_highscore = 1;
            start_page_stage = 1;
            start_page_count = 0.2;
        }
        else if (start_page_stage == 1)
        {
            
            start_page_count += dursec;
            if (set1[0].tips)
            {
                if (start_page_count >= 1.5)
                {
                    start_page_stage = 2;
                    //start_page_count = 2.0;
                }
                DrawBitmap_1(g_pD2DBimtapUI[15], D2D1::RectF(
                    600,
                    200,
                    1000,
                    650
                ), start_page_count);
            }
            else
            {
                if (start_page_count >= 1.2)
                {
                    start_page_stage = 2;
                    //start_page_count = 2.0;
                }
                DrawBitmap_1(g_pD2DBimtapUI[15], D2D1::RectF(
                    600,
                    200,
                    1000,
                    650
                ), start_page_count);
            }
            
            
        }
        else if (start_page_stage == 2)
        {
            start_page_count -= dursec*0.8;
            if (start_page_count <= -0.1)
            {
                page_index = 0;
                return;
            }

            DrawBitmap_1(g_pD2DBimtapUI[15], D2D1::RectF(
                600,
                200,
                1000,
                650
            ), start_page_count);
        }
        if (set1[0].tips)
        {
            g_pD2DDeviceContext->DrawText(
                multiByteToWideChar(tips[tip_used_num]),           // Text to render
                wcslen(multiByteToWideChar(tips[tip_used_num])),       // Text length
                g_pTextFormat,     // Text format
                D2D1::RectF(to_screen(800), to_screen(800), to_screen(1600), to_screen(890)),    // The region of the window where the text will be rendered
                g_pBrushBlack      // The brush used to draw the text
            );
        }
    }
    return;
}

void game_delete_single_save()
{

    string pathname = "./save/", fullfilename = "";
    pathname += usernameC;
    int flag = 0, result = 0;
    
    fullfilename = pathname;
    fullfilename += "/save";
    fullfilename += to_string(currentSave);
    fullfilename += ".dat";
    /*flag = _access(fullfilename.c_str(), 6);
    if (flag != 0)
    {
        writelog("Warning:IO error in delete_single_save()");
        writelog(fullfilename.c_str());
        thread_IO_request_delete_game2048 = -1;
        return;
    }*/
    writelog("try to delete save.");
    writelog(fullfilename.c_str());
    result=remove(fullfilename.c_str());
    if (result != 0)
    {
        writelog("Warning:IO error in delete_single_save()");
        writelog(fullfilename.c_str());
        thread_IO_request_delete_game = -1;
        return;
    }
    fullfilename = pathname;
    fullfilename += "/save";
    fullfilename += to_string(currentSave);
    fullfilename += ".check";
    result = remove(fullfilename.c_str());
    if (result != 0)
    {
        writelog("Warning:IO error in delete_single_save()");
        writelog(fullfilename.c_str());
        thread_IO_request_delete_game = -1;
        return;
    }
    thread_IO_request_delete_game = 0;
    return;
}
void game_record()
{
    string pathname = "./save/", fullfilename = "";
    pathname += usernameC;
    int lines = 0, result = 0,his_num=0;
    fullfilename = pathname;
    fullfilename += "/highscore.dat";
    string readbuf[64] = { "" }, writebuf[64] = { "" };
    //fileinit();
    
    if (!file_private_verify(fullfilename))
    {
        writelog("cert file failed!    "+ fullfilename);
        
        playeffectsound(6);
        playeffectsound(15);
        thread_IO_request_record_game = -1;
        return;
    }
    lines = ReadFile(fullfilename.c_str(), readbuf);
    if (lines > 0)
    {
        writelog("recording...");
        for (int i = 0; i < 64; i++)
        {
            if (readbuf[i] == "[user]")
            {
                if (readbuf[i + 1] != usernameC)
                {
                    playeffectsound(6);
                    playeffectsound(15);
                    thread_IO_request_record_game = -1;
                    writelog("record cert failed!(username)    "+ fullfilename);
                    
                    return;
                }
                writebuf[0] = "[user]";
                writebuf[1] = usernameC;
                
            }
            else if (readbuf[i] == "[mine_sum]")
            {
                writebuf[2] = "[mine_sum]";
                if (page_status == 103)
                {
                    if (set1[0].map_size == 1)
                    {
                        writebuf[3] = to_string(stoi(readbuf[i + 1]) + 10);
                    }
                    else if (set1[0].map_size == 2)
                    {
                        writebuf[3] = to_string(stoi(readbuf[i + 1]) + 40);
                    }
                    else if (set1[0].map_size == 3)
                    {
                        writebuf[3] = to_string(stoi(readbuf[i + 1]) + 99);
                    }
                }
                else
                {
                    writebuf[3] = readbuf[i + 1];
                }
                
            }
            else if (readbuf[i] == "[time_sum]")
            {
                writebuf[4] = "[time_sum]";
                writebuf[5] = to_string(stof(readbuf[i + 1]) + timebuf);
            }
            else if (readbuf[i] == "[win_num]")
            {
                writebuf[6] = "[win_num]";
                writebuf[7] = readbuf[i + 1];
                writebuf[8] = readbuf[i + 2];
                writebuf[9] = readbuf[i + 3];
                writebuf[10] = readbuf[i + 4];
                writebuf[11] = readbuf[i + 5];
                writebuf[12] = readbuf[i + 6];
                if (set1[0].map_size == 1)
                {
                    writebuf[10] = to_string(stoi(readbuf[i + 4]) + 1);
                }
                else if (set1[0].map_size == 2)
                {
                    writebuf[11] = to_string(stoi(readbuf[i + 5]) + 1);
                }
                else if (set1[0].map_size == 3)
                {
                    writebuf[12] = to_string(stoi(readbuf[i + 6]) + 1);
                }
                if (page_status == 103)
                {
                    if (set1[0].map_size == 1)
                    {
                        writebuf[7]= to_string(stoi(readbuf[i + 1]) + 1);
                    }
                    else if (set1[0].map_size == 2)
                    {
                        writebuf[8] = to_string(stoi(readbuf[i + 2]) + 1);
                    }
                    else if (set1[0].map_size == 3)
                    {
                        writebuf[9] = to_string(stoi(readbuf[i + 3]) + 1);
                    }
                }
                
            }
            else if (readbuf[i] == "[highscore]")
            {
                writebuf[13] = "[highscore]";
                writebuf[14] = readbuf[i + 1];
                writebuf[15] = readbuf[i + 2];
                writebuf[16] = readbuf[i + 3];
                writebuf[17] = readbuf[i + 4];
                writebuf[18] = readbuf[i + 5];
                writebuf[19] = readbuf[i + 6];
                if (page_status == 103)
                {
                    if (set1[0].map_size == 1 && timebuf < stof(readbuf[i + 1]))
                    {
                        writebuf[14] = to_string(timebuf);
                        writebuf[17] = getTimeStr();
                    }
                    else if (set1[0].map_size == 2 && timebuf < stof(readbuf[i + 2]))
                    {
                        writebuf[15] = to_string(timebuf);
                        writebuf[18] = getTimeStr();
                    }
                    else if (set1[0].map_size == 3 && timebuf < stof(readbuf[i + 3]))
                    {
                        writebuf[16] = to_string(timebuf);
                        writebuf[19] = getTimeStr();
                    }
                }
                
                
            }
            else if (readbuf[i] == "[date]")
            {
                writebuf[20] = "[date]";
                writebuf[21] = readbuf[i+1];
            }
            
            WriteFile(fullfilename.c_str(), writebuf);
            certfile(fullfilename);
        }
    }
    else
    {
        playeffectsound(6);
        playeffectsound(15);
        thread_IO_request_record_game = -1;
        writelog("record read failed!");
        writelog(fullfilename.c_str());
        return;
    }
    cleanStrBuff(readbuf);
    cleanStrBuff(writebuf);
    writelog("history debug.");
    for (int i = 1; i < 9999; i++)
    {
        
        
        fullfilename = pathname;
        fullfilename += "/history";
        fullfilename += to_string(i);
        fullfilename += ".dat";
        
        if (_access(fullfilename.c_str(), 4) != 0)
        {
            writelog(fullfilename);
            fullfilename = pathname;
            fullfilename += "/history";
            if (i == 1)
            {
                fullfilename += to_string(i);
            }
            else
            {
                fullfilename += to_string(i - 1);
                
            }
            fullfilename += ".dat";
            
            lines = ReadFile(fullfilename.c_str(), readbuf);
            
            if (lines >= 38||lines <= 1)
            {
                fullfilename = pathname;
                fullfilename += "/history";
                fullfilename += to_string(i);
                fullfilename += ".dat";
                writebuf[0] = "[user]";
                writebuf[1] = usernameC;
                writebuf[2] = "[date]";
                writebuf[3] = getTimeStr();
                writebuf[4] = "[mapsize]";
                writebuf[5] = to_string(set1[0].map_size);
                writebuf[6] = "[flag]";
                writebuf[7] = to_string(flagbuf);
                writebuf[8] = "[opened_cube]";
                writebuf[9] = to_string(opened_cubenum_buf);
                writebuf[10] = "[time]";
                writebuf[11] = to_string(timebuf);
                writebuf[12] = "[gamedata]";

                writebuf[13] = "";
                for (int i = 0; i < cubenum_h * cubenum_w; i++)
                {
                    writebuf[13] += to_string(savebuf[i]);
                }
                
                WriteFile(fullfilename.c_str(), writebuf);
                certfile(fullfilename);
                writelog("create new history file.");
                writelog(fullfilename);
            }
            else
            {
                if (!file_private_verify(fullfilename))
                {
                    playeffectsound(6);
                    writelog("cert file failed!");
                    writelog(fullfilename.c_str());
                    playeffectsound(15);
                    thread_IO_request_record_game = -1;
                    return;
                }
                for (int i = 0; i < 64; i++)
                {
                    if (readbuf[i] == "[user]")
                    {
                        if (readbuf[i + 1] != usernameC)
                        {
                            playeffectsound(6);
                            playeffectsound(15);
                            thread_IO_request_record_game = -1;
                            writelog("history cert failed!");
                            writelog(fullfilename.c_str());
                            return;
                        }
                    }
                    writebuf[0] = "[user]";
                    writebuf[1] = usernameC;
                    if (readbuf[i] == "[date]")
                    {
                        for (int j = 1; j < 6; j++)
                        {
                            if (readbuf[i+j] == "[mapsize]")
                            {
                                his_num = j - 1;
                                break;
                            }
                        }
                    }
                    

                }
                writelog("add history.");
                writelog(fullfilename);
                writebuf[2] = "[date]";
                for (int j = 1; j <= his_num; j++)
                {
                    writebuf[2 + j] = readbuf[2 + j];
                }
                writebuf[3 + his_num] = getTimeStr();
                writebuf[4 + his_num] = "[mapsize]";
                for (int j = 1; j <= his_num; j++)
                {
                    writebuf[4 + his_num + j] = readbuf[3 + his_num+j];
                }
                writebuf[5 + 2 * his_num] = to_string(set1[0].map_size);
                writebuf[6 + 2*his_num] = "[flag]";
                for (int j = 1; j <= his_num; j++)
                {
                    writebuf[6 + 2 * his_num + j] = readbuf[4 + 2 * his_num + j];
                }
                writebuf[7 + 3 * his_num] = to_string(flagbuf);
                writebuf[8 + 3 * his_num] = "[opened_cube]";
                for (int j = 1; j <= his_num; j++)
                {
                    writebuf[8 + 3 * his_num + j] = readbuf[5 + 3 * his_num + j];
                }
                writebuf[9 + 4 * his_num] = to_string(opened_cubenum_buf);
                writebuf[10 + 4 * his_num] = "[time]";
                for (int j = 1; j <= his_num; j++)
                {
                    writebuf[10 + 4 * his_num + j] = readbuf[6 + 4 * his_num + j];
                }
                writebuf[11 + 5 * his_num] = to_string(timebuf);
                writebuf[12 + 5 * his_num] = "[gamedata]";
                for (int j = 1; j <= his_num; j++)
                {
                    writebuf[12 + 5 * his_num + j] = readbuf[7 + 5 * his_num + j];
                }
                writebuf[13 + 6 * his_num] = "";
                int count = 0;
                for (int i = 0; i < cubenum_h * cubenum_w; i++)
                {
                    if (to_string(savebuf[i]) == "1")
                    {
                        count++;
                    }
                    writebuf[13 + 6 * his_num] += to_string(savebuf[i]);
                }
                writelog("dbg writehis " + to_string(count));
                WriteFile(fullfilename.c_str(), writebuf);
                certfile(fullfilename);
            }
            break;
        }
        
    }
    

    thread_IO_request_record_game = 0;
    return;
}
void game_read_highscore()
{
    if (thread_IO_request_read_highscore <= 0)
    {
        return;
    }
    string pathname = "./save/", fullfilename = "";
    pathname += usernameC;
    int lines = 0, result = 0, his_num = 0;
    fullfilename = pathname;
    fullfilename += "/highscore.dat";

    if (!file_private_verify(fullfilename))
    {
        playeffectsound(6);
        writelog("cert file failed!");
        writelog(fullfilename.c_str());
        thread_IO_request_read_highscore = -1;
        return;
    }
    string readbuf[64] = { "" };
    lines = ReadFile(fullfilename.c_str(), readbuf);
    if (lines > 0)
    {
        for (int i = 0; i < 64; i++)
        {
            if (readbuf[i] == "[user]")
            {
                if (readbuf[i + 1] != usernameC)
                {
                    playeffectsound(6);
                    playeffectsound(15);
                    thread_IO_request_read_highscore = -1;
                    writelog("history cert failed!");
                    writelog(fullfilename.c_str());
                    return;
                }
            }
            else if (readbuf[i] == "[mine_sum]")
            {
                highscore[0].mine_sum = readbuf[i + 1];
            }
            else if (readbuf[i] == "[time_sum]")
            {
                highscore[0].time = getTimeDigit(stof(readbuf[i + 1])) ;
            }
            else if (readbuf[i] == "[win_num]")
            {
                highscore[1].win_num = readbuf[i + 1];
                highscore[2].win_num = readbuf[i + 2];
                highscore[3].win_num = readbuf[i + 3];
                highscore[1].totle_num = readbuf[i + 4];
                highscore[2].totle_num = readbuf[i + 5];
                highscore[3].totle_num = readbuf[i + 6];
            }
            else if (readbuf[i] == "[highscore]")
            {
                //highscore[0].score = readbuf[i + 1];
                //highscore[0].step = readbuf[i + 2];
                highscore[1].time = getTimeDigit(stof(readbuf[i + 1]));
                highscore[1].date = readbuf[i + 4];
                highscore[2].time = getTimeDigit(stof(readbuf[i + 2]));
                highscore[2].date = readbuf[i + 5];
                highscore[3].time = getTimeDigit(stof(readbuf[i + 3]));
                highscore[3].date = readbuf[i + 6];

            }
            else if (readbuf[i] == "[date]")
            {
                highscore[0].date = readbuf[i + 1];
            }
        }
    }
    else
    {
        playeffectsound(6);
        writelog("read highscore failed!");
        thread_IO_request_read_highscore = -1;
    }
    thread_IO_request_read_highscore = 0;
    return;
}
void game_read_history()
{
    if (thread_IO_request_read_history <= 0)
    {
        return;
    }
    string pathname = "./save/", fullfilename = "";
    pathname += usernameC;
    int lines = 0, result = 0, his_num = 0;
    fullfilename = pathname;
    fullfilename += "/history";
    fullfilename += to_string(thread_IO_request_read_history);
    fullfilename += ".dat";
    string readbuf[64] = { "" };
    if (_access(fullfilename.c_str(), 4) != 0)
    {
        playeffectsound(7);
        thread_IO_request_read_history = -2;

        return;
    }
    lines = ReadFile(fullfilename.c_str(), readbuf);
    if (lines > 0)
    {
        if (!file_private_verify(fullfilename))
        {
            playeffectsound(6);
            writelog("cert file failed!");
            writelog(fullfilename.c_str());
            thread_IO_request_read_history = -1;
            playeffectsound(15);
            return;
        }
        for (int i = 0; i < 64; i++)
        {
            if (readbuf[i] == "[user]")
            {
                if (readbuf[i + 1] != usernameC)
                {
                    playeffectsound(6);
                    playeffectsound(15);
                    thread_IO_request_read_history = -1;
                    writelog("history cert failed!");
                    writelog(fullfilename.c_str());
                    return;
                }
            }
            
                if (readbuf[i] == "[date]")
                {
                    for (int j = 1; j < 7; j++)
                    {
                        if (readbuf[i + j] == "[mapsize]")
                        {
                            his_num = j - 1;
                            break;
                        }
                    }
                }
            
        }
        for (int i = 0; i < his_num; i++)
        {
            history_buf[i].active = 1;
        }
        for (int i = his_num; i < 5; i++)
        {
            history_buf[i].active = 0;
        }
        for (int i = 0; i < 64; i++)
        {
            if (readbuf[i] == "[date]")
            {
                for (int j = 1; j <= his_num; j++)
                {
                    history_buf[his_num - j].date = readbuf[i + j];
                }
            }
            else if (readbuf[i] == "[mapsize]")
            {
                for (int j = 1; j <= his_num; j++)
                {
                    history_buf[his_num - j].mapsize = readbuf[i + j];
                }
            }
            else if (readbuf[i] == "[flag]")
            {
                for (int j = 1; j <= his_num; j++)
                {
                    history_buf[his_num - j].flag = readbuf[i + j];
                }
            }
            else if (readbuf[i] == "[time]")
            {
                for (int j = 1; j <= his_num; j++)
                {
                    history_buf[his_num - j].time = readbuf[i + j];
                }
            }
            else if (readbuf[i] == "[opened_cube]")
            {
                for (int j = 1; j <= his_num; j++)
                {
                    history_buf[his_num - j].opened_cube = readbuf[i + j];
                }
            }
            else if (readbuf[i] == "[gamedata]")
            {
                for (int j = 1; j <= his_num; j++)
                {
                    history_buf[his_num - j].gamedata = readbuf[i + j];
                }
            }
        }
    }
    else
    {
        
        thread_IO_request_read_history = -1;
        playeffectsound(15);
        writelog("file corrupted! " + fullfilename);
        return;
    }
    
    thread_IO_request_read_history = 0;
    return;
}
int savenum = 0;
int button_pointer[11] = { 0 };
bool save_isloading = 0, save_succeed_notice = 0;
bool overwriteWarning=0;
void page_game_save()
{
    string strbuf = "";
    float timebuf2 = 0;
    if (last_page_index == 2&& time_used>0.2)
    {
        time_used += dursec;
    }
    if (page_status == 0)
    {
        InitPage(3);
        save_isloading = 0;
        savenum = 0;
        page_status = 1;
        overwriteWarning = 1;
    }
    else if (page_status == 1)
    {
        CreateButton(100, 40, 250, 90, ""/*lan[0].str_return*/, g_pBrushGreen, g_pBrushWhite, g_pBrushGreen, g_pD2DBimtapUI[10]);//0
        thread_IO_request_read_all_info = 1;
        page_status = 2;
    }
    else if (page_status == 2) 
    {
        g_pD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::LightGoldenrodYellow));
        if (!thread_IO_request_read_all_info)
        {
            page_status = 3;
        }
        g_pD2DDeviceContext->DrawText(
            multiByteToWideChar("Loading..."),           // Text to render
            wcslen(multiByteToWideChar("Loading...")),       // Text length
            g_pTextFormat,     // Text format
            textrbRect,    // The region of the window where the text will be rendered
            g_pBrushBlack      // The brush used to draw the text
        );
        DrawCallNum++;
        if (boxes[buttons[0].box_index].clicked)
        {
            boxes[buttons[0].box_index].clicked = 0;
            page_index = last_page_index;
            if (last_page_index == 0)
            {
                page_status = 0;
            }
            else if (last_page_index == 2)
            {
                InitPage(2);
                page_status = 1;
                last_page_index = 3;
            }

        }
    }
    else if (page_status == 3)
    {
        CreateLable(100, 790, 1500, 880, "", NULL, g_pBrushPurple, g_pBrushBlack, NULL, 0);//0
        if (save_succeed_notice)
        {
            lables[0].text = lan[0].save_succeed;
            lables[0].Brush3 = g_pBrushLightGreen;
            save_succeed_notice = 0;
            
        }
        for (int i = 0; i < 10; i++)
        {
            if (read_infos[i].active == 1)
            {
                savenum++;
                if (currentSave == 0)
                {
                    CreateButton(read_infos[i].rect.left, read_infos[i].rect.top, read_infos[i].rect.right, read_infos[i].rect.bottom, to_string(i + 1), g_pBrushGreen, g_pBrushWhite, g_pBrushGreen, g_pD2DBimtapUI[9]);//1-10/1

                    button_pointer[savenum] = i;
                }
                else if (currentSave == i + 1)
                {
                    

                    CreateButton(read_infos[i].rect.left, read_infos[i].rect.top, read_infos[i].rect.right, read_infos[i].rect.bottom, to_string(i + 1), g_pBrushGreen, g_pBrushWhite, g_pBrushRed, g_pD2DBimtapUI[9]);//1-10/1

                    button_pointer[savenum] = i;
                }
                else
                {
                    CreateLable(read_infos[i].rect.left, read_infos[i].rect.top, read_infos[i].rect.right, read_infos[i].rect.bottom, "Not\navailable", g_pBrushYellow, g_pBrushWhite, g_pBrushLightGray, g_pD2DBimtapUI[8], 4);
                }
                    strbuf = read_infos[i].opened_cube;
                    strbuf += " / ";
                    if (read_infos[i].mapsize == "1")
                    {
                        strbuf += "100";
                    }
                    else if (read_infos[i].mapsize == "2")
                    {
                        strbuf += "256";
                    }
                    else if (read_infos[i].mapsize == "3")
                    {
                        strbuf += "480";
                    }
                    strbuf += "\n";
                    strbuf += "\n";

                    CreateLable(read_infos[i].rect.left, read_infos[i].rect.top, read_infos[i].rect.right, read_infos[i].rect.bottom, strbuf, NULL, NULL, g_pBrushLightGreen, NULL, 1);//1,4,7...
                    strbuf = "\n";
                    strbuf += read_infos[i].flag;
                    strbuf += " / ";
                    if (read_infos[i].mapsize == "1")
                    {
                        strbuf += "10";
                    }
                    else if (read_infos[i].mapsize == "2")
                    {
                        strbuf += "40";
                    }
                    else if (read_infos[i].mapsize == "3")
                    {
                        strbuf += "99";
                    }
                    strbuf += "\n";

                    CreateLable(read_infos[i].rect.left, read_infos[i].rect.top, read_infos[i].rect.right, read_infos[i].rect.bottom, strbuf, NULL, NULL, g_pBrushYellow, NULL, 1);//2,5,8...
                    strbuf = "\n";
                    strbuf += "\n";
                    strbuf += getTimeDigit(stof(read_infos[i].time));
                    

                    CreateLable(read_infos[i].rect.left, read_infos[i].rect.top, read_infos[i].rect.right, read_infos[i].rect.bottom, strbuf, NULL, NULL, g_pBrushLightBlue, NULL, 1);//3,6,9...
                
                
            }
            else if (read_infos[i].active == 0)
            {
                if (currentSave == 0)
                {
                    CreateButton(read_infos[i].rect.left, read_infos[i].rect.top, read_infos[i].rect.right, read_infos[i].rect.bottom, to_string(i + 1), g_pBrushGreen, g_pBrushWhite, g_pBrushGreen, g_pD2DBimtapUI[8]);//1-10/1

                    button_pointer[savenum] = i;
                }
                else
                {
                    CreateLable(read_infos[i].rect.left, read_infos[i].rect.top, read_infos[i].rect.right, read_infos[i].rect.bottom, "Empty", g_pBrushGreen, g_pBrushWhite, g_pBrushGray, g_pD2DBimtapUI[8], 1);
                }
                
                //CreateButton(read_infos[i].rect.left, read_infos[i].rect.top, read_infos[i].rect.right, read_infos[i].rect.bottom, "Empty", g_pBrushGreen, g_pBrushWhite, g_pBrushGray, g_pD2DBimtapUI[8]);//1-10
            }
            else if (read_infos[i].active == 2)
            {
                if (currentSave == 0)
                {
                    CreateButton(read_infos[i].rect.left, read_infos[i].rect.top, read_infos[i].rect.right, read_infos[i].rect.bottom, "Corrupted", g_pBrushGreen, g_pBrushWhite, g_pBrushGreen, g_pD2DBimtapUI[7]);//1-10/1

                    button_pointer[savenum] = i;
                }
                else
                {
                    CreateLable(read_infos[i].rect.left, read_infos[i].rect.top, read_infos[i].rect.right, read_infos[i].rect.bottom, "Corrupted", g_pBrushGreen, g_pBrushWhite, g_pBrushGray, g_pD2DBimtapUI[7], 1);
                }
                
                //CreateButton(read_infos[i].rect.left, read_infos[i].rect.top, read_infos[i].rect.right, read_infos[i].rect.bottom, "Corrupted", g_pBrushGreen, g_pBrushWhite, g_pBrushGray, g_pD2DBimtapUI[6]);//1-10
            }

        }
        page_status = 4;
    }
    else if (page_status == 4)
    {
    g_pD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::LightGoldenrodYellow));
    if (boxes[buttons[0].box_index].clicked)
    {
        boxes[buttons[0].box_index].clicked = 0;
        page_index = last_page_index;
        if (last_page_index == 0)
        {
            page_status = 0;
        }
        else if (last_page_index == 2)
        {
            InitPage(2);
            page_status = 1;
            last_page_index = 3;
        }

    }
    if (!save_isloading)
    {
        if (currentSave != 0)
        {
            if (boxes[buttons[1].box_index].clicked)
            {
                if (overwriteWarning)
                {
                    lables[0].text = lan[0].save_overwrite_warning;
                    lables[0].Brush3 = g_pBrushBlack;
                    overwriteWarning = 0;
                }
                else
                {
                    writelog("saving...");

                    lables[0].text = "saving...";
                    lables[0].Brush3 = g_pBrushBlack;
                    save_isloading = 1;
                    saveMS(currentSave);
                    
                }
                boxes[buttons[1].box_index].clicked = 0;
            }
        }
        else
        {
            for (int i = 1; i <= 10; i++)
            {

                if (boxes[buttons[i].box_index].clicked)
                {
                    boxes[buttons[i].box_index].clicked = 0;
                    if (overwriteWarning&& read_infos[i-1].active==1)
                    {
                        lables[0].text = lan[0].save_overwrite_warning;
                        lables[0].Brush3 = g_pBrushBlack;
                        overwriteWarning = 0;

                    }
                    else
                    {
                        writelog("saving...");

                        lables[0].text = "saving...";
                        lables[0].Brush3 = g_pBrushBlack;
                        save_isloading = 1;
                        saveMS(i);

                        
                        break;
                    }
                    

                }
            }
        }
        
    }
    else
    {

        if (thread_IO_request_save_gameMS == 0)
        {
            save_succeed_notice = 1;
            playeffectsound(3);
            save_isloading = 0;
            page_status = 0;
        }
        else if (thread_IO_request_save_gameMS == -1)
        {
            lables[0].text = lan[0].save_failed;
            lables[0].Brush3 = g_pBrushRed;
            save_isloading = 0;
            thread_IO_request_save_gameMS = 0;

        }
    }
    }
}

void page_game_load()
{
    string strbuf = "";
    float timebuf2 = 0;
    static bool loadWarning = 0;
    //int pointer = 0;
    if (last_page_index == 2 && time_used > 0.2)
    {
        time_used += dursec;
    }
    if (page_status == 0)
    {
        InitPage(4);
        save_isloading = 0;
        savenum = 0;
        page_status = 1;
        loadWarning = 1;
    }
    else if (page_status == 1)
    {
        CreateButton(100, 40, 250, 90, ""/*lan[0].str_return*/, g_pBrushGreen, g_pBrushWhite, g_pBrushGreen, g_pD2DBimtapUI[10]);//0
        thread_IO_request_read_all_info = 1;
        page_status = 2;
    }
    else if (page_status == 2)
    {
        g_pD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::LightGoldenrodYellow));
        if (!thread_IO_request_read_all_info)
        {
            page_status = 3;
        }
        g_pD2DDeviceContext->DrawText(
            multiByteToWideChar("Loading..."),           // Text to render
            wcslen(multiByteToWideChar("Loading...")),       // Text length
            g_pTextFormat,     // Text format
            textrbRect,    // The region of the window where the text will be rendered
            g_pBrushBlack      // The brush used to draw the text
        );
        if (boxes[buttons[0].box_index].clicked)
        {
            boxes[buttons[0].box_index].clicked = 0;
            page_index = last_page_index;
            if (last_page_index == 0)
            {
                page_status = 0;
            }
            else if (last_page_index == 2)
            {
                InitPage(2);
                page_status = 1;
                last_page_index = 4;
            }
            
        }
    }
    else if (page_status == 3)
    {
        CreateLable(100, 790, 1500, 880, "", NULL, g_pBrushPurple, g_pBrushBlack, NULL, 0);//0
        for (int i = 0; i < 10; i++)
        {
            if (read_infos[i].active==1)
            {
                savenum++;
                
                CreateButton(read_infos[i].rect.left, read_infos[i].rect.top, read_infos[i].rect.right, read_infos[i].rect.bottom, to_string(i+1), g_pBrushGreen, g_pBrushWhite, g_pBrushGray, g_pD2DBimtapUI[9]);//1-10
                
                button_pointer[savenum] = i;
                strbuf = read_infos[i].opened_cube;
                strbuf += " / ";
                if (read_infos[i].mapsize == "1")
                {
                    strbuf += "90";
                }
                else if (read_infos[i].mapsize == "2")
                {
                    strbuf += "216";
                }
                else if (read_infos[i].mapsize == "3")
                {
                    strbuf += "381";
                }
                strbuf += "\n";
                strbuf += "\n";

                CreateLable(read_infos[i].rect.left, read_infos[i].rect.top, read_infos[i].rect.right, read_infos[i].rect.bottom, strbuf, NULL, NULL, g_pBrushLightGreen, NULL, 1);//1,4,7...
                strbuf = "\n";
                strbuf += read_infos[i].flag;
                strbuf += " / ";
                if (read_infos[i].mapsize == "1")
                {
                    strbuf += "10";
                }
                else if (read_infos[i].mapsize == "2")
                {
                    strbuf += "40";
                }
                else if (read_infos[i].mapsize == "3")
                {
                    strbuf += "99";
                }
                strbuf += "\n";
                //strbuf = read_infos[i].score;
                //strbuf += "\n";
                //strbuf += "\n";
                ////writelog(strbuf.c_str());
                //CreateLable(read_infos[i].rect.left, read_infos[i].rect.top, read_infos[i].rect.right, read_infos[i].rect.bottom, strbuf, NULL, NULL, g_pBrushLightGreen, NULL, 1);//1,4,7...
                //strbuf = "\n";
                //strbuf += read_infos[i].step;
                //strbuf += "\n";
                ////writelog(strbuf.c_str());
                CreateLable(read_infos[i].rect.left, read_infos[i].rect.top, read_infos[i].rect.right, read_infos[i].rect.bottom, strbuf, NULL, NULL, g_pBrushYellow, NULL, 1);//2,5,8...
                
                strbuf = "\n";
                strbuf += "\n";
                strbuf += getTimeDigit(stof(read_infos[i].time));
                
                //strbuf += read_infos[i].time.substr(0,5);
                //writelog(strbuf.c_str());
                CreateLable(read_infos[i].rect.left, read_infos[i].rect.top, read_infos[i].rect.right, read_infos[i].rect.bottom, strbuf, NULL, NULL, g_pBrushLightBlue, NULL, 1);//3,6,9...
                //continue;
            }
            else if (read_infos[i].active == 0)
            {
                /*strbuf = "Score ";
                strbuf += read_infos[i].score;
                strbuf += "\nStep ";
                strbuf += read_infos[i].step;
                strbuf += "\nTime ";
                strbuf += read_infos[i].time;*/
                CreateLable(read_infos[i].rect.left, read_infos[i].rect.top, read_infos[i].rect.right, read_infos[i].rect.bottom,"No Info", g_pBrushYellow, g_pBrushWhite, g_pBrushGray, g_pD2DBimtapUI[8], 4);
            }
            else if (read_infos[i].active == 2)
            {
                CreateLable(read_infos[i].rect.left, read_infos[i].rect.top, read_infos[i].rect.right, read_infos[i].rect.bottom, "Not\navailable", g_pBrushYellow, g_pBrushWhite, g_pBrushGray, g_pD2DBimtapUI[7], 4);
            }

        }
        //writelog(to_string(savenum));
        page_status = 4;
    }
    else if (page_status == 4)
    {
        g_pD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::LightGoldenrodYellow));
        if (boxes[buttons[0].box_index].clicked)
        {
            boxes[buttons[0].box_index].clicked = 0;
            page_index = last_page_index;
            if (last_page_index == 0)
            {
                page_status = 0;
            }
            else if (last_page_index == 2)
            {
                InitPage(2);
                page_status = 1;
                last_page_index = 4;
            }

        }
        if (!save_isloading)
        {
            /*if (savenum == 0)
            {
                writelog("11");
            }*/
            //writelog(to_string(savenum));
            for (int i = 1; i <= savenum; i++)
            {
                //writelog(to_string(i));
                if (boxes[buttons[i].box_index].clicked)
                {
                    boxes[buttons[i].box_index].clicked = 0;
                    if (loadWarning&&last_page_index==2)
                    {
                        lables[0].text = lan[0].load_warning;
                        lables[0].Brush3 = g_pBrushBlack;
                        loadWarning = 0;
                    }
                    else
                    {
                        writelog("loading save...");
                        //writelog(to_string(button_pointer[i]));
                        lables[0].text = "loading save...";
                        lables[0].Brush3 = g_pBrushBlack;
                        save_isloading = 1;
                        thread_IO_request_load_gameMS = button_pointer[i] + 1;
                        
                        break;
                    }
                    

                }
            }
        }
        else
        {
            
            if (thread_IO_request_load_gameMS == 0)
            {
                playeffectsound(4);
                page_index = 2;
                InitPage(2);
                //init2048();
                //claenStage();
                game_mode = 1;
                //initMS();
                page_status = 0;
                loadMS();
                last_page_index = 4;
            }
            else if(thread_IO_request_load_gameMS==-1)
            {
                lables[0].text = lan[0].load_failed;
                lables[0].Brush3 = g_pBrushRed;
                save_isloading = 0;
                thread_IO_request_load_gameMS = 0;
                
            }
        }
        
    }
    return;
}



void rend_page_history()
{
    static bool rendEndOfList = 0;
    static int currentPage = 1;
    static bool reading = 0;
    g_pD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::FloralWhite));
    if (page_status == 0)
    {
        InitPage(5);
        rendEndOfList = 0;
        currentPage = 1;
        page_status = 1;
        thread_IO_request_read_highscore = 1;
    }
    else if (page_status == 1)
    {
        CreateButton(100, 40, 250, 90, ""/*lan[0].str_return*/, g_pBrushGreen, g_pBrushWhite, g_pBrushGreen, g_pD2DBimtapUI[10]);//0
        CreateButton(300, 820, 500, 890, lan[0].lastpage, g_pBrushGreen, g_pBrushWhite, g_pBrushGreen, NULL);//1
        CreateButton(1100, 820, 1300, 890, lan[0].nextpage, g_pBrushGreen, g_pBrushWhite, g_pBrushGreen, NULL);//2
        CreateButton(1350, 40, 1500, 90, lan[0].highscore, g_pBrushGreen, g_pBrushYellow, g_pBrushGreen, NULL);//3
        CreateLable(600, 800, 1000, 880, "Page 1", NULL, g_pBrushPurple, g_pBrushBlack, NULL, 4);//0
        CreateLable(300, 160, 1300, 270, "", g_pBrushPurple, g_pBrushLightBlue, g_pBrushBlack, NULL, 1);//1
        CreateLable(300, 290, 1300, 400, "", g_pBrushPurple, g_pBrushLightBlue, g_pBrushBlack, NULL, 1);//2
        CreateLable(300, 420, 1300, 530, "", g_pBrushPurple, g_pBrushLightBlue, g_pBrushBlack, NULL, 1);//3
        CreateLable(300, 550, 1300, 660, "", g_pBrushPurple, g_pBrushLightBlue, g_pBrushBlack, NULL, 1);//4
        CreateLable(300, 680, 1300, 790, "", g_pBrushPurple, g_pBrushLightBlue, g_pBrushBlack, NULL, 1);//5
        CreateLable(280, 160, 650, 270, "", NULL, NULL, g_pBrushBlack, NULL, 1);//1 date*2
        CreateLable(280, 290, 650, 400, "", NULL, NULL, g_pBrushBlack, NULL, 1);//2
        CreateLable(280, 420, 650, 530, "", NULL, NULL, g_pBrushBlack, NULL, 1);//3
        CreateLable(280, 550, 650, 660, "", NULL, NULL, g_pBrushBlack, NULL, 1);//4
        CreateLable(280, 680, 650, 790, "", NULL, NULL, g_pBrushBlack, NULL, 1);//5
        CreateLable(650, 160, 850, 270, "", NULL, NULL, g_pBrushGreen, NULL, 4);//1 opened cube*3
        CreateLable(650, 290, 850, 400, "", NULL, NULL, g_pBrushGreen, NULL, 4);//2
        CreateLable(650, 420, 850, 530, "", NULL, NULL, g_pBrushGreen, NULL, 4);//3
        CreateLable(650, 550, 850, 660, "", NULL, NULL, g_pBrushGreen, NULL, 4);//4
        CreateLable(650, 680, 850, 790, "", NULL, NULL, g_pBrushGreen, NULL, 4);//5
        CreateLable(900, 160, 1050, 270, "", NULL, NULL, g_pBrushBrown, NULL, 4);//1 flag*4
        CreateLable(900, 290, 1050, 400, "", NULL, NULL, g_pBrushBrown, NULL, 4);//2
        CreateLable(900, 420, 1050, 530, "", NULL, NULL, g_pBrushBrown, NULL, 4);//3
        CreateLable(900, 550, 1050, 660, "", NULL, NULL, g_pBrushBrown, NULL, 4);//4
        CreateLable(900, 680, 1050, 790, "", NULL, NULL, g_pBrushBrown, NULL, 4);//5
        CreateLable(1100, 160, 1300, 270, "", NULL, NULL, g_pBrushBlue, NULL, 4);//1 time*5
        CreateLable(1100, 290, 1300, 400, "", NULL, NULL, g_pBrushBlue, NULL, 4);//2
        CreateLable(1100, 420, 1300, 530, "", NULL, NULL, g_pBrushBlue, NULL, 4);//3
        CreateLable(1100, 550, 1300, 660, "", NULL, NULL, g_pBrushBlue, NULL, 4);//4
        CreateLable(1100, 680, 1300, 790, "", NULL, NULL, g_pBrushBlue, NULL, 4);//5
        
        CreateLable(600, 50, 1000, 150, usernameC, NULL, NULL, g_pBrushBlack, NULL, 5);//26

        thread_IO_request_read_history = 1;
        reading = 1;
        page_status = 2;
        //debug
        if (stof(highscore[3].time) < 180&& stof(highscore[3].time)>1)
        {
            CreateAnimation(3, 600, 250);
        }
        else if (stof(highscore[3].time) < 360 && stof(highscore[3].time) > 1)
        {
            CreateAnimation(4, 600, 250);
        }
        else if (stof(highscore[3].time) > 1)
        {
            CreateAnimation(2, 600, 250);
        }
        else
        {
            CreateAnimation(1, 600, 250);
        }
    }
    else if (page_status == 2)
    {
        if (boxes[buttons[0].box_index].clicked)
        {
            page_index = 0;
            page_status = 0;
        }
        if (boxes[buttons[1].box_index].clicked)
        {
            boxes[buttons[1].box_index].clicked = 0;
            rendEndOfList = 0;
            if (currentPage > 1)
            {
                currentPage--;
                thread_IO_request_read_history = currentPage;
                reading = 1;
                for (int i = 6; i < 26; i++)
                {
                    lables[i].text = "";
                }
                lables[0].text = "Page ";
                lables[0].text += to_string(currentPage);
                
            }
            
        }
        if (boxes[buttons[2].box_index].clicked)
        {
            boxes[buttons[2].box_index].clicked = 0;
            currentPage++;
            thread_IO_request_read_history = currentPage;
            reading = 1;
            for (int i = 6; i < 26; i++)
            {
                lables[i].text = "";

            }
            lables[0].text = "Page ";
            lables[0].text += to_string(currentPage);
            rendEndOfList = 0;
        }
        if (boxes[buttons[3].box_index].clicked)
        {
            
            page_index = 6;
            page_status = 0;
        }
        for (int K = 0; K < 5; K++)
        {
            if (boxes[buttons[4 + K].box_index].clicked&& history_buf[K].active)
            {
                writelog("attemp to load history " + to_string(currentPage) + "-"+ to_string(K+1) + ".");
                if (history_buf[K].mapsize == "1")
                {
                    set1[0].map_size = 1;
                    
                }
                else if (history_buf[K].mapsize == "2")
                {
                    set1[0].map_size = 2;
                }
                else if (history_buf[K].mapsize == "3")
                {
                    set1[0].map_size = 3;
                }
                initMS();
                int count = 0;
                //read historybuf->savebuf

                for (int i = 1; i <= cubenum_w; i++)
                {
                    for (int j = 1; j <= cubenum_h; j++)
                    {
                        cubes[i][j].value = 0;
                        cubes_g[i][j].value = 0;
                        if (history_buf[K].gamedata.substr(count, 1) != "0" && history_buf[K].gamedata.substr(count, 1) != "1")
                        {
                            writelog("error in loading gamedata at " + to_string(count) + ": \"" + history_buf[K].gamedata.substr(count, 1) + "\"!");
                            Sleep(200);
                        }
                        savebuf[count] = stoi(history_buf[K].gamedata.substr(count, 1));
                        count++;
                    }
                }
                game_mode = 1; //replay mode
                page_index = 2;
                page_status = 0;
            }
        }
        
        
        if (reading)
        {
            if (thread_IO_request_read_history > 0)
            {
                g_pD2DDeviceContext->DrawText(
                    multiByteToWideChar("Loading..."),           // Text to render
                    wcslen(multiByteToWideChar("Loading...")),       // Text length
                    g_pTextFormat,     // Text format
                    textrbRect,    // The region of the window where the text will be rendered
                    g_pBrushBlack      // The brush used to draw the text
                );
                DrawCallNum++;
            }
            else if(thread_IO_request_read_history==-1)
            {
                
                g_pD2DDeviceContext->DrawText(
                    multiByteToWideChar("Load failed."),           // Text to render
                    wcslen(multiByteToWideChar("Load failed.")),       // Text length
                    g_pTextFormat,     // Text format
                    textrbRect,    // The region of the window where the text will be rendered
                    g_pBrushRed      // The brush used to draw the text
                );
                DrawCallNum++;
                thread_IO_request_read_history = 0;
                reading = 0;
            }
            else if (thread_IO_request_read_history == -2)
            {
                
                currentPage--;
                if (currentPage > 0)
                {
                    thread_IO_request_read_history = currentPage;
                }
                else
                {
                    currentPage = 1;
                    thread_IO_request_read_history = 0;
                }
                rendEndOfList = 1;
                reading = 1;
            }
            else
            {
                reading = 0;

                for (int i = 0; i <5; i++)
                {
                    if (history_buf[i].active)
                    {
                        lables[i + 6].text = history_buf[i].date;
                    }
                    else
                    {
                        lables[i + 6].text = "";
                    }
                    
                }
                for (int i = 0; i < 5; i++)
                {
                    if (history_buf[i].active)
                    {
                        lables[i + 11].text = history_buf[i].opened_cube;
                        lables[i + 11].text += "/";
                        if (history_buf[i].mapsize == "1")
                        {
                            lables[i + 11].text += "90";
                        }
                        else if (history_buf[i].mapsize == "2")
                        {
                            lables[i + 11].text += "216";
                        }
                        else if (history_buf[i].mapsize == "3")
                        {
                            lables[i + 11].text += "381";
                        }
                    }
                    else
                    {
                        lables[i + 11].text = "";
                    }
                    
                }
                for (int i = 0; i < 5; i++)
                {
                    if (history_buf[i].active)
                    {
                        lables[i + 16].text = history_buf[i].flag;
                        lables[i + 16].text += "/";
                        if (history_buf[i].mapsize == "1")
                        {
                            lables[i + 16].text += "10";
                        }
                        else if (history_buf[i].mapsize == "2")
                        {
                            lables[i + 16].text += "40";
                        }
                        else if (history_buf[i].mapsize == "3")
                        {
                            lables[i + 16].text += "99";
                        }
                    }
                    else
                    {
                        lables[i + 16].text = "";
                    }

                }
                for (int i = 0; i < 5; i++)
                {

                    if (history_buf[i].active)
                    {
                        lables[i + 21].text = getTimeDigit(stof(history_buf[i].time));
                    }
                    else
                    {
                        lables[i + 21].text = "";
                    }

                }
                for (int i = 0; i < 5; i++)
                {
                    if (history_buf[i].active)
                    {
                        CreateButton(1400, 160 + i * 130, 1500, 270 + i * 130, "Replay", g_pBrushLightBlue, g_pBrushLight, g_pBrushLightGreen, NULL); //4-8
                    }
                    else
                    {
                        buttons[i + 4].active = 0;
                    }
                }
                lables[0].text = "Page ";
                lables[0].text += to_string(currentPage);
            }
        }
        DrawSpriteSheet();
        rendButton();
        rendLable();
        for (int i = 0; i < 5; i++)
        {
            if (history_buf[i].active)
            {
                DrawBitmap_1(g_pD2DBimtapUI[21], D2D1::RectF(850, 130 * i + 180, 930, 130 * i + 260), 1.0f);
                DrawBitmap_1(g_pD2DBimtapUI[30], D2D1::RectF(1030, 130 * i + 180, 1110, 130 * i + 260), 1.0f);
            }
        }
        if (rendEndOfList)
        {
            g_pD2DDeviceContext->DrawText(
                multiByteToWideChar("End of list."),           // Text to render
                wcslen(multiByteToWideChar("End of list.")),       // Text length
                g_pTextFormat,     // Text format
                textrbRect,    // The region of the window where the text will be rendered
                g_pBrushRed      // The brush used to draw the text
            );
            DrawCallNum++;
        }
    }
    return;
}
bool hsloaded = 0;
bool notice_maxnum_not_enough = 0;BOOL fullsrcstatev = 0;


struct ed_string
{
    
    string ori = "";
    float time = 0;
    float load_char_time = 0;
    int charnum = 0;
    int font = 0;
    int pos = 0;
};
ed_string ed_strings[64];
void rend_page_highscore()
{

    static float ed_bg_black = 0, ed_time = 0, ed_bg_logo = 0, ed_bg_logo_posy = 0, ed_fg_white = 0;
    static int flow_speed=70;
    g_pD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::LightSeaGreen));
    if (page_status == 0)
    {
        InitPage(6);
        hsloaded = 0;
        ed_time = 0;
        ed_stage = 0;
        ed_bg_black = 0;
        ed_fg_white = 0;
        ed_bg_logo = 0;
        page_status = 1;
        notice_maxnum_not_enough = 0;
        ed_bg_logo_posy = 900;
        for (int i = 0; i < 64; i++)
        {
            ed_strings[i].charnum = 0;
            
            ed_strings[i].load_char_time = 0;
            
            ed_strings[i].time = 0;


        }
    }
    else if (page_status == 1)
    {
        CreateButton(100, 40, 250, 90, ""/*lan[0].str_return*/, g_pBrushGreen, g_pBrushWhite, g_pBrushGreen, g_pD2DBimtapUI[10]);//0
        CreateButton(1400, 40, 1500, 90, "Credit", g_pBrushYellow, g_pBrushWhite, g_pBrushPink, NULL);//1
        CreateLable(600, 50, 1000, 150, usernameC, NULL, NULL, g_pBrushBlack, NULL, 5);//0
        CreateLable(300, 160, 1300, 270, "", g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL, 1);//1
        CreateLable(300, 290, 1300, 400, /*lan[0].scoresum*/"", g_pBrushLightGreen, g_pBrushBlue, g_pBrushBlack, NULL, 1);//2
        CreateLable(300, 420, 1300, 530, /*lan[0].stepsum*/"", g_pBrushBrown, g_pBrushBlue, g_pBrushBlack, NULL, 1);//3
        CreateLable(300, 550, 1300, 660, /*lan[0].timesum*/"", g_pBrushLightBlue, g_pBrushBlue, g_pBrushBlack, NULL, 1);//4
        CreateLable(300, 680, 1300, 790, lan[0].accountTime, g_pBrushPurple, g_pBrushBlue, g_pBrushBlack, NULL, 1);//5
        CreateLable(490, 160, 600, 270, "", NULL, NULL, g_pBrushBlack, g_pD2DBimtapUI[15], 1);//6 pic
        CreateLable(640, 160, 800, 270, "", NULL, NULL, g_pBrushLightGreen, NULL, 1);//7 mine_num
        CreateLable(850, 160, 960, 270, "", NULL, NULL, g_pBrushBrown, g_pD2DBimtapUI[30], 1);//8 pic
        CreateLable(1000, 160, 1300, 270, "", NULL, NULL, g_pBrushLightBlue, NULL, 1);//9 time

        CreateLable(300, 290, 1300, 400, "", NULL, NULL, g_pBrushBlack, NULL, 1);//10
        CreateLable(300, 420, 1300, 530, "", NULL, NULL, g_pBrushBlack, NULL, 1);//11
        CreateLable(300, 550, 1300, 660, "", NULL, NULL, g_pBrushBlack, NULL, 1);//12
        CreateLable(700, 680, 1300, 790, "", NULL, NULL, g_pBrushBlack, NULL, 1);//13
        thread_IO_request_read_highscore = 1;
        page_status = 2;
    }
    else if(page_status == 2)
    {
        //DrawBitmap_1(g_pD2DBimtapUI[21], D2D1::RectF(850, 130 * i + 180, 930, 130 * i + 260), 1.0f);
        //DrawBitmap_1(g_pD2DBimtapUI[30], D2D1::RectF(1030, 130 * i + 180, 1110, 130 * i + 260), 1.0f);
        if (boxes[buttons[0].box_index].clicked)
        {
            page_index = 5;
            page_status = 0;
        }
        else if (boxes[buttons[1].box_index].clicked)
        {
            boxes[buttons[1].box_index].clicked = 0;
            if (stof(highscore[2].time) > 1|| stof(highscore[3].time) > 1)
            {
                writelog("Play credit.");
                page_status = 3;
                thread_Audio_target_music = 5;
                thread_Audio_switch_immediately = 1;
                playeffectsound(16);
                g_pSwapChain->GetFullscreenState(&fullsrcstatev, NULL);

                if (!fullsrcstatev)
                {
                    g_pSwapChain->SetFullscreenState(1, NULL);
                }
                
            }
            else
            {
                notice_maxnum_not_enough = 1;
            }
            
        }
        if (notice_maxnum_not_enough)
        {
            g_pD2DDeviceContext->DrawText(
                multiByteToWideChar(lan[0].notice_maxnum_not_enough),           // Text to render
                wcslen(multiByteToWideChar(lan[0].notice_maxnum_not_enough)),       // Text length
                g_pTextFormat,     // Text format
                D2D1::RectF(to_screen(1100), to_screen(800), to_screen(1550), to_screen(870)),    // The region of the window where the text will be rendered
                g_pBrushBlack      // The brush used to draw the text
            );
            DrawCallNum++;
        }
        if (thread_IO_request_read_highscore == 1)
        {
            g_pD2DDeviceContext->DrawText(
                multiByteToWideChar("Loading..."),           // Text to render
                wcslen(multiByteToWideChar("Loading...")),       // Text length
                g_pTextFormat,     // Text format
                textrbRect,    // The region of the window where the text will be rendered
                g_pBrushBlack      // The brush used to draw the text
            );
            DrawCallNum++;
        }
        else if (thread_IO_request_read_highscore == -1)
        {
            
            g_pD2DDeviceContext->DrawText(
                multiByteToWideChar("Load failed."),           // Text to render
                wcslen(multiByteToWideChar("Load failed.")),       // Text length
                g_pTextFormat,     // Text format
                textrbRect,    // The region of the window where the text will be rendered
                g_pBrushRed      // The brush used to draw the text
            );
            DrawCallNum++;
        }
        else
        {
            
            if (hsloaded == 0)
            {
                lables[6].text = "";
                lables[7].text = highscore[0].mine_sum;
                lables[8].text = /*highscore[0].win_num*/"";
                lables[9].text = highscore[0].time;
                lables[10].text = lan[0].easy;
                lables[11].text = lan[0].normal;
                lables[12].text = lan[0].hard;
                lables[10].text += "  clear: ";
                lables[11].text += "  clear: ";
                lables[12].text += "  clear: ";
                lables[10].text += highscore[1].win_num;
                lables[10].text += "/";
                lables[10].text += highscore[1].totle_num;
                lables[11].text += highscore[2].win_num;
                lables[11].text += "/";
                lables[11].text += highscore[2].totle_num;
                lables[12].text += highscore[3].win_num;
                lables[12].text += "/";
                lables[12].text += highscore[3].totle_num;
                lables[10].text += "  ";
                lables[11].text += "  ";
                lables[12].text += "  ";
                lables[10].text += highscore[1].time;
                lables[11].text += highscore[2].time;
                lables[12].text += highscore[3].time;
                lables[10].text += "  ";
                lables[11].text += "  ";
                lables[12].text += "  ";
                lables[10].text += highscore[1].date;
                lables[11].text += highscore[2].date;
                lables[12].text += highscore[3].date;
                lables[13].text = highscore[0].date;
                //lables[13].text += "  Best: ";
                //lables[13].text += highscore[1].time;
                
                
                hsloaded = 1;

            }
            
        }
    }
    else if (page_status == 3)
    {
        ed_time += dursec;
        if (ed_bg_black < 1)
        {
            ed_bg_black += dursec * 0.25;
        }
        g_pBrushDark->SetOpacity(ed_bg_black);
        g_pD2DDeviceContext->FillRectangle(

            D2D1::RectF(to_screen(0), to_screen(0), to_screen(1600), to_screen(900)),
            g_pBrushDark
        );
        DrawCallNum++;
        g_pBrushDark->SetOpacity(0.5);
        for (int i = 0; i < 2; i++)
        {
            if (buttons[i].y2 > 0)
            {
                buttons[i].y1 -= dursec * flow_speed;
                buttons[i].y2 -= dursec * flow_speed;
                boxes[buttons[i].box_index].y1 -= dursec * flow_speed;
                boxes[buttons[i].box_index].y2 -= dursec * flow_speed;
            }
            else
            {
                buttons[i].active = 0;
                boxes[buttons[i].box_index].active = 0;
            }

        }
        for (int i = 0; i < 128; i++)
        {
            if (!lables[i].active)
            {
                continue;
            }
            if (lables[i].y2 > -100)
            {
                lables[i].y1 -= dursec * flow_speed;
                lables[i].y2 -= dursec * flow_speed;
            }
            else
            {
                //lables[i].active = 0;
            }
        }
        if (ed_time > 60&& ed_stage==1)
        {
            CreateLable(0, 900+5.7*flow_speed, 1500, 1100+ 5.7 * flow_speed, "Thank you for playing", NULL, NULL, g_pBrushWhite, NULL, 3);//39
            if (!long_credit)
            {
                /*music_volume_buf = (float)set3[0].music_volume;
                music_volume_buf_c= (float)set3[0].music_volume;*/
                //playeffectsound(18);
                ed_stage = 2;
                
            }
            else
            {
                writelog("long credit.");
                thread_IO_request_update_profile = 1;
                ed_stage = 3;
                //spritesheet
                //CreateAnimation(5, 0, 0);
            }
            
        }
        if (ed_stage==2)
        {
            /*if (set3[0].music_volume > 0)
            {
                music_volume_buf_c -= dursec * 30;
                set3[0].music_volume = (int)music_volume_buf_c;
                thread_Audio_volume_changed_music = 1;
            }
            else if(thread_Audio_target_music!=0)
            {
                thread_Audio_target_music = 0;
                thread_Audio_switch_immediately = 1;
                
            }*/
            if (ed_bg_logo_posy > 200)
            {
                ed_bg_logo_posy -= dursec * flow_speed;
            }
            if (lables[41].y1 < 600)
            {
                lables[41].y1 = 600;
                lables[41].y2 = 800;
            }
            DrawBitmap_1(g_pD2DBimtapUI[15], D2D1::RectF(
                600,
                ed_bg_logo_posy,
                1000,
                ed_bg_logo_posy + 400
            ), 1.0f);
            DrawCallNum++;
            rendLable();
            if (ed_time > 73)
            {
                if (ed_fg_white < 1)
                {
                    ed_fg_white += dursec * 0.2;
                }
                else
                {
                    ed_stage = 0;
                    page_index = 1001;
                    page_status = 0;
                    start_page_stage = 0;
                }
                g_pBrushLight->SetOpacity(ed_fg_white);
                g_pD2DDeviceContext->FillRectangle(

                    D2D1::RectF(to_screen(0), to_screen(0), to_screen(1600), to_screen(900)),
                    g_pBrushLight
                );
                DrawCallNum++;
                g_pBrushLight->SetOpacity(0.5);
            }
            
        }
        else if (ed_stage == 3)
        {
            if (ed_bg_logo_posy > 200)
            {
                ed_bg_logo_posy -= dursec * flow_speed;
            }
            if (lables[41].y1 < 600)
            {
                lables[41].y1 = 600;
                lables[41].y2 = 800;
            }
            DrawBitmap_1(g_pD2DBimtapUI[15], D2D1::RectF(
                600,
                ed_bg_logo_posy,
                1000,
                ed_bg_logo_posy + 400
            ), 1.0f);
            DrawCallNum++;
            rendLable();
            if (ed_time > 75)
            {
                
                g_pBrushLight->SetOpacity(1.0);
                g_pD2DDeviceContext->FillRectangle(

                    D2D1::RectF(to_screen(0), to_screen(0), to_screen(1600), to_screen(900)),
                    g_pBrushLight
                );
                
                g_pBrushLight->SetOpacity(0.5);
                g_pD2DDeviceContext->DrawText(
                    multiByteToWideChar("MineSweeper"),           // Text to render
                    wcslen(multiByteToWideChar("MineSweeper")),       // Text length
                    g_pTextFormatLarge,     // Text format
                    D2D1::RectF(to_screen(200), to_screen(100), to_screen(1400), to_screen(350)),    // The region of the window where the text will be rendered
                    g_pBitmapBrushUI[12]      // The brush used to draw the text
                );
                g_pD2DDeviceContext->DrawText(
                    multiByteToWideChar("Z"),           // Text to render
                    wcslen(multiByteToWideChar("Z")),       // Text length
                    g_pTextFormatNormal,     // Text format
                    D2D1::RectF(to_screen(690), to_screen(490), to_screen(890), to_screen(690)),    // The region of the window where the text will be rendered
                    g_pBrushRed      // The brush used to draw the text
                );
                g_pD2DDeviceContext->DrawText(
                    multiByteToWideChar("Z"),           // Text to render
                    wcslen(multiByteToWideChar("Z")),       // Text length
                    g_pTextFormatNormal,     // Text format
                    D2D1::RectF(to_screen(700), to_screen(500), to_screen(900), to_screen(700)),    // The region of the window where the text will be rendered
                    g_pBrushGreen      // The brush used to draw the text
                );
                g_pD2DDeviceContext->DrawText(
                    multiByteToWideChar("Z"),           // Text to render
                    wcslen(multiByteToWideChar("Z")),       // Text length
                    g_pTextFormatNormal,     // Text format
                    D2D1::RectF(to_screen(710), to_screen(510), to_screen(910), to_screen(710)),    // The region of the window where the text will be rendered
                    g_pBrushLightBlue      // The brush used to draw the text
                );
                g_pD2DDeviceContext->DrawText(
                    multiByteToWideChar("C/C++ & DirectX Game Project\n2D Application Framework"),           // Text to render
                    wcslen(multiByteToWideChar("C/C++ & Game DirectX Project\n2D Application Framework")),       // Text length
                    g_pTextFormat,     // Text format
                    textLayoutRect,    // The region of the window where the text will be rendered
                    g_pBrushBlue      // The brush used to draw the text
                );
                DrawCallNum += 6;
            }
            if (ed_time > 78)
            {
                self_restart();
                //normal_quit = 0;
            }
            if (ed_time > 81)
            {
                quit_single = 1;
            }
            
        }
        if (ed_stage >= 1)
        {
            for (int i = 0; i < 35; i++)
            {
                if (ed_strings[i].ori == "")
                {
                    continue;
                }
                ed_strings[i].time += dursec;
                if (ed_strings[i].time > 2.5)
                {
                    if (/*ed_strings[i].time > -8 && */ed_strings[i].charnum < ed_strings[i].ori.length())
                    {
                        ed_strings[i].load_char_time -= dursec;
                        if (ed_strings[i].load_char_time < 0)
                        {
                            ed_strings[i].load_char_time = 0.1;
                            ed_strings[i].charnum++;



                        }
                        
                        lables[i + 14].text = ed_strings[i].ori.substr(0, ed_strings[i].charnum);
                        if ((int)(ed_strings[i].time * 5) % 4 == 0)
                        {
                            lables[i + 14].text += "-";
                        }
                        else if ((int)(ed_strings[i].time * 5) % 4 == 1)
                        {
                            lables[i + 14].text += "\\";
                        }
                        else if ((int)(ed_strings[i].time * 5) % 4 == 2)
                        {
                            lables[i + 14].text += "|";
                        }
                        else
                        {
                            lables[i + 14].text += "/";
                        }
                        
                    }
                    else
                    {
                        lables[i + 14].text = ed_strings[i].ori;
                    }
                }
                else
                {
                    lables[i + 14].text += "";
                }
                
            }
        }
        if (ed_stage == 0&&ed_time>1)
        {
            ed_stage = 1;
            ed_strings[0].ori = "MineSweeper Credit";
            ed_strings[0].time = -6;
            ed_strings[0].font = 3;
            ed_strings[26].ori = "Ver 1.1.0.1";
            ed_strings[26].time = -8;
            ed_strings[26].font = 4;
            ed_strings[1].ori = "Programming";
            ed_strings[1].time = -11;
            ed_strings[1].font = 3;
            ed_strings[2].ori = "AlexZ";
            ed_strings[2].time = -12.5;
            ed_strings[2].font = 2;
            ed_strings[3].ori = "Art & Animation Design";
            ed_strings[3].time = -14.5;
            ed_strings[3].font = 3;
            ed_strings[4].ori = "AlexZ";
            ed_strings[4].time = -16;
            ed_strings[4].font = 2;
            ed_strings[5].ori = "Assets Processing";
            ed_strings[5].time = -18;
            ed_strings[5].font = 3;
            ed_strings[6].ori = "AlexZ";
            ed_strings[6].time = -19.5;
            ed_strings[6].font = 2;
            
            ed_strings[7].ori = "Special Thanks";
            ed_strings[7].time = -22.5;
            ed_strings[7].font = 3;
            ed_strings[8].ori = "Technical Support";
            ed_strings[8].time = -24;
            ed_strings[8].font = 3;
            ed_strings[9].ori = "Microsoft DirectX";
            ed_strings[9].time = -25.5;
            ed_strings[9].font = 4;
            ed_strings[9].pos = 1;
            ed_strings[10].ori = "openAL.org";
            ed_strings[10].time = -25.5;
            ed_strings[10].font = 4;
            ed_strings[10].pos = 2;
            ed_strings[11].ori = "OpenSSL.org";
            ed_strings[11].time = -27;
            ed_strings[11].font = 4;
            ed_strings[11].pos = 1;
            ed_strings[12].ori = "Xiph.org";
            ed_strings[12].time = -27;
            ed_strings[12].font = 4;
            ed_strings[12].pos = 2;

            ed_strings[13].ori = "Fonts";
            ed_strings[13].time = -30;
            ed_strings[13].font = 3;
            ed_strings[14].ori = "得意黑";
            ed_strings[14].time = -31.5;
            ed_strings[14].font = 6;
            ed_strings[14].pos = 1;
            ed_strings[15].ori = "Quarlow";
            ed_strings[15].time = -31.5;
            ed_strings[15].font = 4;
            ed_strings[15].pos = 2;
            ed_strings[16].ori = "PublicSans";
            ed_strings[16].time = -33;
            ed_strings[16].font = 3;
            ed_strings[16].pos = 1;
            ed_strings[17].ori = "Pacifico";
            ed_strings[17].time = -33;
            ed_strings[17].font = 2;
            ed_strings[17].pos = 2;

            ed_strings[18].ori = "Music & Sounds & Assets";
            ed_strings[18].time = -36;
            ed_strings[18].font = 3;
            ed_strings[19].ori = "8号披萨PizzaNo8@Bilibili";
            ed_strings[19].time = -37.5;
            ed_strings[19].font = 6;
            ed_strings[19].pos = 1;
            ed_strings[20].ori = "AD:PIANO (JP)";
            ed_strings[20].time = -37.5;
            ed_strings[20].font = 6;
            ed_strings[20].pos = 2;
            ed_strings[21].ori = "Nepkey纳百技@Bilibili";
            ed_strings[21].time = -39;
            ed_strings[21].font = 6;
            ed_strings[21].pos = 1;
            ed_strings[25].ori = "Music is for audition only";
            ed_strings[25].time = -41;
            ed_strings[25].font = 3;
            /*ed_strings[22].ori ="Ver 1.1.0.1" ;
            ed_strings[22].time = -27;
            ed_strings[22].font = 6;
            ed_strings[22].pos = 2;
            ed_strings[23].ori = "";
            ed_strings[23].time = -28;
            ed_strings[23].font = 6;
            ed_strings[23].pos = 1;
            ed_strings[24].ori = "";
            ed_strings[24].time = -28;
            ed_strings[24].font = 6;
            ed_strings[24].pos = 2;
            ed_strings[25].ori = "";
            ed_strings[25].time = -29;
            ed_strings[25].font = 6;
            ed_strings[25].pos = 1;
            ed_strings[26].ori = "";
            ed_strings[26].time = -29;
            ed_strings[26].font = 6;
            ed_strings[26].pos = 2;
            ed_strings[27].ori = "";
            ed_strings[27].time = -30;
            ed_strings[27].font = 6;
            ed_strings[27].pos = 1;
            ed_strings[28].ori = "";
            ed_strings[28].time = -30;
            ed_strings[28].font = 6;
            ed_strings[28].pos = 2;
            ed_strings[29].ori = "";
            ed_strings[29].time = -31;
            ed_strings[29].font = 6;
            ed_strings[29].pos = 1;*/

            ed_strings[22].ori = "Pictures";
            ed_strings[22].time = -44;
            ed_strings[22].font = 3;
            ed_strings[23].ori = "https://products.ls.graphics and other assets come from Internet";
            ed_strings[23].time = -45.5;
            ed_strings[23].font = 6;

            /*ed_strings[32].ori = "";
            ed_strings[32].time = -36;
            ed_strings[32].font = 3;
            ed_strings[33].ori = "";
            ed_strings[33].time = -37;
            ed_strings[33].font = 2;*/
            
            ed_strings[24].ori = "For non-commercial use only";
            ed_strings[24].time = -49;
            ed_strings[24].font = 3;


            for (int i = 0; i < 35; i++)//14+i
            {
                if (ed_strings[i].time > -3)
                {
                    continue;
                }
                if (ed_strings[i].pos == 1)
                {
                    CreateLable(0, 900-ed_strings[i].time * flow_speed, 800, 1100-ed_strings[i].time * flow_speed, "", NULL, NULL, g_pBrushWhite, NULL, ed_strings[i].font);
                }
                else if (ed_strings[i].pos == 2)
                {
                    CreateLable(800, 900-ed_strings[i].time * flow_speed, 1600, 1100-ed_strings[i].time * flow_speed, "", NULL, NULL, g_pBrushWhite, NULL, ed_strings[i].font);
                }
                else
                {
                    CreateLable(0, 900-ed_strings[i].time * flow_speed, 1500, 1100-ed_strings[i].time * flow_speed, "", NULL, NULL, g_pBrushWhite, NULL, ed_strings[i].font);
                }
                
            }
            

        }
    }
    return;
}

void rendEffect()
{
    ID2D1SolidColorBrush* g_pBrushbuf = g_pBrushLight;
    for (int i = 0; i < 4096; i++)
    {
        if (!alleffects[i].active|| alleffects[i].index>=64)
        {
            continue;
        }
        
        switch (alleffects[i].index)
        {
        case 1://loading
            alleffects[i].time += dursec;
            break;
        case 2:
            alleffects[i].time -= dursec;
            if (alleffects[i].time < 0)
            {
                alleffects[i].active = 0;
            }
            break;
        case 3:
            alleffects[i].time -= dursec;
            if (alleffects[i].time < 0)
            {
                alleffects[i].active = 0;
            }
            break;

        default:
            break;
        }
        if (alleffects[i].index == 1)//0-0.3s第1张滑入，0.3-0.6s第1张滑出,0.2-0.4s第2张滑入,0.4-0.6第2张滑出
        {
            if (g_pBrushBGSelect == NULL)
            {
                g_pD2DDeviceContext->FillRectangle(

                    D2D1::RectF(to_screen(alleffects[i].time * 5333 - 1600), to_screen(0), to_screen(alleffects[i].time * 5333), to_screen(900)),
                    g_pBrushLight
                );
            }
            else
            {
                g_pD2DDeviceContext->FillRectangle(

                    D2D1::RectF(to_screen(alleffects[i].time * 5333 - 1600), to_screen(0), to_screen(alleffects[i].time * 5333), to_screen(900)),
                    g_pBrushBGSelect
                );
            }
            g_pBrushLight->SetOpacity(0.3);
            g_pD2DDeviceContext->FillRectangle(

                D2D1::RectF(to_screen(alleffects[i].time * 8000 - 3200), to_screen(0), to_screen(alleffects[i].time * 8000-1600), to_screen(900)),
                g_pBrushLight
            );
            DrawCallNum += 2;
            if (alleffects[i].time > 0.6)
            {
                alleffects[i].active = 0;
            }
        }
        else if (alleffects[i].index == 2)
        {
            g_pBrushLight->SetOpacity(0.2f);
            g_pD2DDeviceContext->FillRectangle(

                D2D1::RectF(to_screen(alleffects[i].posx), to_screen(alleffects[i].posy), to_screen(alleffects[i].posx+cubenum_w*cubeMap_width), to_screen(alleffects[i].posy+5)),
                g_pBrushLight
            );
        }
        else if (alleffects[i].index == 3)
        {
            g_pBrushLight->SetOpacity(alleffects[i].time*2);
            g_pD2DDeviceContext->FillRectangle(

                D2D1::RectF(to_screen(alleffects[i].posx), to_screen(alleffects[i].posy), to_screen(alleffects[i].posx + cubeMap_width), to_screen(alleffects[i].posy + cubeMap_width)),
                g_pBrushLight
            );
        }

    }
    g_pBrushLight->SetOpacity(0.5);
    return;
}
void rend_verification_warning()
{
    g_pD2DDeviceContext->FillEllipse(D2D1::Ellipse(D2D1::Point2(to_screen(800), to_screen(350)), to_screen(200), to_screen(200)), g_pBrushRed);
    g_pD2DDeviceContext->FillRoundedRectangle(
        D2D1::RoundedRect(
            D2D1::RectF(to_screen(780), to_screen(200), to_screen(820),
                to_screen(430)),
            to_screen(10.0f),
            to_screen(10.0f)),
        g_pBrushYellow
        
    );
    g_pD2DDeviceContext->FillEllipse(D2D1::Ellipse(D2D1::Point2(to_screen(800), to_screen(470)), to_screen(20), to_screen(20)), g_pBrushYellow);
    g_pD2DDeviceContext->DrawText(
        multiByteToWideChar("Oops, found resources file corruption , please reinstall the application.\nYou may move the \"save\" folder to keep your progress."),           // Text to render
        wcslen(multiByteToWideChar("Oops, found resources file corruption , please reinstall the application.\nYou may move the \"save\" folder to keep your progress.")),       // Text length
        g_pTextFormatNormal,     // Text format
        D2D1::RectF(to_screen(300), to_screen(650), to_screen(1300),
            to_screen(800)),
            // The region of the window where the text will be rendered
        g_pBrushBlack      // The brush used to draw the text
    );
    if (clicking)
    {
        normal_quit = 1;
    }
    return;
}
string input_username = ""; char usernamebuf;
bool char_num_inlegal = 0;
float enter_totice_time = 0;
void rend_new_user()
{
    g_pD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::Snow));
    if (page_status == 0)
    {
        InitPage(3001);
        

        page_status = 1;
        playeffectsound(17);
    }
    else if (page_status == 1)
    {
        CreateButton(730, 650, 870, 720, "OK", g_pBrushBlack, g_pBrushYellow, g_pBrushLightGreen, NULL);//0
        CreateLable(600, 450, 1000, 550, "", g_pBrushPink, NULL, g_pBrushBlack, NULL, 1);//0
        CreateLable(300, 350, 1100, 450, lan[0].enter_username, NULL, NULL, g_pBrushBlack, NULL, 3);//1
        page_status = 2;
        input_expand = 1;
    }
    else if (page_status == 2)
    {
        enter_totice_time -= dursec;
        lables[0].text = input_username;
        if (enter_totice_time > 0)
        {
            lables[0].text += "_";
        }
        if (enter_totice_time < -1)
        {
            enter_totice_time = 1;
        }

        if (expand_input_key != NULL)
        {
            playeffectsound(17);
            if (expand_input_key == 35)
            {
                if (input_username.length() > 0)
                {
                    input_username = input_username.substr(0, input_username.length()-1);
                    
                }
            }
            else if (expand_input_key == 36)
            {
                boxes[buttons[0].box_index].clicked = 1;
            }
            else
            {
                input_username += expand_input_key;
            }
            
        }
        if (input_username.length() > 8)
        {
            input_username = input_username.substr(0,8);
            char_num_inlegal = 1;
        }
        
        if (boxes[buttons[0].box_index].clicked)
        {
            boxes[buttons[0].box_index].clicked = 0;
            if (input_username.length() > 8 || input_username.length() < 1)
            {
                char_num_inlegal = 1;
            }
            else
            {
                playeffectsound(16);
                buttons[0].text = "loading...";
                buttons[0].Brush3 = g_pBrushYellow;
                page_status = 3;
            }
        }
        g_pD2DDeviceContext->DrawText(
            multiByteToWideChar("Sign up"),           // Text to render
            wcslen(multiByteToWideChar("Sign up")),       // Text length
            g_pTextFormatNormal,     // Text format
            D2D1::RectF(to_screen(200), to_screen(150), to_screen(600),
                to_screen(300)),
            // The region of the window where the text will be rendered
            g_pBrushBlack      // The brush used to draw the text
        );
        if (char_num_inlegal)
        {
            g_pD2DDeviceContext->DrawText(
                multiByteToWideChar("restriction: 1-8 character"),           // Text to render
                wcslen(multiByteToWideChar("restriction: 1-8 character")),       // Text length
                g_pTextFormatNormal,     // Text format
                D2D1::RectF(to_screen(500), to_screen(580), to_screen(1100),
                    to_screen(650)),
                // The region of the window where the text will be rendered
                g_pBrushRed      // The brush used to draw the text
            );
        }
    }
    else if (page_status == 3)
    {
    g_pD2DDeviceContext->DrawText(
        multiByteToWideChar("Sign up"),           // Text to render
        wcslen(multiByteToWideChar("Sign up")),       // Text length
        g_pTextFormatNormal,     // Text format
        D2D1::RectF(to_screen(200), to_screen(150), to_screen(600),
            to_screen(300)),
        // The region of the window where the text will be rendered
        g_pBrushBlack      // The brush used to draw the text
    );
    usernameC = input_username;
    thread_IO_request_userinit=1;

    playeffectsound(17);
    page_status = 4;
    }
    else if (page_status == 4)
    {
        
    if (thread_IO_request_userinit==0)
    {
        buttons[0].text = "Done";
        buttons[0].Brush3 = g_pBrushLightGreen;
        if (!self_restarted)
        {
            self_restart();
        }
        thread_IO_request_userinit = -2;
    }
    else if (thread_IO_request_userinit == -1)
    {
        buttons[0].text = "Error";
        buttons[0].Brush3 = g_pBrushRed;
    }
    else if (thread_IO_request_userinit == -2)
    {
        Sleep(500);
        thread_IO_request_userinit = -3;
    }
    }
    return;
}
void rendPage()
{
    //writelog(to_string(page_index));
    switch (page_index)
    {
    case 0:
        rend_home_page();
        break;
    case 1:
        rend_setting_page();
        break;
    case 2:
        game_main();
        break;
    case 3:
        page_game_save();
        break;
    case 4:
        page_game_load();
        break;
    case 5:
        rend_page_history();
        break;
    case 6:
        rend_page_highscore();
        break;
    case 1000:
    case 1001:
        rend_start();
        break;
    case 2001:
        rend_verification_warning();
        break;
    case 3001:
        rend_new_user();
        break;
    default:
        writelog("exception in rendPage!");
        MessageBox(NULL, "rendPage failed!", "Error", 0);
        break;
    }
    return;
}


string str1;

void render()
{
    
    HRESULT hr = S_OK;
    //MessageBox(NULL, "start rending", "3", 0);
    //g_pD2DDeviceContext->GetTarget();
    current = Clock::now();
    ::end = current;
    durnanosec = chrono::duration_cast<std::chrono::nanoseconds>(::end - start).count();
    dursec = (float)durnanosec / 1000000000;
    start = current;
    DrawCallNum = 1;
    
    update();
    //g_pSwapChain->SetBackgroundColor()
    g_pD2DDeviceContext->BeginDraw();

    if (FAILED(hr))
    {
        MessageBox(NULL, "Draw failed!", "Error", 0);
        return;
    }
    g_pD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::LightSlateGray));
    //g_pD2DDeviceContext->FillEllipse(
    //    D2D1::Ellipse(D2D1::Point2F(f1 * scale, f2 * scale), 50 * scale, 50 * scale),
    //    g_pBrushGreen);
    ////g_pBrushtank->SetColor(tanks[i].color4);
    //g_pD2DDeviceContext->DrawEllipse(
    //    D2D1::Ellipse(D2D1::Point2F(f1 * scale, f2 * scale), 50 * scale, 50 * scale),
    //    g_pBrushRed, 1.0F * scale);
    //DrawCallNum += 2;
    
    rendPage();
    
    if (page_index == 1)
    {
        
        rendLable();
        rendButton();
    }
    else if(ed_stage<2&& page_index!=5)
    {
        rendButton();
        rendLable();
    }
    if (set2[0].visual_effect)
    {
        rendEffect();
    }
    if (page_index != 5)
    {
        DrawSpriteSheet();
    }
    
    rend_quit();

    frame++;
    nframe++;
    if (nframe == 50)	//计算帧率
    {
        nframe = 0;
        endb = current;
        dur20 = chrono::duration_cast<std::chrono::nanoseconds>(endb - startb).count();

        //fps1 = 5000.0F / (float)dur20;
        fps = 50000000000 / dur20;
        startb = current;
        //fps = (int)(10.0 * fps + 0.5) / 10.0;
    }
    str1 = "";
    /*str1 += to_string(cpos.x);
    str1 += "  ";
    str1 += to_string(cpos.y);
    
    str1 += "\n";*/
    str1 += to_string(fps);
    str1 += " ";
    /*str1 += to_string(rc.bottom - rc.top);
    str1 += to_string(rc.right - rc.left);
    str1 += "\n";*/
    str1 += to_string(DrawCallNum);
    //str1 += " ";
    //str1 += getTimeDigit(mouse_move_duration);
    
    if (page_index < 100&&set2[0].show_framerate)
    {
 g_pD2DDeviceContext->DrawText(
            multiByteToWideChar(str1),           // Text to render
            wcslen(multiByteToWideChar(str1)),       // Text length
            g_pTextFormat,     // Text format
            fpsRect,    // The region of the window where the text will be rendered
            g_pBrushBlue      // The brush used to draw the text
        );
    }
        
       

    hr = g_pD2DDeviceContext->EndDraw(); 

    if (FAILED(hr))
    {
        writelog("Rending Error: Draw failed!");
        
        MessageBox(NULL, "Draw failed!", "Rending Error", 0);
        Sleep(3000);
        quit_single = 1;
        return;
    }
    if (set2[0].vsync&& g_output)
    {
        g_output->WaitForVBlank();
        if (!set2[0].visual_effect)
        {
            g_output->WaitForVBlank();
        }
    }
    
    g_pSwapChain->Present(0, 0);
    return;
}
//渲染逻辑部分结束

string wstring2string(wstring wstr)
{
    string result;
    //获取缓冲区大小，并申请空间，缓冲区大小按字节计算
    int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
    char* buffer = new char[len + 1];
    //宽字节编码转换成多字节编码  
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, NULL, NULL);
    buffer[len] = '\0';
    //删除缓冲区并返回值  
    result.append(buffer);
    delete[] buffer;
    return result;
}
LPCWSTR stringToLPCWSTR(string orig)
{
    size_t origsize = orig.length() + 1;
    const size_t newsize = 100;
    size_t convertedChars = 0;
    wchar_t* wcstring = (wchar_t*)malloc(sizeof(wchar_t) * (orig.length() + 1));
    mbstowcs_s(&convertedChars, wcstring, origsize, orig.c_str(), _TRUNCATE);

    return wcstring;
}


void initgame()
{
    
    current = Clock::now();
    startb = current;
    start = current;
    writelog("Loading Res table...");

    res[0].filename = ".\\pic\\home_setting.png"; res[0].md5 = "D99A737976059CB3A2D89928A47B0C36";//textures of ui
    res[1].filename = ".\\pic\\home_exit.png"; res[1].md5 = "3B94B2D92E76B074438255DA74E7DE41";
    res[2].filename = ".\\pic\\setting_general.png"; res[2].md5 = "A502C14ED186CBB5B29ABFB01B52F405";
    res[3].filename = ".\\pic\\setting_graphics1.png"; res[3].md5 = "CB5406ECF3AF342ECCBFB1E0D5405CF7";
    res[4].filename = ".\\pic\\setting_graphics2.png"; res[4].md5 = "E930F7354F7DECE20B0831C6BADD94CD";
    res[5].filename = ".\\pic\\setting_audio1.png"; res[5].md5 = "1C4C1657E742236342C45363F991B5B4";
    res[6].filename = ".\\pic\\setting_audio2.png"; res[6].md5 = "C005637059C36F1A7397AF46927FD203";
    res[7].filename = ".\\pic\\read_failed.png"; res[7].md5 = "E9F0465562EC31499E7B3063FC8A6B14";
    res[8].filename = ".\\pic\\save_success1.png"; res[8].md5 = "C54900ECC61043AF96A96FE2C5484172";
    res[9].filename = ".\\pic\\save_success2.png"; res[9].md5 = "4B7D127A76421BB5FA1C88FD51C657FF";
    res[10].filename = ".\\pic\\setting_return.png"; res[10].md5 = "0D72F585852429425AE86A77AD06A82C";
    res[11].filename = ".\\pic\\restart_1.png"; res[11].md5 = "B61C5657C1B955AD5B4967DE3023F005";
    res[12].filename = ".\\pic\\restart_2.png"; res[12].md5 = "3A9FA73044CBA8BE20643D38C0E61817";
    

    res[13].filename = ".\\pic\\skin101.jpg"; res[13].md5 = "84F1386AA1EB15C56CEE2C87F0E48287";  //textures of bg
    res[14].filename = ".\\pic\\skin102.jpg"; res[14].md5 = "A5E5741227E5F5BFB0F3808C9CE4703E";

             //textures
    res[15].filename = ".\\pic\\mine1.png"; res[15].md5 = "94DFFB63E40869F145293D2EBA476A73";
    res[16].filename = ".\\pic\\mine2.png"; res[16].md5 = "FA2EB18474A1A3DEBDD3BE582C751AEA";
    res[17].filename = ".\\pic\\mine3.png"; res[17].md5 = "80599C49996AFD0F557AD68A7D8BCEA5";
    res[18].filename = ".\\pic\\mine1r.png"; res[18].md5 = "3292AE8BA721FE05581F6543D87C9F03";
    res[19].filename = ".\\pic\\mine2r.png"; res[19].md5 = "AAE195823715A1CD40583DA817678273";
    res[20].filename = ".\\pic\\mine3r.png"; res[20].md5 = "0104FB5CC05D041B7495AEE9A18E9C27";
    res[21].filename = ".\\pic\\flag.png"; res[21].md5 = "F5DA9ECBC2686AB9D1E6731BEA8072D7";
  
    res[22].filename = ".\\sprite\\Avatar_1.png"; res[22].md5 = "2FB5AD99B9E915A2BDC8757433715207";
    res[23].filename = ".\\sprite\\Avatar_2.png";  res[23].md5 = "05F1A962A3735A77ED9A8817BDF64B26";    //sprite textures
    res[24].filename = ".\\sprite\\Avatar_3.png"; res[24].md5 = "A7D14EDB58079B32B65FE30DC05D7CBA";
    res[25].filename = ".\\sprite\\Avatar_4.png"; res[25].md5 = "63E8C32F3F130E1BB0DF094EBA60182A";
    res[26].filename = ".\\sprite\\bang1.png"; res[26].md5 = "7B6FE14040A47FD1EEBC0FE059BD80B3";
    res[27].filename = ".\\sprite\\bang2.png"; res[27].md5 = "09B4B89131B1F0842F84C9428D3A0FBA";
    res[28].filename = ".\\sprite\\bang3.png"; res[28].md5 = "B5B2E1EAD42322283567128D18EF5534";
    res[29].filename = ".\\sprite\\bang4.png"; res[29].md5 = "18E8BAF152980B9DC5CAA7409C3FC7B1";

    res[30].filename = ".\\pic\\time.png"; res[30].md5 = "0B0106A68002F6D4B13BBB1D33EB4EB1";

    //res[32].filename = ".\\music\\sys\\mainpage1.ogg"; res[32].md5 = "E8DE51E10E74D23FFD0D4404784FE11B";
    //res[33].filename = ".\\music\\sys\\mainpage2.ogg"; res[33].md5 = "BEBD37B788A0BC36732FDFE5C83E79FE";
    //res[34].filename = ".\\music\\sys\\mainpage3.ogg"; res[34].md5 = "4D58F2DDE3694B2F18D61D782B905728";
    //res[35].filename = ".\\music\\sys\\historypage1.ogg"; res[35].md5 = "5F62DB687FB931878DBAFEEAF24046DD";

    //res[36].filename = ".\\music\\sys\\historypage2.ogg"; res[36].md5 = "7EAB6400BA4572960FAAEE75EB986D92";
    //res[37].filename = ".\\music\\sys\\finish1.ogg"; res[37].md5 = "5BF60A47C777814326E54523B70497B4";
    //res[38].filename = ".\\music\\sys\\finish2.ogg"; res[38].md5 = "BBE9027C1484D58FA270C1D11006EF6E";

    res[39].filename = ".\\music\\sys\\ed.ogg"; res[39].md5 = "33F5BEF1192BC204DC9B0CEF7AF87751";
    res[40].filename = ".\\music\\sys\\atmosphere_rainy.ogg"; res[40].md5 = "3789A971CCC72649E3287566A67E2ED6";

    res[41].filename = ".\\sounds\\Button1.wav"; res[41].md5 = "D00573C17D3B5639261839BC59EEF76F";
    res[42].filename = ".\\sounds\\Button2.wav"; res[42].md5 = "A292398982A14804D067D21DA5C64DC5";
    res[43].filename = ".\\sounds\\save.wav"; res[43].md5 = "B0A66E74055B49F3B76A75A2A84547B6";
    res[44].filename = ".\\sounds\\load.wav"; res[44].md5 = "5AE81F2D731AD6A185DE426AA89EB766";
    res[45].filename = ".\\sounds\\gameover.wav"; res[45].md5 = "4574070CA0531CD827879C0A5DB6BE24";
    res[46].filename = ".\\sounds\\alertTick.wav"; res[46].md5 = "3A449EA4AE4BFAFFF341F36C9F885546";
    res[47].filename = ".\\sounds\\endofpage.wav"; res[47].md5 = "C772AE11BBE74516838855E6198FA852";
    res[48].filename = ".\\sounds\\boom.wav"; res[48].md5 = "23C33D7C3889CAF5370513FBED10AE9F";
    
    res[49].filename =  ".\\sounds\\ioerror.wav"; res[49].md5 = "F83BD1119C3DC605525D648DE963C7EC";
    res[50].filename =  ".\\sounds\\pass.wav"; res[50].md5 = "F9EC142C6A49D29F0D01EF916ABCE65A";
    res[51].filename =  ".\\sounds\\tick.wav"; res[51].md5 = "53C4F41AAE08974782B99A6C553E3B0C";
    res[52].filename =  ".\\sounds\\loading.wav"; res[52].md5 = "83C94BFFC673E2801FE9A77DFF751549";
    //res[32].filename = "";  //bgm

    //res[64].filename = "";  //se
    //
    for (int i = 0; i < 128; i++)
    {
        if (i == 127)
        {
            md5_result = 1;
        }
        if (res[i].filename != "")
        {
            if (!md5_verify(i))
            {
                writelog("resources md5 verification failed!  "+ res[i].filename);
                //writelog(res[i].filename);
                md5_result = -1;
                break;
            }
            
        }
        
    }
    //md5_result = 1;//debug
    if (md5_result==1)
    {
        for (int i = 0; i < 32; i++)
        {
            if (res[i].filename != "")
            {
                res[i].Lfilename = stringToLPCWSTR(res[i].filename);
            }

        }
        writelog("Loading Bitmap From File...");
        for (int i = 0; i < 32; i++)
        {
            if (res[i].filename == "")
            {
                continue;
            }
            if (!SUCCEEDED(LoadBitmapFromFile(g_pD2DDeviceContext, pIWICFactory, res[i].Lfilename, 0, 0, &g_pD2DBimtapUI[i])))
            {
                //MessageBoxEx(hWnd, to_string(i).c_str(), "Error", 0, NULL);
                //MessageBoxEx(hWnd, "Load Bitmap From File failed!\npic", "Error", 0, NULL);
                writelog("Load Bitmap From File failed!");
                writelog(res[i].filename);
                quit_single = 1;
                return;
            }
        }

        D2D1_BITMAP_BRUSH_PROPERTIES1 BITMAP_BRUSH_PROPERTY1;
        BITMAP_BRUSH_PROPERTY1.extendModeX = D2D1_EXTEND_MODE_WRAP;
        BITMAP_BRUSH_PROPERTY1.extendModeY = D2D1_EXTEND_MODE_WRAP;
        BITMAP_BRUSH_PROPERTY1.interpolationMode = D2D1_INTERPOLATION_MODE_CUBIC;
        writelog("Creating BitmapBrush From Bitmap...");
        for (int i = 13; i < 15; i++)
        {
            if (!SUCCEEDED(g_pD2DDeviceContext->CreateBitmapBrush(g_pD2DBimtapUI[i], BITMAP_BRUSH_PROPERTY1, &g_pBitmapBrushUI[i-1])))
            {
                //MessageBoxEx(hWnd, to_string(i).c_str(), "Error", 0, NULL);
                //MessageBoxEx(hWnd, "Create BitmapBrush From Bitmap failed!\npic", "Error", 0, NULL);
                writelog("Create BitmapBrush From Bitmap failed!");
                writelog(to_string(i));
                quit_single = 1;
                return;
            }
        }
    }
    
    for (int i = 13; i < 15; i++)
    {

        SAFE_RELEASE(g_pD2DBimtapUI[i]);
    }
    //writelog("Load Bitmap From File finished.");
    read_infos[0].rect = D2D1::RectF(150, 100, 400, 400);
    read_infos[1].rect = D2D1::RectF(150, 450, 400, 750);
    read_infos[2].rect = D2D1::RectF(440, 100, 690, 400);
    read_infos[3].rect = D2D1::RectF(440, 450, 690, 750);
    read_infos[4].rect = D2D1::RectF(730, 100, 980, 400);
    read_infos[5].rect = D2D1::RectF(730, 450, 980, 750);
    read_infos[6].rect = D2D1::RectF(1020, 100, 1270, 400);
    read_infos[7].rect = D2D1::RectF(1020, 450, 1270, 750);
    read_infos[8].rect = D2D1::RectF(1310, 100, 1560, 400);
    read_infos[9].rect = D2D1::RectF(1310, 450, 1560, 750);
    GetCursorPos(&cpos);                       //获取鼠标在屏幕上的位置
    ScreenToClient(hWnd, &cpos);
    init_string();
    tip_used_num = rand() % 16;
    return;
}
int orix = 0;
int oriy = 0;
HANDLE hThreadLog, hThreadAudio;
void init_once()
{
    hThreadLog = (HANDLE)_beginthreadex(NULL, 0, File_IO, NULL, 0, NULL);	//创建线程

    if (hThreadLog == 0)
    {
        writelog("Failed to create IO thread!");
        MessageBoxEx(NULL, "Failed to create IO thread", "Error", MB_OK | MB_ICONSTOP, NULL);
        quit_single = 1;
        return;
    }
    srand((unsigned int)GetTickCount64());
    RECT winrc;
    //string dbg;
    SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
    SetProcessPriorityBoost(GetCurrentProcess(), TRUE);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
    
    GetLocalTime(&st);
    GetClientRect(hWnd, &rc);
    /*DEVMODE dm;
    EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
    nWidth = dm.dmPelsWidth;
    nHeight= dm.dmPelsHeight;*/
    if (nWidth >= 7680 && nHeight >= 4320)
    {
        orix = 3840;
        oriy = 2160;
        scale = 2.4;
        resolution_allow = 6;
    }
    else if (nWidth >= 3840 && nHeight >= 2160)
    {
        orix = 2560;
        oriy = 1440;
        scale = 1.6;
        resolution_allow = 5;
    }
    else if (nWidth >= 2560 && nHeight >= 1440)
    {
        orix = 1920;
        oriy = 1080;
        scale = 1.2;
        resolution_allow = 4;
    }
    else if (nWidth >= 1920 && nHeight >= 1080)
    {
        orix = 1600;
        oriy = 900;
        scale = 1.0;
        resolution_allow = 3;
    }
    else if (nWidth >= 1280 && nHeight >= 760)
    {
        orix = 1280;
        oriy = 720;
        scale = 0.8;
        resolution_allow = 2;
    }
    else
        //不安全的分辨率
    {
        orix = 800;
        oriy = 450;
        scale = 0.5;
        resolution_allow = 1;
    }
    set2[1].resolution = resolution_allow;
    int io_wait_num = 0;
    while (io_wait_num < 10)
    {
        if (!thread_IO_config_read)
        {
            writelog("waiting IO thread...");
            Sleep(100);
            io_wait_num++;
        }
        else
        {
            break;
        }
    }
    
    if (!thread_IO_config_read)
    {
        writelog("use default config.");
        writelog("target resolution: " + to_string(orix) + "x" + to_string(oriy)+" from "+ to_string(nWidth) + "x" + to_string(nHeight));
    }
    else
    {
        if (set2[0].resolution <= resolution_allow)
        {
            switch (set2[0].resolution)
            {
            case 1:
                orix = 800;
                oriy = 450;
                scale = 0.5;
                break;
            case 2:
                orix = 1280;
                oriy = 720;
                scale = 0.8;
                break;
            case 3:
                orix = 1600;
                oriy = 900;
                scale = 1.0;
                break;
            case 4:
                orix = 1920;
                oriy = 1080;
                scale = 1.2;
                break;
            case 5:
                orix = 2560;
                oriy = 1440;
                scale = 1.6;
                break;
            case 6:
                orix = 3840;
                oriy = 2160;
                scale = 2.4;
                break;
            }
        }
    }

    while (1)
    {
        GetWindowRect(hWnd, &winrc);
        GetClientRect(hWnd, &rc);
        //dbg = to_string((rc.bottom - rc.top) * 16 / 9);
        //dbg += "   ";
        //dbg += to_string(rc.right - rc.left);
        if (rc.bottom - rc.top +5< oriy)
        {
            //writelog(to_string(rc.right - rc.left));
            //writelog(to_string(rc.bottom - rc.top));
            SetWindowPos(hWnd, HWND_TOP, 0, 0, winrc.right - winrc.left, winrc.bottom - winrc.top + 5, NULL);
        }
        else if (rc.bottom - rc.top-5 > oriy)
        {
            //writelog(to_string(rc.bottom - rc.top));
            SetWindowPos(hWnd, HWND_TOP, 0, 0, winrc.right - winrc.left, winrc.bottom - winrc.top - 5, NULL);
        }
        else if (rc.right - rc.left+5 < orix)
        {
            //writelog(to_string(rc.right - rc.left));
            //writelog(to_string(rc.right - rc.left));
            SetWindowPos(hWnd, HWND_TOP, 0, 0, winrc.right - winrc.left + 5, winrc.bottom - winrc.top, NULL);
        }
        else if (rc.right - rc.left-5 > orix)
        {
            //writelog(to_string(rc.right - rc.left));
            SetWindowPos(hWnd, HWND_TOP, 0, 0, winrc.right - winrc.left - 5, winrc.bottom - winrc.top, NULL);
        }
        else if (rc.bottom - rc.top < oriy)
        {
            //writelog(to_string(rc.right - rc.left));
            //writelog(to_string(rc.bottom - rc.top));
            SetWindowPos(hWnd, HWND_TOP, 0, 0, winrc.right - winrc.left, winrc.bottom - winrc.top + 1, NULL);
        }
        else if (rc.bottom - rc.top > oriy)
        {
            //writelog(to_string(rc.bottom - rc.top));
            SetWindowPos(hWnd, HWND_TOP, 0, 0, winrc.right - winrc.left, winrc.bottom - winrc.top - 1, NULL);
        }
        else if (rc.right - rc.left < orix)
        {
            //writelog(to_string(rc.right - rc.left));
            //writelog(to_string(rc.right - rc.left));
            SetWindowPos(hWnd, HWND_TOP, 0, 0, winrc.right - winrc.left + 1, winrc.bottom - winrc.top, NULL);
        }
        else if (rc.right - rc.left > orix)
        {
            //writelog(to_string(rc.right - rc.left));
            SetWindowPos(hWnd, HWND_TOP, 0, 0, winrc.right - winrc.left - 1, winrc.bottom - winrc.top, NULL);
        }
        else
        {
            break;
        }
    }
    string buf = "Window completely resized @";
    buf += to_string(orix);
    buf += "x";
    buf += to_string(oriy);
    writelog(buf);
    hThreadAudio = (HANDLE)_beginthreadex(NULL, 0, ThreadPlayMusic, NULL, 0, NULL);	//创建线程
    if (hThreadAudio == 0)
    {
        writelog("Failed to create Audio thread!");
        MessageBoxEx(NULL, "Failed to create Audio thread!", "Error", MB_OK | MB_ICONSTOP, NULL);
        quit_single = 1;
        return;
    }
    SetThreadPriority(hThreadAudio, THREAD_PRIORITY_ABOVE_NORMAL);
    
    CreateD3DResource(hWnd);
    CreateD2DResource(hWnd);
    fpsRect= D2D1::RectF(
        static_cast<FLOAT>(0),
        static_cast<FLOAT>(0),
        static_cast<FLOAT>(to_screen(150)),
        static_cast<FLOAT>(to_screen(50))
    );
    textLayoutRect = D2D1::RectF(
        static_cast<FLOAT>(rc.left),
        static_cast<FLOAT>(rc.top),
        static_cast<FLOAT>(rc.right - rc.left),
        static_cast<FLOAT>(rc.bottom - rc.top)
    );
    textrbRect = D2D1::RectF(
        static_cast<FLOAT>(rc.right- to_screen(250)),
        static_cast<FLOAT>(rc.bottom - to_screen(150)),
        static_cast<FLOAT>(rc.right),
        static_cast<FLOAT>(rc.bottom)
    );
    return;
}

void process_quit()
{
    writelog("Program quiting...(error occured)");
    WaitForSingleObject(ThreadPlayMusic, 500);
    WaitForSingleObject(hThreadLog, 500);
    
    //RemoveFontResourceExA(".\\fonts\\SmileySans-Oblique.ttf", FR_PRIVATE, NULL);
    return;
}




// 全局变量:
HINSTANCE hInst;                                // 当前实例
//CHAR szTitle[MAX_LOADSTRING] = "MineSweeper";                  // 标题栏文本
CHAR szWindowClass[MAX_LOADSTRING]="MineSweeper";            // 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
char* buf1;
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。
    
    // 初始化全局字符串
    //LoadStringA(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    //LoadStringA(hInstance, IDC_MY2048, szWindowClass, MAX_LOADSTRING);
    
    //LoadStringA(hInstance, IDC_MY2048,buf1 , MAX_LOADSTRING);
    
    
    MyRegisterClass(hInstance);
    
    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY2048));

    MSG msg;
    init_once();
    page_index = 1000;
    if (quit_single == 1)
    {
        writelog("Init Failed, quiting...");
        process_quit();
        DestroyWindow(hWnd);
        return -1;
    }

    render();
    
    initgame();
    if (md5_result == 1)
    {
        if (usernameC == "")
        {
            page_index = 3001;
        }
        else
        {
            page_index = 1001;//ready
        }
        
    }
    else
    {
        page_index = 2001;
    }
    
    if (quit_single == 1)
    {
        writelog("Init Failed, quiting...");
        process_quit();
        return -1;
    }
    writelog("Init finished.");
    //Sleep(1);
    // 主消息循环:
    while (TRUE)
    {
        input();
        render();
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                normal_quit = 1;
            }
                
            
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            
        }
        if (quit_single == 1)
        {
            
            process_quit();
            DestroyWindow(hWnd);
            //ExitProcess(0);
            break;
        }
        
    }


    

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_MY2048);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中
   //const char* buf = "2048";
   hWnd = CreateWindowEx(0L,"MineSweeper", "MineSweeper", WS_OVERLAPPEDWINDOW,
      0, 0, 800, 600, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
       MessageBox(NULL, "hWnd = 0", "Error", 0);
      return FALSE;
   }
   
   //DWORD   dwStyle = GetWindowLong(hWnd, GWL_STYLE);
   //dwStyle &= ~(WS_SIZEBOX);
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                normal_quit = 1;
                
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
    case   WM_CLOSE:
        normal_quit = 1;
        break;
    case WM_DESTROY:
        //normal_quit = 1;
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
