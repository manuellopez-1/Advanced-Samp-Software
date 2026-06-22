#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <filesystem>
#include <cmath>
#include <algorithm>
#include <limits>

std::vector<std::string> listdirStart(std::string path, std::string start){
    std::vector<std::string> files;
    for (const auto &entry : std::filesystem::directory_iterator(path)){
        std::string name = entry.path().filename().string();
        if (name.rfind(start, 0)== 0){
            files.push_back(entry.path().filename().string()); //.filename() to remove ath
        }
    }
    return files;
}

std::vector<std::string> readlines(const std::string& path) {
    std::ifstream in(path);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)){
        lines.push_back(line);
    }
    return lines;
}


std::vector<double> piLister(std::string path, double delT, double lambda0, int direction, double& farthest,std::vector<double>& mfptlist, std::vector<int>& mfptsucc, int& tjcount, double currentlambda, double nextlambda){
    int bin;
    std::vector<std::string> lines = readlines(path + "/orderparameters.txt"); //find the largest reached order parameter
    std::string name; double value;
    char comma;
    for (std::string line:lines){
        std::stringstream stream(line);
        stream>>name>>comma>>value;
        if (direction*value>farthest*direction){
            farthest = value;
        }
    }
    int length = (int)std::round(direction*(farthest-lambda0)/delT)+1;
    std::vector<double> piList(length, 0.0);
    std::vector<std::string> colvarfiles = listdirStart(path,"C");

    int currentbin = (int) std::round(direction*(currentlambda-lambda0)/delT) + 1;
    int nextbin = (int) std::round(direction*(nextlambda-lambda0)/delT) + 1;
    int mfptListSize = nextbin -currentbin;
    mfptlist.assign(mfptListSize,0.0);
    mfptsucc.assign(mfptListSize, 0);
    std::vector<double> succlength(mfptListSize,0.0);
    std::vector<double> faillength(mfptListSize,0.0);
    std::vector<int> failCount(mfptListSize,0.0);
    tjcount = 0;

    for (std::string file:colvarfiles){
        bool first= true;
        double prevval = 0.0;
        std::vector<std::string> lines = readlines(path + "/" + file);
        double time, value;
        
        double starttime = 0.0;
        double lasttime = 0.0;
        int binmax = -1000;
        std::vector<double> firstTime(mfptListSize, -1.0);

        for (std::string line:lines){
            std::stringstream stream(line);
            stream>>time>>value;
            bin = (int)(direction*(value-lambda0)/delT);
            if (first){
                if (bin >= 0 && bin < length){
                    piList[bin] +=1.0;
                }
                prevval = value;
                first = false;
                
                starttime = time;
                lasttime = time;
                binmax = bin;

                int diff0 = std::min(std::max(bin-currentbin,0), mfptListSize);
                for (int l = 0; l<diff0; l++){
                    firstTime[l]= 0.0;
                }
                

                continue;
            }
            int prevbin = (int)(direction*(prevval-lambda0)/delT);
            int bindistance = bin-prevbin;
            if (std::abs(bindistance)<=1){
                if (bin >= 0 && bin < length){
                    piList[bin] +=1.0;
                }
            }else{
                int step = bindistance > 0 ? 1 : -1;
                for (int q = prevbin + step; q != bin + step; q += step) {
                    if (q >= 0 && q < length) {
                        piList[q] += 1.0 / std::abs(bindistance);
                    }
                }
            }
            prevval = value;

            lasttime= time;
            if (bin> binmax){
                binmax = bin;
            }
            int diff = std::min(std::max(bin - currentbin+1,0), mfptListSize); //changed this
            for (int l= 0; l<diff;l++){
                if (firstTime[l]<0){
                    firstTime[l] = time - starttime;
                }
            }
        }
        if (!first && mfptListSize>0){
            tjcount ++;
            double trajlength = lasttime-starttime;
            int diff = std::min(std::max(binmax-currentbin+1,0), mfptListSize); //changed this
            for (int l = diff; l<mfptListSize; l++){
                failCount[l]++;
                faillength[l] += trajlength;
            }
            for (int l = 0; l<diff; l++){
                mfptsucc[l]++;
                succlength[l] += firstTime[l];
            }
        } else if (!first){
            tjcount++;
        }
    }


    for (double& v:piList){
        v/= (double) tjcount;
    }


    for (int l = 0; l<mfptListSize; l++){
        if (mfptsucc[l]>0 ){
            mfptlist[l] = (succlength[l]+ faillength[l])/(double)mfptsucc[l];
        }else{
            mfptlist[l] = 0.0;
        }
    }

    return piList;
}

std::vector<double> crossingProbability(std::vector<std::string> Paths){
    std::vector<double> successes(Paths.size(),0.0);
    std::vector<double> totals( Paths.size(),0.0);

    for (size_t i = 0; i< Paths.size(); i++){
        std::vector<std::string> Lines = readlines(Paths[i] + "/path.txt");
        for (std::string line:Lines){
            if (line.find("success") != std::string::npos) {
                successes[i] +=1.0;
                totals[i] +=1.0;
            }
            if (line.find("failure")!= std::string::npos){
                totals[i] +=1.0;
            }
        }
    }
    std::vector<double> Probabilities(Paths.size(),0.0);
    for (size_t i = 0; i<Paths.size(); i++){
        if (totals[i] !=0){
            Probabilities[i] = successes[i]/totals[i];
        }
    }
    return Probabilities;
}

std::vector< double> PiScale(std::vector<double> PiList, double weight, double delT){
    std::vector<double> scaled(PiList.size(),0.0);
    for (size_t i = 0; i<PiList.size(); i++){
        scaled[i] = PiList[i] *weight; //delT?
    }
    return scaled;
}

std::vector<double> BasinProbabilityDensity(std::string path, double lambda0, double lambda1, double delT, int direction, std::vector<double>& mfptlist){
    
    int bin;
    int length = (int)std::round(direction*(lambda1 - lambda0) / delT)+1;
    std::vector<double> omega(length, 0.0);
    //iterate through all C files in the basin folder
    mfptlist.assign(length,0.0);
    int total = 0.0;  

    std::vector<std::string> colvarfiles = listdirStart(path, "C");
    for (std::string file:colvarfiles){
        std::vector<std::string> lines = readlines(path + "/" + file);
        double time, value;

        bool first = true;
        double prevval = 0.0;
        bool in = false;
        double starttime = 0.0;
        std::vector<double> firstTime;

        for (std::string line:lines){
            std::stringstream stream(line);
            stream>>time>>value;
            bin = (int)(direction*(value-lambda0)/delT);
            if (bin >= 0 && bin < length){
                omega[bin] +=1.0;
            }

            if (!first){
                int prevbin = (int)(direction*(prevval-lambda0)/delT);
                if (!in && prevbin<0 && bin>=0){ //past lambdaB
                    in = true;
                    starttime=time;
                    firstTime.assign(length, -1.0);
                    int reach = std::min(bin, length-1); //limit to within lambda0
                    for (int l = 0; l <= reach; l++) {
                        firstTime[l] = 0.0; //instantly got it
                    }

                }
                if (in){
                    int reach = std::min(std::max(0,bin), length-1);
                    for (int l = 0; l<=reach; l++){
                        if (firstTime[l]<0){
                            firstTime[l] = time - starttime;
                        }
                    }
                    if (bin>= length-1){
                        for (int l = 0; l<length; l++){
                            if (firstTime[l]<0){
                                firstTime[l] = time-starttime;
                            }
                            mfptlist[l] +=firstTime[l];
                        }
                        total++;
                        in=false;
                    }
                }
            }
            prevval=value;
            first = false;
        }
    }
    double omegatotal=0.0;
    for (double value:omega){
        omegatotal+=value;
    }
    for (double&v:omega){
        v/=(double) total;
    }
    if (total!=0){
        for (double& v:mfptlist){
            v /= (double) total;
        }
    }
    return omega;
}

void freeEnergy(std::vector<std::string> paths, std::vector<double> milestones, double delt){
    int direction =milestones[1]>milestones[0]?1:-1;
    std::vector<double> basinMFPT;
    std::vector<double> omega = BasinProbabilityDensity(paths[0], milestones[0], milestones[1],delt, direction, basinMFPT);
    double farthest = milestones[0];
    std::vector<std::string> ffspaths(paths.begin()+1, paths.end());
    std::vector<double> probs = crossingProbability(ffspaths);
    std::vector<std::vector<double>> allscaledpilists;
    double weight = 1.0;
    std::vector<std::vector<double>> allMFPT;
    std::vector<std::vector<int>> allMFPTsucc;
    std::vector<int> allTJCount;
    for (size_t i =1; i<paths.size(); i++){
        weight*= probs[i-1];
        std::vector<double> mfptList; std::vector<int> mfptsucc; int tjcount;
        std::vector<double> piList = piLister(paths[i], delt, milestones[0], direction, farthest, mfptList, mfptsucc, tjcount, milestones[i], milestones[i+1]);
        std::vector<double> scaledpiList =PiScale(piList, weight, delt);
        allscaledpilists.push_back(scaledpiList);
        allMFPT.push_back(mfptList);
        allMFPTsucc.push_back(mfptsucc);
        allTJCount.push_back(tjcount);
    }
    int length = (int)std::round(direction*(farthest-milestones[0])/delt)+1;
    std::vector<double> Merged(length, 0.0);
    for (size_t i= 0; i<omega.size(); i++){
        Merged[i] += omega[i];
    }
    for (size_t i = 0;i<allscaledpilists.size();i++){
        for (size_t I = 0; I<allscaledpilists[i].size(); I++){
            Merged[I] += allscaledpilists[i][I];
        }
    }
    std::vector<double> mfptListAll(length, 0.0);
    for (size_t j = 0; j < basinMFPT.size() && j < mfptListAll.size(); j++) {
        mfptListAll[j] = basinMFPT[j];
    }
    for (size_t i = 0; i < allMFPT.size(); i++) {
        int offset = (int)std::round(direction * (milestones[i + 1] - milestones[0]) / delt) + 1;
        for (size_t j = 0; j < allMFPT[i].size(); j++) {
            int idx = offset + (int)j;
            if (idx >= 0 && idx < (int)mfptListAll.size()) {
                if (offset > 0 && allMFPTsucc[i][j] > 0) {
                    mfptListAll[idx] =
                        allMFPT[i][j]
                        + (double)allTJCount[i] * mfptListAll[offset - 1] / (double)allMFPTsucc[i][j];
                } else {
                    mfptListAll[idx] = 0.0;
                }
            }
        }
    }
    std::vector<double> piListAll = Merged;
    std::vector<double> intPst(length, 0.0);
    std::vector<double> bFunc(length, 0.0);
    std::vector<double> intOneOverBFunc(length, 0.0);
    std::vector<double> FE(length, 0.0);

    intPst[length - 1] = piListAll[length - 1];

    for (int j = length - 2; j >= 0; j--) {
        intPst[j] = intPst[j + 1] + piListAll[j];
    }

    double intPstTotal = intPst[0];

    if (intPstTotal != 0.0) {
        for (int j = 0; j < length; j++) {
            intPst[j] /= intPstTotal;
            piListAll[j] /= intPstTotal;
        }
    }

    for (int j = 0; j < length; j++) {
        if (j == length - 1) {
            intPst[j] = 1.0;
        } else {
            intPst[j] = 1.0 - intPst[j + 1];
        }
    }
    int finalIdx = (int)std::round(direction * (milestones.back() - milestones[0]) / delt);

    double mfptFinal = 0.0;
    if (finalIdx >= 0 && finalIdx < (int)mfptListAll.size()) {
        mfptFinal = mfptListAll[finalIdx];
    } else {
        std::cerr << "ERROR: finalIdx out of range: " << finalIdx
                << " mfptListAll.size=" << mfptListAll.size() << "\n";
    }

    if (mfptFinal != 0.0 && piListAll[0] != 0.0) {
        bFunc[0] = piListAll[0] - mfptListAll[0] / mfptFinal;
        bFunc[0] /= piListAll[0];
    }

    for (int j = 1; j < length; j++) {
        if (mfptFinal != 0.0 && piListAll[j] != 0.0) {
            bFunc[j] = intPst[j] - mfptListAll[j] / mfptFinal;
            bFunc[j] /= piListAll[j];
        }
    }
    if (bFunc[0] > 0.0) {
        intOneOverBFunc[0] = 1.0 / bFunc[0];
    }

    for (int j = 1; j < length; j++) {
        if (bFunc[j] > 0.0) {
            intOneOverBFunc[j] = intOneOverBFunc[j - 1] + 1.0 / bFunc[j];
        } else {
            intOneOverBFunc[j] = std::numeric_limits<double>::quiet_NaN();
        }
    }
    for (int j = 0; j < length; j++) {
        if (bFunc[j] > 0.0) {
            FE[j] = std::log(bFunc[j]) - intOneOverBFunc[j];
        } else {
            FE[j] = std::numeric_limits<double>::quiet_NaN();
        }
    }
    std::ofstream feout("free_energy.txt");

    for (int j = 0; j < length; j++) {
        double lam = milestones[0] + direction * (j + 0.5) * delt;

        feout
            << lam << " "
            << FE[j] << " "
            << piListAll[j] << " "
            << mfptListAll[j] << " "
            << bFunc[j] << " "
            << intPst[j] << " "
            << intOneOverBFunc[j] << "\n";
    }

    feout.close();
}

int main(int argc, char* argv[]) {
    std::string base = "/gpfs/gibbs/pi/balou/mal258/";

    std::vector<std::string> paths = {base + "BasinOutputs",base + "FFS1Outputs",base + "FFS2Outputs",base + "FFS3Outputs",base + "FFS4Outputs",base + "FFS5Outputs",base + "FFS6Outputs",base + "FFS7Outputs",base + "FFS8Outputs"};

    std::vector<double> milestones = {5.8,6.1,6.3,6.5,6.77,7.0,7.28,7.5,7.7,8.5};

    double delT = 0.005;

    freeEnergy(paths, milestones, delT);

    return 0;
}