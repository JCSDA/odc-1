/*
 * (C) Copyright 1996-2012 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <memory>

#include "eckit/testing/Test.h"

#include "odc/api/odc.h"

using namespace eckit::testing;

// Specialise custom deletion for odb_t

#define CHECK_RETURN(x) EXPECT((x) == ODC_SUCCESS)

namespace std {
template <> struct default_delete<odc_reader_t> {
    void operator() (const odc_reader_t* reader) { CHECK_RETURN(odc_close(reader)); }
};

template <> struct default_delete<odc_frame_t> {
    void operator() (const odc_frame_t* frame) { CHECK_RETURN(odc_free_frame(frame)); }
};

template <> struct default_delete<odc_decoder_t> {
    void operator() (odc_decoder_t* t) { CHECK_RETURN(odc_free_decoder(t)); }
};
}

// ------------------------------------------------------------------------------------------------------

CASE("Count lines in an existing ODB file") {

    odc_reader_t* reader = nullptr;
    CHECK_RETURN(odc_open_path(&reader, "../2000010106.odb"));
    std::unique_ptr<odc_reader_t> reader_deleter(reader);

    odc_frame_t* frame = nullptr;
    CHECK_RETURN(odc_new_frame(&frame, reader));
    std::unique_ptr<odc_frame_t> frame_deleter(frame);

    size_t ntables = 0;
    size_t totalRows = 0;

    int ierr;
    while ((ierr = odc_next_frame(frame)) == ODC_SUCCESS) {

        long nrows;
        CHECK_RETURN(odc_frame_row_count(frame, &nrows));
        totalRows += nrows;

        int ncols;
        CHECK_RETURN(odc_frame_column_count(frame, &ncols));
        EXPECT(ncols == 51);

        ++ntables;
    }

    EXPECT(ierr == ODC_ITERATION_COMPLETE);
    EXPECT(ntables == 333);
    EXPECT(totalRows == 3321753);
}

// ------------------------------------------------------------------------------------------------------

CASE("Decode an entire ODB file") {


//    std::unique_ptr<odb_t> o(odc_open_path("../2000010106.odb"));
//
//    size_t ntables = 0;
//
//    std::unique_ptr<odb_frame_t> table;
//    while (table.reset(odc_alloc_next_frame(o.get())), table) {
//
//        std::unique_ptr<const odb_decoded_t> decoded(odc_frame_decode_all(table.get()));
//        EXPECT(decoded->nrows == odc_frame_row_count(table.get()));
//        EXPECT(decoded->ncolumns == 51);
//
//        ++ntables;
//    }
}

// ------------------------------------------------------------------------------------------------------

CASE("Decode an entire ODB file preallocated data structures") {
//
//    std::unique_ptr<odb_t> o(odc_open_path("../2000010106.odb"));
//
//    int ntables = odc_num_frames(o.get());
//    EXPECT(ntables == 333);
//
//    odb_decoded_t decoded;
//    odb_strided_data_t strided_data[51];
//
//    for (int i = 0; i < ntables; i++) {
//
//        std::unique_ptr<odb_frame_t> table(odc_get_frame(o.get(), i));
//
//        ASSERT(odc_frame_column_count(table.get()) == 51);
//
//        decoded.ncolumns = 51;
//        decoded.nrows = 10000;
//        decoded.columnData = strided_data;
//
//        ///   odc_frame_decode(table.get(), &decoded);
//
//        ///EXPECT(decoded.nrows == odc_frame_row_count(table.get()));
//        ///EXPECT(decoded.ncolumns == 51);
//
//        ///eckit::Log::info() << "Decoded: ncolumns = " << decoded.ncolumns << std::endl;
//        ///eckit::Log::info() << "Decoded: nrows = " << decoded.nrows << std::endl;
//        ///eckit::Log::info() << "Decoded: data = " << decoded.columnData << std::endl;
//    }
}

// ------------------------------------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    return run_tests(argc, argv);
}
