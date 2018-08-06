#ifndef PARSER_FILE_H
#define PARSER_FILE_H

class CParserFile {
        int fd;
        int Level;
        char Buffer[500];

        void SaveOld(void);
        char* GetNumSp(void);

        int WordPos;
        char* GetWord(char*, int&);

        bool StrListOut;
    public:
        CParserFile();
        ~CParserFile();

        bool Open(const char*, bool);
        void Close(void);

        void StartLevel(void);
        void AddCommand(const char*, ...);
        void AddString(const char*, char*);
        void AddScripts(const char*, int*, int);

        void EndLine(void);
        int GetLevel(void);
        void NextLevel(void);
        void CloseStruct(void);

        void NewStrList(void);
        void AddStrList(const char*, char*);
};

#endif
