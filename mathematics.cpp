#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <filesystem>
#include <cmath>


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


std::vector<double> piLister(std::string path, double delT, double lambda0, int direction, double& farthest){
    int bin;
    std::vector<std::string> lines = readlines(path + "/orderparameters.txt"); //find the largest reached order parameter
    farthest=lambda0;
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
    for (std::string file:colvarfiles){
        bool first= true;
        double prevval = 0.0;
        std::vector<std::string> lines = readlines(path + "/" + file);
        double time, value;
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
        }
    }
    double total=0.0;
    for (double value:piList){
        total+=value;
    }
    for (double&v:piList){
        v/=total;
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
        scaled[i] = PiList[i] *weight /delT;
    }
    return scaled;
}

std::vector<double> BasinProbabilityDensity(std::string path, double lambda0, double lambda1, double delT, int direction, std::vector<double>& mfptlist){
    
    int bin;
    int length = (int)std::round(direction*(lambda1 - lambda0) / delT)+1;
    std::vector<double> omega(length, 0.0);
    //iterate through all C files in the basin folder
    mfptlist.assign(length,0.0)
    int total = 0;  

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
                if (!in && prevbin<0 && bin>0){ //past lambdaB
                    in = true;
                    starttime=time;
                    firstTime.assing(length, -1.0);
                    int reach = std::min(bin, length-1); //limit to within lambda0
                    for (int l = 0; l <= reached; l++) {
                        firstTime[l] = 0.0; //instantly got it
                    }

                }
                if (in){
                    int reach = std::min(std::max(0,bin), length-1);
                    for (int l = 0; l<=reach; l++){
                        if (firstTime[l]<0){
                            firsttime[l] = time - starttime;
                        }
                    }
                    if (bin>= length-1){
                        for (int l = 0; l<length; l++1){
                            if (firstTime[l]<0){
                                firsttime[l] = time-starttime
                            }
                            mfptlist[l] +=firsttime[l]
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
    double total=0.0;
    for (double value:omega){
        total+=value;
    }
    for (double&v:omega){
        v/=total;
    }
    if (total!=0){
        for (double v:mfptlist){
            v /= (double) total
        }
    }
    return omega;
}

void freeEnergy(std::vector<std::string> paths, std::vector<double> milestones, double delt){
    int direction =milestones[1]>milestones[0]?1:-1;
    std::vector<double> omega = BasinProbabilityDensity(paths[0], milestones[0], milestones[1],delt, direction);
    double farthest = milestones[0];
    std::vector<std::string> ffspaths(paths.begin()+1, paths.end());
    std::vector<double> probs = crossingProbability(ffspaths);
    std::vector<std::vector<double>> allscaledpilists;
    double weight = 1.0;
    for (size_t i =1; i<paths.size(); i++){
        weight*= probs[i-1];
        std::vector<double> piList = piLister(paths[i], delt, milestones[0], direction, farthest);
        std::vector<double> scaledpiList =PiScale(piList, weight, delt);
        allscaledpilists.push_back(scaledpiList);
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
    std::ofstream out("merged_pi.txt");

    for (size_t j = 0; j < Merged.size(); j++) {
        double lambda_mid = milestones[0] + direction * (j + 0.5) * delt;
        out << lambda_mid << " " << Merged[j] << "\n";
    }

    out.close();
    

}

int main(int argc, char* argv[]) {
    std::string base = "/gpfs/gibbs/pi/balou/mal258/";

    std::vector<std::string> paths = {
        base + "BasinOutputs",
        base + "FFS1Outputs",
        base + "FFS2Outputs"
    };

    std::vector<double> milestones = {
        5.8,
        6.1,
        6.3,
        6.5
    };

    double delT = 0.005;

    freeEnergy(paths, milestones, delT);

    std::cout << "Finished freeEnergy calculation.\n";
    std::cout << "Output written to merged_pi.txt\n";

    return 0;
}