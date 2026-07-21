#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <limits>

//steps to build the free energy, part 1: calculate the MFPT for basin


int binfinder(double lambda, double lambda0, double delT, int dir){
    int bin =std::floor(dir*(lambda-lambda0)/delT);
    return bin;
}

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

void basindensity(std::vector<double>& density, std::vector<double>& MFPT, double lambda0, double lambdaB, double delT,std::string path){
    //step one make an array for density and MFPT
    int dir = lambda0>lambdaB ? 1 : -1; 
    int length = binfinder(lambda0,lambdaB, delT, dir)+1;
    density.assign(length, 0.0);
    MFPT.assign(length, 0.0);  
    std::vector<int> thisTjMfpt (length); 
    std::vector<std::string> colvars = listdirStart(path, "C");
    std::string line;
    double time, value, prev;
    int firsttime = 0, totaltrajs=0;
    for (std::string colvar:colvars){
        std::ifstream colvarstream(path + "/" +colvar );
        bool first = true, reset = false;
        int index = 0;
        while (std::getline(colvarstream, line)){
            if (line.empty() || line[0] == '#' || line[0] == '@') {
                continue;
            }
            std::stringstream stream(line); //collect order parameter per line
            stream>>time>>value;
            if (first ==true){
                prev = value; //start keeping track of prev
                first =false;
            }
            
            if (!reset && (value*dir>lambdaB*dir) && (prev*dir<=lambdaB*dir)){
                reset = true;
                firsttime = index-1;
                thisTjMfpt[0] = 0;
                for (int l = 1; l<length; l++){
                    thisTjMfpt[l]=-1;
                }
            }

            if (reset){
                int thisbin = binfinder(value, lambdaB, delT,dir);
                int lastbin = binfinder(prev, lambdaB, delT,dir);
                int range = std::min(thisbin, length-1);

                for (int l = 0; l <= range; l++){
                    if (thisTjMfpt[l] ==-1){
                        thisTjMfpt[l] =index - firsttime;
                    }
                }

                if (value*dir >=lambdaB*dir && value*dir<=lambda0*dir){
                    int thisbin = binfinder(value, lambdaB, delT,dir);
                    int lastbin = binfinder(prev, lambdaB, delT,dir);
                    if (std::abs(thisbin-lastbin)<=1){
                        density[thisbin] +=1;
                    }else if (thisbin-lastbin>1){ //smear it inbetween
                        for (int q = lastbin+1; q<=thisbin; q++){
                            if (q>=0){
                                density[q] += 1.0 / (double)std::abs(thisbin - lastbin);
                            }
                        }
                    }else{ //negative case
                        for (int q = thisbin; q<=lastbin-1; q++){
                            if (q>=0){
                                density[q] += 1.0 / (double)std::abs(thisbin - lastbin);
                            }
                        }
                    }
                }
            }

            if (reset && (value*dir>=lambda0*dir)){
                reset = false;
                for (int l = 0;l <length; l++){
                    if (thisTjMfpt[l] ==-1){
                        thisTjMfpt[l] = index - firsttime;
                    }
                    MFPT[l] += thisTjMfpt[l];
                }
                totaltrajs++;
            }
            prev = value;
            index++;
        }

    }
    for (size_t i = 0;i<density.size(); i++){
        density[i] /= (double)totaltrajs;
        MFPT[i] /= (double)totaltrajs;
    }
}
static std::string lastdataline(const std::string& colvar) { //faster way to get the last line of a colvar file, used as a lammps speedboost
    std::ifstream in(colvar, std::ios::binary | std::ios::ate);
    if (!in) return "";
    std::streamoff pos = static_cast<std::streamoff>(in.tellg());
    if (pos == 0) return "";
    std::string line;
    while (pos > 0) {
        in.seekg(--pos);
        char c = static_cast<char>(in.peek());
        if (c != '\n' && c != '\r' && c != ' ') break;
    }
    while (pos > 0) {
        in.seekg(--pos);
        char c = static_cast<char>(in.peek());
        if (c == '\n' || c == '\r') { ++pos; break; }
    }
    in.seekg(pos);
    std::getline(in, line);
    if (line.empty() || line[0] == '#') return "";
    return line;
}

std::pair<double,double> lastorderparameter(std::string colvar){ //so truthfully this need not be its own function, however, there's a good chance that LAMMPS and plumed print op in very different ways, this will help provide some uniformity
    std::string lastline = lastdataline(colvar);
    std::stringstream stream(lastline);
    double time, value;
    stream>>time>>value;
    return {time, value};
}

void milestone(std::vector<double>& density, std::vector<double>& MFPT, std::vector<int>& mfptSuccCount, int& tjCount, double lBasin, double lCurrent, double lNext, std::string path,double delT, int& totalsuccesses){
    totalsuccesses=0;
    int dir = lCurrent>lBasin ? 1 : -1; 
    int densitylength = binfinder(lNext,lBasin, delT,dir)+1;
    int MFPTlength = binfinder(lNext, lBasin, delT, dir) - binfinder(lCurrent, lBasin, delT, dir);
    density.assign(densitylength, 0.0);
    MFPT.assign(MFPTlength, 0.0);  
    mfptSuccCount.assign(MFPTlength, 0);

    std::vector<double> mfptSuccLength(MFPTlength, 0.0);
    std::vector<double> mfptFailLength(MFPTlength, 0.0);
    std::vector<int> mfptFailCount(MFPTlength, 0);

    std::string line;
    
    
    std::vector<std::string> colvars = listdirStart(path, "C");
    std::vector<double> lambdaMax(colvars.size(), -1000000.0);
    double time, value, prev;
    int colvarindex=-1;// so that we always increment
    tjCount = 0;
    for (std::string colvar:colvars){
        colvarindex++;
        auto result =lastorderparameter(path + "/"+ colvar);
        double c = result.second;
        if (dir*lBasin<c*dir && dir*c<dir*lNext){
            continue;
        }

        std::ifstream colvarstream(path + "/" +colvar );
        bool first = true, reset = false;
        int index = 0;

        while (std::getline(colvarstream, line)){
            if (line.empty() || line[0] == '#' || line[0] == '@') {
                continue;
            }
            std::stringstream stream(line); //collect order parameter per line
            stream>>time>>value;
            if (value*dir>dir*lambdaMax[colvarindex]){
                lambdaMax[colvarindex] = value;
            }
            if (first ==true){
                prev = value; //start keeping track of prev
                first =false;
            }
            if (lBasin*dir<value*dir && value*dir<lNext*dir){
                int thisbin = binfinder(value, lBasin, delT,dir);
                int lastbin = binfinder(prev, lBasin, delT,dir);
                if (std::abs(thisbin-lastbin)<=1){
                    density[thisbin] ++;
                }else if (thisbin-lastbin>1){ //smear it inbetween
                    for (int q = lastbin+1; q<=thisbin; q++){
                        if (q>=0){
                            density[q] += 1.0 / (double)std::abs(thisbin - lastbin);
                        }
                    }
                }else{ //negative case
                    for (int q = thisbin; q<=lastbin-1; q++){
                        if (q>=0){
                            density[q] += 1.0 / (double)std::abs(thisbin - lastbin);
                        }
                    }
                }
            }
            prev = value;
        }
    }
    colvarindex = -1;
    for (std::string colvar:colvars){
        colvarindex++;
        auto result =lastorderparameter(path +"/" + colvar);
        double c = result.second;
        if (dir*lBasin<c*dir && dir*c<dir*lNext){
            continue;
        }
        if (lambdaMax[colvarindex]* dir>lNext*dir){
            totalsuccesses++;
        }
        if (lambdaMax[colvarindex]*dir<lBasin*dir){
            std::ifstream colvarstream1(path + "/" +colvar );
            int fileLength = 0;
            while (std::getline(colvarstream1, line)) {
                if (line.empty() || line[0] == '#' || line[0] == '@') {
                    continue;
                }
                std::stringstream stream(line);
                if (!(stream >> time >> value)) continue;
                fileLength = time;
            }
            for (int l =0; l<MFPTlength; l++){
                mfptFailCount[l]++;
                mfptFailLength[l] += (double) fileLength-1;
            }
            tjCount++;
            continue;
        }
        tjCount ++;
        int binMax = binfinder(lambdaMax[colvarindex], lCurrent, delT,dir);
        //int delta = std::min(binMax, MFPTlength);
        int delta = std::max(0, std::min(binMax, MFPTlength));
        int fileLength = 0;
        std::ifstream colvarstream2(path + "/" + colvar);
        while (std::getline(colvarstream2, line)) {
            if (line.empty() || line[0] == '#' || line[0] == '@') {
                continue;
            }
            std::stringstream stream(line);
            if (!(stream >> time >> value)) continue;
            fileLength = time;
        }
        for (int l = delta; l<MFPTlength; l++){
            mfptFailLength[l]+= (double) fileLength-1;
            mfptFailCount[l]++;
        }
        for (int l =0; l<delta; l++){
            mfptSuccCount [l]++;
        }
        if (delta<=0) continue;
        //int lengthThisTJMFPT = binfinder(lambdaMax[colvarindex],lCurrent, delT ,dir);
        //std::vector<double> thisTjMfpt(lengthThisTJMFPT, -1.0);
        std::vector<double> thisTjMfpt(MFPTlength, -1.0);
        int index = 0;
        std::ifstream colvarstream3(path + "/" +colvar );
        bool first = true, reset = false;

        while (std::getline(colvarstream3, line)){
            if (line.empty() || line[0] == '#' || line[0] == '@') {
                continue;
            }
            std::stringstream stream(line); 
            if (!(stream >> time >> value)) {
                std::cout<<"issue"<<std::endl;
                continue;
            }
            if (value*dir>dir*lCurrent){
                int bin = binfinder(value,lCurrent, delT ,dir);
                bin = std::max(0, std::min(bin, MFPTlength));
                for (int l=0;l< bin;l++){
                    if (thisTjMfpt[l]==-1){
                        thisTjMfpt[l] = index;
                    }
                }
                if (dir*value>=std::min(dir*lambdaMax[colvarindex], lNext*dir)){
                    for (int l = 0; l<bin; l++){
                        mfptSuccLength[l] += thisTjMfpt[l];
                    }
                    break;
                }
            }
            index++;

        }
    }
    for (size_t i=0; i<densitylength; i++){
        density[i] /= (double) tjCount;
    }
    for (int i = 0;i<MFPTlength; i++){
        MFPT[i] = (mfptSuccLength[i] + mfptFailLength[i])/(double) mfptSuccCount[i];
    }
}




void FreeEnergyCalculation(std::vector<std::string> filepaths, std::vector<double> Milestones, double delT,std::string OutputName){
    int dir = Milestones[1]>Milestones[0] ? 1 : -1; 
    int totalsuccesses=0;
    int tjCount=0;
    std::vector<int> mfptSuccCount;
    std::vector<double> cummprobs(filepaths.size(),1.0);
    std::vector<std::vector<double>> alldensities;
    std::vector<double> density;
    std::vector<double> MFPT;
    int allMFPTlength = binfinder(Milestones.back(), Milestones[0],delT,dir)+1;
    std::vector<double> AllMFPT(allMFPTlength,0.0);
    basindensity(density, MFPT, Milestones[1], Milestones[0], delT,filepaths[0]);
    alldensities.push_back(density);
    for (size_t i = 0; i<MFPT.size(); i++){
        AllMFPT[i] += MFPT[i];
    }
    int i=3;
    milestone(density, MFPT, mfptSuccCount, tjCount,  Milestones[0],  Milestones[i],  Milestones[i+1], filepaths[i], delT,totalsuccesses);
    /*
    for (size_t i = 1; i< Milestones.size()-1; i++){
        std::cout<<"Trying "<<i<<std::endl;
        milestone(density, MFPT, mfptSuccCount, tjCount,  Milestones[0],  Milestones[i],  Milestones[i+1], filepaths[i], delT,totalsuccesses);
        std::cout<<"Completed "<<i<<std::endl;
        //cummprobs[i] *= cummprobs[i-1]*totalsuccesses/(double)tjCount;
        cummprobs[i] *= cummprobs[i-1]*(double)mfptSuccCount.back()/(double)tjCount;
        int offset = binfinder(Milestones[i], Milestones[0],delT,dir)+1;
        for (size_t j = 0; j<MFPT.size();j++){
            AllMFPT[offset + j] = MFPT[j] +tjCount*AllMFPT[offset-1]/(double)mfptSuccCount[j];
        }
        alldensities.push_back(density);
    }
    std::cout<<"calculating pi list"<<std::endl;
    std::vector<double> piListAll(alldensities.back().size(), 0.0); //data manipulation time
    for (int i = alldensities.size()-1; i>=1; i--){
        for (size_t j = 0; j<alldensities[i].size(); j++){
            piListAll[j] += cummprobs[i-1] *alldensities[i][j];
        }
    }
    //add basin
    std::cout<<"Adding basin"<<std::endl;
    for (size_t j = 0; j<alldensities[0].size(); j++){
        piListAll[j] += alldensities[0][j];
    }
    std::cout<<"intPST1"<<std::endl;
    std::vector<double> intPst(allMFPTlength,0.0);
    intPst[allMFPTlength-1] = piListAll[allMFPTlength-1];
    for (int j = allMFPTlength-2; j>=0; j--){
        intPst[j] = intPst[j+1] + piListAll[j];
    }
    std::cout<<"intPST2"<<std::endl;
    double intPstTotal = intPst[0];
    for (size_t j = 0; j<intPst.size(); j++){
        intPst[j] /= intPstTotal;
        piListAll[j] /= intPstTotal;
    }
    std::cout<<"intPST3"<<std::endl;
    for (size_t j = 0; j<intPst.size(); j++){
        if (j == (intPst.size()-1)){
            intPst[j] = 1.0;
        }else{
            intPst[j] = 1.0 - intPst[j + 1];
        }
    }
    std::cout<<"bfunc0"<<std::endl;
    std::vector<double> bFunc(allMFPTlength,0.0);
    std::vector<double> intOneOverbFunc(allMFPTlength,0.0);
    bFunc[0] = piListAll[0] - AllMFPT[0] / AllMFPT.back();
    bFunc[0] /= piListAll[0];
    std::cout<<"bfunc1"<<std::endl;
    for (size_t j = 1; j < AllMFPT.size(); j++) {
        bFunc[j] = intPst[j] - AllMFPT[j] / AllMFPT.back();
        bFunc[j] /= piListAll[j];
    }
    std::cout<<"bfunc2"<<std::endl;
    intOneOverbFunc[0] = 1.0 /bFunc[0];
    for (size_t j = 1; j<allMFPTlength; j++){
        intOneOverbFunc[j] = intOneOverbFunc[j-1] + 1.0/bFunc[j];
    }
    std::cout<<"bfunc3"<<std::endl;
    std::vector<double> FreeEnergy(allMFPTlength, 0.0);
    for (size_t j = 0; j<allMFPTlength; j++){
        FreeEnergy[j] = std::log(bFunc[j])- intOneOverbFunc[j];
    }
    
    std::cout<<"print"<<std::endl;

    //print block

    std::ofstream out(OutputName);

    out << std::scientific << std::setprecision(15);

    // Print milestone-level cumulative probabilities first.
    out << "# cummprobs";
    for (size_t i = 0; i < cummprobs.size(); i++) {
        out << "\tcummprob_" << i << "=" << cummprobs[i];
    }
    out << "\n";

    // Main per-bin table.
    out << "j"
        << "\tlambda"
        << "\tpiListAll"
        << "\tAllMFPT"
        << "\tAllMFPT_norm"
        << "\tintPst"
        << "\tbFunc"
        << "\tintOneOverbFunc"
        << "\tFreeEnergy";

    for (size_t i = 0; i < alldensities.size(); i++) {
        out << "\tdensity_" << i;
    }
    out << "\n";


    for (size_t j = 0; j < AllMFPT.size(); j++) {
        double lambda = Milestones.front() + dir * delT * (double)j;

        out << j
            << "\t" << lambda
            << "\t" << piListAll[j]
            << "\t" << AllMFPT[j]
            << "\t" << AllMFPT[j] / AllMFPT.back()
            << "\t" << intPst[j]
            << "\t" << bFunc[j]
            << "\t" << intOneOverbFunc[j]
            << "\t" << FreeEnergy[j];

        for (size_t i = 0; i < alldensities.size(); i++) {
            if (j < alldensities[i].size()) {
                out << "\t" << alldensities[i][j];
            } else {
                out << "\t" << "";
            }
        }

        out << "\n";
    }

    out.close();
    */
}


int main(int argc, char** argv) { 
    //std::vector<std::string> filepaths = {"/gpfs/gibbs/pi/balou/mal258/BasinOutputs","/gpfs/gibbs/pi/balou/mal258/FFS1Outputs","/gpfs/gibbs/pi/balou/mal258/FFS2Outputs","/gpfs/gibbs/pi/balou/mal258/FFS3Outputs","/gpfs/gibbs/pi/balou/mal258/FFS4Outputs","/gpfs/gibbs/pi/balou/mal258/FFS5Outputs","/gpfs/gibbs/pi/balou/mal258/FFS6Outputs","/gpfs/gibbs/pi/balou/mal258/FFS7Outputs","/gpfs/gibbs/pi/balou/mal258/FFS8Outputs"};
    std::vector<double> Milestones = {5.8,6.1,6.3,6.5,6.77, 7.0, 7.28, 7.5, 7.70,8.5};
    double delT = .01;
    //std::string OutputName1 = "PotassiumEnergies.txt";
    //FreeEnergyCalculation(filepaths,  Milestones,delT,OutputName1);

    std::vector<std::string> filepaths2 = {"/gpfs/gibbs/pi/balou/mal258/SodiumBasin","/gpfs/gibbs/pi/balou/mal258/SFFS1Outputs","/gpfs/gibbs/pi/balou/mal258/SFFS2Outputs","/gpfs/gibbs/pi/balou/mal258/SFFS3Combined","/gpfs/gibbs/pi/balou/mal258/SFFS4Combined","/gpfs/gibbs/pi/balou/mal258/SFFS5Outputs","/gpfs/gibbs/pi/balou/mal258/SFFS6Outputs","/gpfs/gibbs/pi/balou/mal258/SFFS7Outputs","/gpfs/gibbs/pi/balou/mal258/SFFS8Outputs"};
    std::string OutputName2 = "SodiumEnergies.txt";
    FreeEnergyCalculation(filepaths2,  Milestones,delT,OutputName2);

    return 0;
}