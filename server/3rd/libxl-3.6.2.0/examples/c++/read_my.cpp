#include <iostream>
#include "libxl.h"

using namespace libxl;
using namespace std;

int main()
{
    Book* book = xlCreateBook();
    if(book)
    {
        if(book->load("new.xls"))
        {
            Sheet* sheet = book->getSheet(0);
            if(sheet)
            {
            	const char* s0 = sheet->readStr(0, 0);
            	if(s0) cout << s0 << endl;

                const char* s = sheet->readFormula(1, 1);
                if(s) cout << s << endl;

                double d = sheet->readNum(3, 1);
                cout << d << endl;

                int h = sheet->lastRow();
                cout << h << endl;

//                int year, month, day;
//                book->dateUnpack(sheet->readNum(4, 1), &year, &month, &day);
//                cout << year << "-" << month << "-" << day << endl;
            }
        }

        book->release();
    }

    return 0;
}
