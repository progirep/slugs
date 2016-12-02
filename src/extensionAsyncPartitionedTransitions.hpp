#ifndef __ASYNC_PARTITIONED_TRANSITIONS__
#define __ASYNC_PARTITIONED_TRANSITIONS__


#include "gr1context.hpp"
#include <string>

/**
 * Experimental ASynchronous Partitioned Transitions Plugin
 */
template<class T> class XAsynchronousPartitionedTransitions : public T {
protected:

    // Inherited stuff used
    using T::mgr;
    using T::strategyDumpingData;
    using T::livenessGuarantees;
    using T::livenessAssumptions;
    using T::initEnv;
    using T::initSys;
    using T::varVectorPre;
    using T::varVectorPost;
    using T::varCubePostOutput;
    using T::varCubePost;
    using T::varCubePostInput;
    using T::safetyEnv;
    using T::safetySys;
    using T::winningPositions;
    using T::lineNumberCurrentlyRead;
    using T::addVariable;
    using T::parseBooleanFormula;
    using T::variableNames;

    // Actions for the two players
    std::vector<BF> envPlayerActions;
    std::vector<BF> sysPlayerActions;

    // Constructor
    XAsynchronousPartitionedTransitions<T>(std::list<std::string> &filenames) : T(filenames) {}

public:

    void init(std::list<std::string> &filenames) {
        if (filenames.size()==0) {
            throw "Error: Cannot load SLUGS input file - there has been no input file name given!";
        }

        std::string inFileName = filenames.front();
        filenames.pop_front();

        // Open input file or produce error message if that does not work
        std::ifstream inFile(inFileName.c_str());
        if (inFile.fail()) {
            std::ostringstream errorMessage;
            errorMessage << "Cannot open input file '" << inFileName << "'";
            throw errorMessage.str();
        }

        // Prepare safety and initialization constraints
        initEnv = mgr.constantTrue();
        initSys = mgr.constantTrue();
        safetyEnv = mgr.constantTrue();
        safetySys = mgr.constantTrue();

        // EnvPlayer/SysPlayer-Action
        BF currentEnvPlayerAction = mgr.constantTrue();
        BF currentSysPlayerAction = mgr.constantTrue();
        bool currentEnvPlayerActionPartsRead = false;
        bool currentSysPlayerActionPartsRead = false;

        // The readmode variable stores in which chapter of the input file we are
        int readMode = -1;
        std::string currentLine;
        lineNumberCurrentlyRead = 0;
        while (std::getline(inFile,currentLine)) {
            lineNumberCurrentlyRead++;
            boost::trim(currentLine);
            if ((currentLine.length()>0) && (currentLine[0]!='#')) {
                if (currentLine[0]=='[') {
                    if (currentLine=="[INPUT]") {
                        readMode = 0;
                    } else if (currentLine=="[OUTPUT]") {
                        readMode = 1;
                    } else if (currentLine=="[ENV_INIT]") {
                        readMode = 2;
                    } else if (currentLine=="[SYS_INIT]") {
                        readMode = 3;
                    } else if (currentLine=="[ENV_TRANS]") {
                        readMode = 4;
                    } else if (currentLine=="[SYS_TRANS]") {
                        readMode = 5;
                    } else if (currentLine=="[ENV_LIVENESS]") {
                        readMode = 6;
                    } else if (currentLine=="[SYS_LIVENESS]") {
                        readMode = 7;
                    } else {
                        std::cerr << "Sorry. Didn't recognize category " << currentLine << "\n";
                        throw "Aborted.";
                    }
                } else {
                    if (readMode==0) {
                        // Add input variable, but ignore the special proposition "_endgroup"
                        if (currentLine!="_endgroup") {
                            addVariable(PreInput,currentLine);
                            addVariable(PostInput,currentLine+"'");
                        }
                    } else if (readMode==1) {
                        addVariable(PreOutput,currentLine);
                        addVariable(PostOutput,currentLine+"'");
                    } else if (readMode==2) {
                        std::set<VariableType> allowedTypes;
                        allowedTypes.insert(PreInput);
                        initEnv &= parseBooleanFormula(currentLine,allowedTypes);
                    } else if (readMode==3) {
                        std::set<VariableType> allowedTypes;
                        allowedTypes.insert(PreInput);
                        allowedTypes.insert(PreOutput);
                        initSys &= parseBooleanFormula(currentLine,allowedTypes);
                    } else if (readMode==4) {
                        // SafetyEnv / Actions of the environment player
                        std::set<VariableType> allowedTypes;
                        allowedTypes.insert(PreInput);
                        allowedTypes.insert(PreOutput);
                        allowedTypes.insert(PostInput);
                        if (currentLine=="_endgroup") {
                            envPlayerActions.push_back(currentEnvPlayerAction);
                            currentEnvPlayerAction = mgr.constantTrue();
                            currentEnvPlayerActionPartsRead = false;
                        } else {
                            currentEnvPlayerActionPartsRead = true;
                            currentEnvPlayerAction &= parseBooleanFormula(currentLine,allowedTypes);
                        }
                    } else if (readMode==5) {
                        // SafetyEnv / Actions of the system player
                        std::set<VariableType> allowedTypes;
                        allowedTypes.insert(PreInput);
                        allowedTypes.insert(PreOutput);
                        allowedTypes.insert(PostInput);
                        allowedTypes.insert(PostOutput);
                        if (currentLine=="_endgroup") {
                            sysPlayerActions.push_back(currentSysPlayerAction);
                            currentSysPlayerAction = mgr.constantTrue();
                            currentSysPlayerActionPartsRead = false;
                        } else {
                            currentSysPlayerActionPartsRead = true;
                            currentSysPlayerAction &= parseBooleanFormula(currentLine,allowedTypes);
                        }
                    } else if (readMode==6) {
                        std::set<VariableType> allowedTypes;
                        allowedTypes.insert(PreInput);
                        allowedTypes.insert(PreOutput);
                        //allowedTypes.insert(PostOutput); -- not for this plugin
                        //allowedTypes.insert(PostInput);
                        livenessAssumptions.push_back(parseBooleanFormula(currentLine,allowedTypes));
                    } else if (readMode==7) {
                        std::set<VariableType> allowedTypes;
                        allowedTypes.insert(PreInput);
                        allowedTypes.insert(PreOutput);
                        //allowedTypes.insert(PostInput); -- not for this plugin
                        //allowedTypes.insert(PostOutput);
                        livenessGuarantees.push_back(parseBooleanFormula(currentLine,allowedTypes));
                    } else {
                        std::cerr << "Error with line " << lineNumberCurrentlyRead << "!";
                        throw "Found a line in the specification file that has no proper categorial context.";
                    }
                }
            }
        }

        // Check if variable names have been used twice
        std::set<std::string> variableNameSet(variableNames.begin(),variableNames.end());
        if (variableNameSet.size()!=variableNames.size()) throw SlugsException(false,"Error in input file: some variable name has been used twice!\nPlease keep in mind that for every variable used, a second one with the same name but with a \"'\" appended to it is automacically created.");

        // Do all environment/system player actions have been terminated by "_endgroup"?
        if (currentEnvPlayerActionPartsRead) throw "There is an environment player action whose specification has not been terminated by '_endgroup'.";
        if (currentSysPlayerActionPartsRead) throw "There is a system player action whose specification has not been terminated by '_endgroup'.";

        // Make sure that there is at least one liveness assumption and one liveness guarantee
        // The synthesis algorithm might be unsound otherwise
        if (livenessAssumptions.size()==0) livenessAssumptions.push_back(mgr.constantTrue());
        if (livenessGuarantees.size()==0) livenessGuarantees.push_back(mgr.constantTrue());
    }


    void computeWinningPositions() {

        // The greatest fixed point - called "Z" in the GR(1) synthesis paper
        BFFixedPoint nu2(mgr.constantTrue());

        // Iterate until we have found a fixed point
        for (;!nu2.isFixedPointReached();) {

            // Iterate over all of the liveness guarantees. Put the results into the variable 'nextContraintsForGoals' for every
            // goal. Then, after we have iterated over the goals, we can update nu2.
            BF nextContraintsForGoals = mgr.constantTrue();
            for (unsigned int j=0;j<livenessGuarantees.size();j++) {

                BF liveGoals = livenessGuarantees[j] & nu2.getValue();
                BF_newDumpDot(*this,liveGoals,NULL,"/tmp/livegoals.dot");

                // Compute the middle least-fixed point (called 'Y' in the GR(1) paper)
                BFFixedPoint mu1(mgr.constantFalse());
                for (;!mu1.isFixedPointReached();) {

                    // Update the set of transitions that lead closer to the goal.
                    liveGoals |= mu1.getValue();

                    // Iterate over the liveness assumptions. Store the positions that are found to be winning for *any*
                    // of them into the variable 'goodForAnyLivenessAssumption'.
                    BF goodForAnyLivenessAssumption = mu1.getValue();
                    for (unsigned int i=0;i<livenessAssumptions.size();i++) {

                        // Inner-most greatest fixed point. The corresponding variable in the paper would be 'X'.
                        BFFixedPoint nu0(mgr.constantTrue());
                        for (;!nu0.isFixedPointReached();) {

                            BF targetPositions = (nu0.getValue() & !livenessAssumptions[i]) | liveGoals;
                            BF_newDumpDot(*this,targetPositions,NULL,"/tmp/target.dot");

                            // Iterate over system player actions - check if we find an applicable one
                            BF newTargetPositions = mgr.constantFalse();
                            for (BF &a : sysPlayerActions) {
                                newTargetPositions |= (targetPositions.SwapVariables(varVectorPre,varVectorPost) & a).ExistAbstract(varCubePost);
                            }

                            BF_newDumpDot(*this,newTargetPositions,NULL,"/tmp/nt.dot");

                            // Now check from which states every environment player actions are ok.
                            BF finalTargetPositions = mgr.constantTrue();
                            for (BF &a : envPlayerActions) {
                                finalTargetPositions &= (newTargetPositions.SwapVariables(varVectorPre,varVectorPost) | !a).UnivAbstract(varCubePost);
                                BF_newDumpDot(*this,finalTargetPositions,NULL,"/tmp/inner.dot");
                                std::cerr << ".";
                            }

                            nu0.update(finalTargetPositions);

                        }

                        goodForAnyLivenessAssumption |= nu0.getValue();
                        // BF_newDumpDot(*this,nu0.getValue(),NULL,"/tmp/nu0finalvalue.dot");

                    }

                    // Update the moddle fixed point
                    mu1.update(goodForAnyLivenessAssumption);
                }

                // Update the set of positions that are winning for any goal for the outermost fixed point
                nextContraintsForGoals &= mu1.getValue();
            }

            // Update the outer-most fixed point
            nu2.update(nextContraintsForGoals);
        }

        // We found the set of winning positions
        winningPositions = nu2.getValue();
        BF_newDumpDot(*this,winningPositions,NULL,"/tmp/winningPositions.dot");
    }



    static GR1Context* makeInstance(std::list<std::string> &filenames) {
        return new XAsynchronousPartitionedTransitions<T>(filenames);
    }
};

































#endif
