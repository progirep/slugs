#ifndef __EXTENSION_COOPERATIVE_STRATEGY_HPP
#define __EXTENSION_COOPERATIVE_STRATEGY_HPP

#include "gr1context.hpp"
#include <string>

/**
 * A class that makes the synthesized strategy more "cooperative". 
 */
template<class T> class XCooperativeStrategy : public T {
protected:

    // Inherited stuff used
    using T::mgr;
    using T::strategyDumpingData;
    using T::livenessGuarantees;
    using T::livenessAssumptions;
    using T::varVectorPre;
    using T::varVectorPost;
    using T::varCubePostOutput;
    using T::varCubePostInput;
    using T::safetyEnv;
    using T::safetySys;
    using T::winningPositions;

    // Constructor
    XCooperativeStrategy<T>(std::list<std::string> &filenames) : T(filenames) {}

public:

    /**
     * @brief Modified basic synthesis algorithm - adds to 'strategyDumpingData' transitions
     * that represent "a bias for action"
     */
    void computeWinningPositions() {

        // Save safetyEnv, so that the old environment assumptions can be used during
        // explicit-state strategy extraction
        BF originalSafetyEnv = safetyEnv;
        BF originalSafetySys = safetySys;

        // Save initial strategy
        std::vector<std::vector<std::pair<unsigned int,BF> > > dumpingData;
        T::computeWinningPositions();
        dumpingData.push_back(strategyDumpingData);

        // Now alternate between assumpting and guarantee strengthening

        // Save safetyEnv and safetySys to detect convergence of the following loop
        BF oldSafetyEnv;
        BF oldSafetySys;

        unsigned int nof_iteration = 0;
        do {
            std::cerr << "Cooperative Strategy Extraction Iteration\n";
            oldSafetyEnv = safetyEnv;
            oldSafetySys = safetySys;
            nof_iteration++;

            // Make Guarantees more strict"
            BF oldWinningPositions = winningPositions;
            BF winningPositionsPost = winningPositions.SwapVariables(varVectorPre,varVectorPost);
            BF nonDeadEndsEnvironment = safetyEnv.ExistAbstract(varCubePostInput).SwapVariables(varVectorPre,varVectorPost);
            std::ostringstream fn1;
            fn1 << "/tmp/nonDeadEndEnv" << nof_iteration << ".dot";
            BF_newDumpDot(*this,nonDeadEndsEnvironment,"Post",fn1.str());

            // New:
            BF safetyEnvRestriction = (safetySys & winningPositionsPost).ExistAbstract(varCubePostOutput) & !((safetySys & nonDeadEndsEnvironment & winningPositionsPost).ExistAbstract(varCubePostOutput));
            std::ostringstream fn2;
            fn2 << "/tmp/safetyEnvRestriction" << nof_iteration << ".dot";
            BF_newDumpDot(*this,safetyEnvRestriction & safetyEnv,"Pre Post",fn2.str());

            safetyEnv &= !safetyEnvRestriction;
            safetySys &= nonDeadEndsEnvironment;

            // strategyDumpingData.clear();
            // T::computeWinningPositions();
            // dumpingData.push_back(strategyDumpingData);

            // Add the requirement that the environment must not force the system
            // to actively work towards the violation of the assumptions
            // Old:
            //BF niceDestinationStates = safetyEnv.ExistAbstract(varCubePostInput) & winningPositions;
            //BF_newDumpDot(*this,niceDestinationStates,NULL,"/tmp/niceDestState.dot");
            //BF niceDestinationTransitions = niceDestinationStates.SwapVariables(varVectorPre,varVectorPost) & safetySys;
            //BF_newDumpDot(*this,niceDestinationTransitions,NULL,"/tmp/niceDestTrans.dot");

            //BF niceDestinationTransitions =

            // safetyEnv &= niceDestinationTransitions.ExistAbstract(varCubePostOutput);
            BF_newDumpDot(*this,safetyEnv,NULL,"/tmp/safetyEnvNeu.dot");

            // BF nonDeadEndsSystem = (safetySys & oldWinningPositions.SwapVariables(varVectorPre,varVectorPost)).ExistAbstract(varCubePostOutput);
            // safetyEnv &= nonDeadEndsEnvironment;
            // strategyDumpingData.clear();
            // T::computeWinningPositions();

            strategyDumpingData.clear();
            T::computeWinningPositions();
            dumpingData.push_back(strategyDumpingData);

            // throw 7;

        } while ((oldSafetySys!=safetySys) || (safetyEnv!=safetyEnv));

        // Prepare strategy
        strategyDumpingData.clear();
        unsigned int dumpingNumber = 0;
        for (auto it = dumpingData.rbegin();it!=dumpingData.rend();it++) {
            std::vector<std::pair<unsigned int,BF> > &current = *it;
            dumpingNumber++;

            // Compute positional strategy on the current level
            for (unsigned int i=0;i<livenessGuarantees.size();i++) {
                BF casesCovered = mgr.constantFalse();
                BF strategy = mgr.constantFalse();
                for (auto it = current.begin();it!=current.end();it++) {
                    if (it->first == i) {
                        BF newCases = it->second.ExistAbstract(varCubePostOutput) & !casesCovered;
                        strategy |= newCases & it->second;
                        casesCovered |= newCases;
                    }
                }
                strategyDumpingData.push_back(std::pair<unsigned int,BF>(i,strategy));
                std::ostringstream os;
                os << "/tmp/coopStrat-" << dumpingNumber << "-" << i << ".dot";
                BF_newDumpDot(*this,strategy,"PreInput PreOutput PostInput PostOutput",os.str());
            }
        }

        // Restore old safetyEnv, so that the explicit-state strategy extraction method
        // produces transitions for all possible inputs.
        safetyEnv = originalSafetyEnv;
        safetySys = originalSafetySys;

    }

    static GR1Context* makeInstance(std::list<std::string> &filenames) {
        return new XCooperativeStrategy<T>(filenames);
    }
};

#endif

