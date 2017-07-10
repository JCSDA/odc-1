#include "eckit/eckit_config.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/log/Timer.h"
#include "eckit/testing/Test.h"

#include "odb_api/Writer.h"
#include "odb_api/Select.h"
#include "odb_api/Reader.h"

#include <algorithm>

using namespace eckit::testing;


// ------------------------------------------------------------------------------------------------------

// A fixture to create a temporary ODB file.

class TemporaryODB {

    eckit::PathName path_;

public: // methods

    template <typename InitFunc>
    TemporaryODB(InitFunc f) :
        path_(eckit::PathName::unique("_temporary_select.odb")) {

        eckit::Timer t("Writing test.odb");
        odb::Writer<> oda(path_);
        odb::Writer<>::iterator writer = oda.begin();
        f(writer);
    }

    ~TemporaryODB() {
        path_.unlink();
    }

    const eckit::PathName& path() const { return path_; }
};


// ------------------------------------------------------------------------------------------------------

// TODO: Test with WHERE clause?

const Test specification[] = {

    CASE("Test reading with iterators") {
    SETUP("An odb file containing some pre-prepared data") {

        // Write (and clean up) a temporary ODB file
        TemporaryODB tmpODB([](odb::Writer<>::iterator& writer) {
            writer->setNumberOfColumns(3);

            writer->setColumn(0, "ifoo", odb::INTEGER);
            writer->setColumn(1, "nbar", odb::REAL);
            writer->setColumn(2, "string", odb::STRING);

            writer->writeHeader();

            for (size_t i = 1; i <= 10; i++) {
                writer->data()[0] = i; // col 0
                writer->data()[1] = i; // col 1
                ++writer;
            }
        });

        SECTION("Test select iterator for each") {

            odb::Select oda("select * from \"" + tmpODB.path() + "\";", tmpODB.path());

            long count = 0;
            std::for_each(oda.begin(), oda.end(), [&](odb::Select::row& row) {
                ++count;
                int64_t i = row[0];
                double n = row[1];
                EXPECT(i == count);
                EXPECT(int(n) == count);
            });

            EXPECT(count == 10);
        }


        SECTION("Test select data in explicit loop") {

            odb::Select oda("select * from \"" + tmpODB.path() + "\";", tmpODB.path());

            int count = 0;
            for (odb::Select::iterator it = oda.begin(); it != oda.end(); ++it) {
                ++count;
                EXPECT((*it)[0] == count);
                EXPECT((*it)[1] == count);
            }
            EXPECT(count == 10);
        }


        SECTION("Test read iterator for_each") {

            odb::Reader oda(tmpODB.path());

            long count = 0;
            std::for_each(oda.begin(), oda.end(), [&](odb::Reader::row& row) {
                ++count;
                int64_t i = row[0];
                double n = row[1];
                EXPECT(i == count);
                EXPECT(int(n) == count);
            });

            EXPECT(count == 10);
        }


        SECTION("Test read data in explicit loop") {

            odb::Reader oda(tmpODB.path());

            int count = 0;
            for (odb::Reader::iterator it = oda.begin(); it != oda.end(); ++it) {
                ++count;
                int i = (*it)[0];
                double d = (*it)[1];
                EXPECT(i == count);
                EXPECT(d == count);
            }
            EXPECT(count == 10);
        }
    }},


    CASE("Test bugfix 01, quote <<UnitTest problem fixed with p4 change 23687>>") {

        unsigned char REF_DATA[] = {
            0x0,0x0,0x0,0x60,0x9b,0x5e,0x41,0x40,  // 34.7391
            0x0,0x0,0x0,0x20,0xcc,0xf,0x11,0xc0,   // -4.26543
            0x0,0x0,0x0,0x60,0x66,0x46,0x6b,0x40,  // 218.2
            0x0,0x0,0x0,0x0,0x77,0xc8,0x3b,0x40,   // 27.7831
            0x0,0x0,0x0,0x80,0xed,0xb,0xef,0xbf,   // -0.970206
            0x0,0x0,0x0,0xc0,0xcc,0xcc,0x6b,0x40,  // 222.4
            0x0,0x0,0x0,0x80,0x6,0xff,0x48,0x40,   // 49.9924
            0x0,0x0,0x0,0x80,0x81,0xec,0xeb,0x3f,  // 0.87262
            0x0,0x0,0x0,0x60,0x66,0x26,0x6b,0x40,  // 217.2
            0x0,0x0,0x0,0x0,0xc7,0x18,0x45,0x40,   // 42.1936
            0x0,0x0,0x0,0xc0,0x56,0x91,0xe7,0x3f,  // 0.736492
            0x0,0x0,0x0,0xc0,0xcc,0xec,0x6a,0x40,  // 215.4
            0x0,0x0,0x0,0x0,0xc7,0x18,0x45,0x40,   // 42.1936
            0x0,0x0,0x0,0xc0,0x56,0x91,0xe7,0xbf,  // -0.736492
            0x0,0x0,0x0,0xc0,0xcc,0xec,0x6a,0x40,  // 215.4
            0x0,0x0,0x0,0xa0,0xa5,0x7c,0x45,0x40,  // 42.9738
            0x0,0x0,0x0,0x40,0xc7,0x2,0xf8,0xbf,   // -1.50068
            0x0,0x0,0x0,0x60,0x66,0xe6,0x6a,0x40,  // 215.2
            0x0,0x0,0x0,0x60,0xf9,0xcb,0x45,0x40,  // 43.5935
            0x0,0x0,0x0,0x80,0x9,0x63,0x8,0xc0,    // -3.04836
            0x0,0x0,0x0,0x60,0x66,0xe6,0x6a,0x40,  // 215.2
            0x0,0x0,0x0,0x40,0xa3,0x22,0x36,0x40   // 22.1353
        };

#ifdef EC_BIG_ENDIAN
        // Swap the data on this big-endian box.
        for (int i = 0; i < sizeof(REF_DATA) / 8; i++)
        {
            for (int j = 0; j < 4; j++)
                swap(REF_DATA[i * 8 + j], REF_DATA[(i + 1) * 8 - 1 - j]);
            //cout << *(reinterpret_cast<double *>(&REF_DATA[i * 8])) << std::endl;
        }
#endif

        const double *OBSVALUE = reinterpret_cast<const double*>(REF_DATA);

        odb::Select oda("select obsvalue from \"2000010106.odb\";");

        // Only consider the first section of the file...
        size_t count = 0;
        for (odb::Select::iterator it = oda.begin();
            it != oda.end() && count < sizeof(REF_DATA)/sizeof(double);
            ++it, ++count) {

//            eckit::Log::info() << "testBug01: it[" << count << "]=" << (*it)[0] << ", should be " << OBSVALUE[count] << std::endl;
            EXPECT((*it)[0] == OBSVALUE[count] );
        }
    },


    CASE("Test bugfix 02, quote <<UnitTest problem fixed with p4 change 23687>>") {

        const std::vector<double> VALUE{1, 2, 3};

        // Write (and clean up) a temporary ODB file
        TemporaryODB tmpODB([&](odb::Writer<>::iterator& writer) {
            writer->setNumberOfColumns(1);

            writer->setColumn(0, "value", odb::INTEGER);
            writer->writeHeader();

            for (size_t i = 0; i < VALUE.size(); ++i) {
                (*writer)[0] = VALUE[i];
                ++writer;
            }
        });

        odb::Select oda("select * from \"" + tmpODB.path() + "\";", tmpODB.path());

        size_t count = 0;
        for (odb::Select::iterator it = oda.begin(); it != oda.end(); ++it, ++count) {
//            eckit::Log::info() << "testBug02: it[" << count << "]=" << (*it)[0] << ", should be " << VALUE[count] << std::endl;
            EXPECT((*it)[0] == VALUE[count]);
        }

        EXPECT(count == 3);
    }
};

// ------------------------------------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    return run_tests(specification, argc, argv);
}

