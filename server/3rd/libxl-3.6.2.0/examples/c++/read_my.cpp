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

                cout << "firstRow:" << sheet->firstRow() << endl;
                cout << "lastRow:" << sheet->lastRow() << endl;
                cout << "firstCol:" << sheet->firstCol() << endl;
                cout << "lastCol:" << sheet->lastCol() << endl;

                cout << "lastUinit:" << sheet->readStr(sheet->lastRow()-1, sheet->lastCol()-1) << endl;

                const char* last = sheet->readStr(sheet->lastRow(), sheet->lastCol());
                if(NULL == last)
                {
                	cout << "last is empty!" << endl;
                }
//                int year, month, day;
//                book->dateUnpack(sheet->readNum(4, 1), &year, &month, &day);
//                cout << year << "-" << month << "-" << day << endl;
            }
        }

        book->release();
    }

    return 0;
}
