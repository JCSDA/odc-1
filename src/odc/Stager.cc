/*
 * (C) Copyright 1996-2013 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <ctype.h>
#include <fstream>

#include "eckit/config/Resource.h"
#include "eckit/utils/StringTools.h"
#include "eckit/io/MemoryHandle.h"

#include "odc/FileCollector.h"
#include "odc/FileMapper.h"
#include "odc/Partition.h"
#include "odc/Indexer.h"
#include "odc/Partitioner.h"
#include "odc/Writer.h"
#include "odc/Stager.h"
#include "odc/RequestUtils.h"

using namespace std;
using namespace eckit;
using namespace odc;
using namespace odc::tool;
using namespace odc::core;

void Stager::prepareMapper(FileMapper&                                            mapper,
                           const std::vector<std::string>&                        keywords,
                           const std::map<std::string,std::vector<std::string> >& request)
{
    vector<string> expandedRoots, roots (eckit::StringTools::split(":", request.at("odbserverroots")[0]));
    for (size_t i(0); i < roots.size(); ++i)
        expandedRoots.push_back(FileCollector::expandTilde(roots[i]));
    mapper.addRoots(expandedRoots);
    mapper.checkRoots();
}

size_t Stager::numberOfPartitions(const std::map<std::string,std::vector<std::string> >& request)
{
    vector<string> nParts ( request.at("n_parts") );
    if (nParts.size() != 1) 
        throw UserError(string("N_PARTS should have one value"));
    
    size_t n (atol(nParts[0].c_str()));
    if (n == 0)
        throw UserError ("STAGE: N_PARTS should be a positive integer, was '" + nParts[0] + "'");
    return n;
}

void Stager::createIndices(const std::vector<eckit::PathName>& files)
{
    Log::info() << "Creating indices for ";
    for (size_t i(0); i < files.size(); ++i)
        Log::info() << files[i] << "," << endl;
    Log::info() << endl;

    Indexer::createIndex(files); // for now for testing, indexing will be done some time earlier
    Log::info() << "Created indices." << endl;
}

vector<PathName> Stager::writePartitionsToFiles (const Partitions& partitions, const string& pathNamePrefix, const string& fileListPath)
{
    Log::info() << "Writing partitions: " << partitions << endl << " to files:" << endl;

    vector<PathName> ps (partitions.write(pathNamePrefix));

    ofstream f;
    f.exceptions(ofstream::badbit | ofstream::failbit);
    f.open(fileListPath.c_str());
    for (size_t i (0); i < ps.size(); ++i)
    {
        Log::info() << i << ": " << ps[i] << endl;
        f << ps[i] << endl;
    }
    f.close();

    Log::info() << "List of files with partitions written to: " << fileListPath << endl;

    return ps;
}

void Stager::stage(eckit::MultiHandle&                                    output,
                   const std::vector<std::string>&                        keywords,
                   const std::map<std::string,std::vector<std::string> >& req)
{
    odc::RequestDict request (odc::unquoteRequestValues(req));

    Log::info() << "STAGE: request: " << request << endl;

    odc::checkKeywordsHaveValues(request, keywords);
    const string partitionsInfoFile (FileCollector::expandTilde(request.at("partitionsinfo")[0]));

    FileMapper mapper (request.at("odbpathnameschema")[0]);
    prepareMapper(mapper, keywords, request);

    MultiHandle devNull;
    FileCollector fileCollector (mapper, devNull);
    fileCollector.findFiles(keywords, request);

    if(devNull.estimate() == Length(0))
        Log::userWarning() << "Data not found" << endl;

    vector<PathName> files (fileCollector.foundFilesAsPathNames());
    createIndices (files);

    Partitions partitions (Partitioner::createPartitions(files, numberOfPartitions(request)));
    size_t requestedNumberOfPartitions (numberOfPartitions(request));

    if (partitions.size() != requestedNumberOfPartitions)
        Log::warning() << "Number of partitions (" << partitions.size() 
            << ") different than requested: " << requestedNumberOfPartitions << std::endl;

    Log::info() << "Saving partitions info to " << partitionsInfoFile << endl;
    partitions.save(partitionsInfoFile);

    vector<PathName> dataFiles (writePartitionsToFiles (partitions, "odb.partition", partitionsInfoFile + ".files"));

    sendPartitionsInfo(output, partitions);
}

void Stager::sendPartitionsInfo(MultiHandle& output, const Partitions& partitions)
{
    Log::info() << "partitions: " << endl << partitions << endl;

    DataHandle* dh = new eckit::MemoryHandle(1024, true);
    dh->openForWrite(0);
    Writer<> out(*dh);
    Writer<>::iterator o (out.begin());
    MetaData md(o->columns());

    md.addColumn("partition_number", "INTEGER");
    md.addColumn("number_of_rows", "INTEGER");
    o->columns(md);
    o->writeHeader();

    for (size_t i(0); i < partitions.size(); ++i)
    {
        const Partition& p(partitions[i]);
        (*o)[0] = i;
        (*o)[1] = p.numberOfRows();
        ++o;
    }

    dh->openForRead();
    output += dh;
}

