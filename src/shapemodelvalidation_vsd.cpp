
/*
* Created by Marcel Luethi
* Copyright (c) 2011 University of Basel
*
* Licensed under the BSD, 3-Clause license
*
**/



#include "shapemodelvalidation.h"

#include "itkDirectory.h"

#include <iostream>
#include <string>
#include <list>
#include "logger.h"


void writeResult(const char* resultfile, unsigned objId, const char* modelFn, unsigned errorCode, const std::string& errorMsg, const GeneralizationResult& gndummy, float specificity, float compactness) {
    std::ofstream resFile(resultfile);

    if (!resFile) {
        std::ostringstream msgos;
        msgos << "cannot create output file " << resultfile;
        throw std::runtime_error(msgos.str().c_str());
    }

    resFile << "{" << std::endl;
    resFile << "\"object-id\" : " << objId << "," << std::endl;
    resFile << "\"shape-model\" : " << "\"" << modelFn << "\"" << "," << std::endl;
    resFile << "\"error-code\" : " << errorCode << "," << std::endl;
    resFile << "\"error-message\" : " << "\"" << errorMsg << "\"" << "," << std::endl;
    resFile << "\"generalization\" : {" << std::endl;
    resFile << "\t\"average-distance\" : " << gndummy.averageDistance << "," << std::endl;
    resFile << "\t\"hausdorff-distance\" : " << gndummy.hausdorffDistance <<  std::endl;
    resFile << "}," << std::endl;
    resFile << "\"specificity\" : " << specificity << "," << std::endl;
    resFile << "\"compactness\" : " << compactness << std::endl;
    resFile << "}" << std::endl;
    resFile.close();
}


int main(int argc, char* argv[]) {

    if (argc < 6) {
        std::cout << "usage: shapeModelValidation obj-id shapemodel testdatadir resultfile logfile" << std::endl;
        return -1;
    }

    unsigned int objId = static_cast<unsigned>(atoi(argv[1]));
    char* modelFn = argv[2];
    char* testdatadir = argv[3];
    char* resultfile = argv[4];
    char* logfile = argv[5];

    try {

        Logger logger(logfile, logDEBUG);

        FileList testfiles = getTestImagesInDir(testdatadir);
        TestImageList testImages;

        for (FileList::const_iterator it = testfiles.begin(); it != testfiles.end(); ++it) {
            std::string filename = std::string(testdatadir) +"/" + *it;
            logger.Get(logINFO) << "reading image " << filename << std::endl;
            BinaryImageType::Pointer testImage = readBinaryImage(filename);
            testImages.push_back(testImage);
        }

        logger.Get(logINFO) << "Reading statistical model " << modelFn << std::endl;

        RepresenterType::Pointer representer = RepresenterType::New();
        StatisticalModelType::Pointer model = StatisticalModelType::New();
        model->Load(representer, modelFn);

        GeneralizationResult generalizationScore = generalization(logger, model, testImages);
        logger.Get(logINFO) << "generalizationScore: avg = " << generalizationScore.averageDistance << " hd = " << generalizationScore.hausdorffDistance << std::endl;

        float specificityValue = specificity(logger, model, 1000);
        logger.Get(logINFO) << "specificity value: " << specificityValue << std::endl;

        float compactnessScore = compactness(logger, model);
        logger.Get(logINFO) << "compactness score: " << compactnessScore  << std::endl;

        writeResult(resultfile, objId, modelFn, 0, "", generalizationScore, specificityValue, compactnessScore);

    } catch (std::exception& e) {
        std::cerr << "An error occurred : " << e.what() << std::endl;
        writeResult(resultfile, objId, modelFn, 1, e.what(), GeneralizationResult(0, 0), 0, 0);
        return -1;
    }



    return 0;
}

