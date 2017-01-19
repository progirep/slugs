#!/usr/bin/env python2
#
# Specification generator for example from the paper

# Basic actions and the like
REGIONS = ["r1","r2","r3","r4","r5","r7"]
REGION_CONNECTIONS = [("r1","r2"),("r1","r3"),("r3","r2"),("r3","r4"),("r4","r5"),("r5","r6"),("r7","r6"),("r5","r7")]

# Move Actions can have side-conditions
MOVE_ACTIONS = [("r1","r2",[]),("r2","r3",[]),("r1","r3",[]),("r3","r2",[]),("r3","r1",[]),("r4","r3",[]),("r4","r5",["u=0"]),("r5","r4",[]),("r7","r5",["u=0 & (req1 | req2 | req3 | req4 | req5)"]),("r5","r7",[]),("r3","r4",[])]
NOF_ROBOTS = 3
PAIRWISE_MANIPULATION_TASKS = ["r3","r4"]
STARTING_REGION = "r1"

# Communication range requirements:
# - (r1 | r2) & (r4 | r5 | r6 | r7) -> r3
# - r3 & (r5 | r6 | r7) -> r4
# - r4 & r7 -> r5
COMMUNICATION_RANGE_REQUIREMENT = "((r1_ready>0 | r2_ready>0) & (r4_ready>0 | r5_ready>0 | r7_ready>0) -> r3_ready>0) & (r3_ready>0 & (r5_ready>0 | r7_ready>0) -> r4_ready>0) & (r4_ready>0 & r7_ready>0 -> r5_ready>0)"


# Additional specification parts
AUX_PROPS = ["req1","req2","req3","req4","req5"]
AUX_INTS = [("u",0,3)]
AUX_PROPS_AND_INTS_STAY_THE_SAME = [("req1","req1<->req1'"),("req2","req2<->req2'"),("req3","req3<->req3'"),("req4","req4<->req4'"),("req5","req5<->req5'"),("u","u=u'")]
AUX_INIT = ["!req1 & !req2 & !req3 & !req4 & !req5 & u=3"]
AUX_TRANS = [(["req1"],["req1' <-> ! req1"]),(["req2"],["req2' <-> ! req2"]),(["req3"],["req3' <-> ! req3"]),(["req4"],["req4' <-> ! req4"]),(["req5"],["req5' <-> ! req5"]),(["u"],["u'+1 = u"]) ,(["u"],["u' = u+1"])]
AUX_ENV_LIVENESS = []
AUX_SYS_LIVENESS = ["r7_ready = "+str(NOF_ROBOTS)+" & completedTask_r3 & completedTask_r4 | req1 | req2 | req3 | req4 | req5 | u>0","! req1 | r1_ready>0 | u>0","! req2 | r2_ready>0 | u>0","! req3 | r3_ready>0 | u>0","! req4 | r4_ready>0 | u>0","! req5 | r5_ready>0 | u>0"]


# ================= Print all the variables =========================
print "[INPUT]"
for a in REGIONS:
    print a+"_ready:0..."+str(NOF_ROBOTS)
for (a,b,sideconditions) in MOVE_ACTIONS:
    print "move"+a+"_"+b+":0..."+str(NOF_ROBOTS)
for a in PAIRWISE_MANIPULATION_TASKS:
    print "pairwiseTask_"+a+":0..."+str(NOF_ROBOTS)
    print "completedTask_"+a
for a in AUX_PROPS:
    print a
for (a,l,u) in AUX_INTS:
    print a+":"+str(l)+"..."+str(u)
print "opsuccess"
print "environmentDidAction"
print "_endgroup\n"


# ================= Print the initial values of the variables =========================
print "[ENV_INIT]"
for a in REGIONS:
    if a==STARTING_REGION:
        print a+"_ready="+str(NOF_ROBOTS)
    else:
        print a+"_ready=0"
for (a,b,sideconditions) in MOVE_ACTIONS:
    print "move"+a+"_"+b+"=0"
for a in PAIRWISE_MANIPULATION_TASKS:
    print "pairwiseTask_"+a+"=0"
    print "! completedTask_"+a
for a in AUX_INIT:
    print a
print "! opsuccess"
print "! environmentDidAction"

# ================= Environment Transitions ======================
print "[ENV_TRANS]"
print "# ======================"
print "# Actions for the environment:"
print "# ======================"

for (rfrom,rto,sideconditions) in MOVE_ACTIONS:
    print "# Move "+rfrom+"->"+rto+" success"
    for a in REGIONS:
        if a==rto:
            print a+"_ready' = "+a+"_ready+1"
        else:
            print a+"_ready' = "+a+"_ready"
    for (a,b,sideconditions) in MOVE_ACTIONS:
        if (a,b)==(rfrom,rto):
            print "move"+a+"_"+b+"'+1 = move"+a+"_"+b
        else:
            print "move"+a+"_"+b+"' = move"+a+"_"+b
    for a in PAIRWISE_MANIPULATION_TASKS:
        print "pairwiseTask_"+a+"' = pairwiseTask_"+a
        print "completedTask_"+a+"' <-> completedTask_"+a
    for (a,b) in AUX_PROPS_AND_INTS_STAY_THE_SAME:
        print b
    print "opsuccess'"
    print "environmentDidAction'"
    print "_endgroup\n"
    
    print "# Move "+rfrom+"->"+rto+" failure"
    for a in REGIONS:
        if a==rfrom:
            print a+"_ready' = "+a+"_ready+1"
        else:
            print a+"_ready' = "+a+"_ready"
    for (a,b,sideconditions) in MOVE_ACTIONS:
        if (a,b)==(rfrom,rto):
            print "move"+a+"_"+b+"'+1 = move"+a+"_"+b
        else:
            print "move"+a+"_"+b+"' = move"+a+"_"+b
    for a in PAIRWISE_MANIPULATION_TASKS:
        print "pairwiseTask_"+a+"' = pairwiseTask_"+a
        print "completedTask_"+a+"' <-> completedTask_"+a
    for (a,b) in AUX_PROPS_AND_INTS_STAY_THE_SAME:
        print b
    print "! opsuccess'"
    print "environmentDidAction'"
    print "_endgroup\n"
    

for rfrom in PAIRWISE_MANIPULATION_TASKS:
    print "# Pairwise manipulation task "+rfrom+" success"
    for a in REGIONS:
        if a==rfrom:
            print a+"_ready' = "+a+"_ready + 2"
        else:
            print a+"_ready' = "+a+"_ready"
    for (a,b,sideconditions) in MOVE_ACTIONS:
        print "move"+a+"_"+b+"' = move"+a+"_"+b
    for a in PAIRWISE_MANIPULATION_TASKS:
        if a==rfrom:
            print "pairwiseTask_"+a+"' +2 = pairwiseTask_"+a
        else:
            print "pairwiseTask_"+a+"' = pairwiseTask_"+a
        print "completedTask_"+a+"'"
    for (a,b) in AUX_PROPS_AND_INTS_STAY_THE_SAME:
        print b
    print "opsuccess'"
    print "environmentDidAction'"
    print "_endgroup\n"
    
    print "# Pairwise manipulation task "+rfrom+" failure"
    for a in REGIONS:
        if a==rfrom:
            print a+"_ready' = "+a+"_ready + 2"
        else:
            print a+"_ready' = "+a+"_ready"
    for (a,b,sideconditions) in MOVE_ACTIONS:
        print "move"+a+"_"+b+"' = move"+a+"_"+b
    for a in PAIRWISE_MANIPULATION_TASKS:
        if a==rfrom:
            print "pairwiseTask_"+a+"' +2 = pairwiseTask_"+a
        else:
            print "pairwiseTask_"+a+"' = pairwiseTask_"+a
        print "completedTask_"+a+"' <-> completedTask_"+a
    for (a,b) in AUX_PROPS_AND_INTS_STAY_THE_SAME:
        print b
    print "! opsuccess'"
    print "environmentDidAction'"
    print "_endgroup\n"
    
    
print "# Environment can set opsuccess to true if there is nothing to do"
for a in REGIONS:
    print a+"_ready' = "+a+"_ready"
for (a,b,sideconditions) in MOVE_ACTIONS:
    print "move"+a+"_"+b+"' = move"+a+"_"+b
    print "move"+a+"_"+b+" = 0"
for a in PAIRWISE_MANIPULATION_TASKS:
    print "pairwiseTask_"+a+"' = pairwiseTask_"+a
    print "pairwiseTask_"+a+" = 0"
    print "completedTask_"+a+"' <-> completedTask_"+a
for (a,b) in AUX_PROPS_AND_INTS_STAY_THE_SAME:
    print b
print "opsuccess'"
print "! environmentDidAction'"
print "_endgroup\n"

print "# Environment can also do nothing"
for a in REGIONS:
    print a+"_ready' = "+a+"_ready"
for (a,b,sideconditions) in MOVE_ACTIONS:
    print "move"+a+"_"+b+"' = move"+a+"_"+b
for a in PAIRWISE_MANIPULATION_TASKS:
    print "pairwiseTask_"+a+"' = pairwiseTask_"+a
    print "completedTask_"+a+"' <-> completedTask_"+a
for (a,b) in AUX_PROPS_AND_INTS_STAY_THE_SAME:
    print b
print "! opsuccess'"
print "! environmentDidAction'"
print "_endgroup\n"

# Additional actions (for the environment changing its data
for (modifiedVars,cmds) in AUX_TRANS:
    print "# Aux. Action "+repr(cmds)
    for a in REGIONS:
        print a+"_ready' = "+a+"_ready"
    for (a,b,sideconditions) in MOVE_ACTIONS:
        print "move"+a+"_"+b+"' = move"+a+"_"+b
    for a in PAIRWISE_MANIPULATION_TASKS:
        print "pairwiseTask_"+a+"' = pairwiseTask_"+a
        print "completedTask_"+a+"' <-> completedTask_"+a
    for (a,b) in AUX_PROPS_AND_INTS_STAY_THE_SAME:
        if not a in modifiedVars:
            print b
    for a in cmds:
        print a
    print "! opsuccess'"
    print "environmentDidAction'"
    print "_endgroup\n"


# ================= System Transitions ======================
print "[SYS_TRANS]"
print "# ======================"
print "# Actions for the system:"
print "# ======================"

for (rfrom,rto,sideconditions) in MOVE_ACTIONS:
    print "# Move "+rfrom+"->"+rto+" init"+" with side conditions "+str(sideconditions)
    for a in REGIONS:
        if a==rfrom:
            print a+"_ready'+1 = "+a+"_ready"
        else:
            print a+"_ready' = "+a+"_ready"
    for (a,b,sideconditions2) in MOVE_ACTIONS:
        if (a,b)==(rfrom,rto):
            print "move"+a+"_"+b+"' = 1+move"+a+"_"+b
        else:
            print "move"+a+"_"+b+"' = move"+a+"_"+b
    for a in PAIRWISE_MANIPULATION_TASKS:
        print "pairwiseTask_"+a+"' = pairwiseTask_"+a
        print "completedTask_"+a+"' <-> completedTask_"+a
    for (a,b) in AUX_PROPS_AND_INTS_STAY_THE_SAME:
        print b
    for s in sideconditions:
        print s
    print "! opsuccess'"
    print "! environmentDidAction"
    print "! environmentDidAction'"
    print COMMUNICATION_RANGE_REQUIREMENT # Idea: System cannot make a move if communication range requirement not satisfied
    print "_endgroup\n"



for rfrom in PAIRWISE_MANIPULATION_TASKS:
    print "# Pairwise manipulation task "+rfrom+" start"
    for a in REGIONS:
        if a==rfrom:
            print a+"_ready' +2 = "+a+"_ready"
        else:
            print a+"_ready' = "+a+"_ready"
    for (a,b,sideconditions) in MOVE_ACTIONS:
        print "move"+a+"_"+b+"' = move"+a+"_"+b
    for a in PAIRWISE_MANIPULATION_TASKS:
        if a==rfrom:
            print "pairwiseTask_"+a+"' = 2+pairwiseTask_"+a
        else:
            print "pairwiseTask_"+a+"' = pairwiseTask_"+a
        print "completedTask_"+a+"' <-> completedTask_"+a
    for (a,b) in AUX_PROPS_AND_INTS_STAY_THE_SAME:
        print b
    print "! opsuccess'"
    print "! environmentDidAction"
    print "! environmentDidAction'"
    print COMMUNICATION_RANGE_REQUIREMENT # Idea: System cannot make a move if communication range requirement not satisfied
    print "_endgroup\n"


print "# Do nothing"
for a in REGIONS:
    print a+"_ready' = "+a+"_ready"
for (a,b,sideconditions) in MOVE_ACTIONS:
    print "move"+a+"_"+b+"' = move"+a+"_"+b
for a in PAIRWISE_MANIPULATION_TASKS:
    print "pairwiseTask_"+a+"' = pairwiseTask_"+a
    print "completedTask_"+a+"' <-> completedTask_"+a
for (a,b) in AUX_PROPS_AND_INTS_STAY_THE_SAME:
    print b
print "opsuccess' <-> opsuccess"
print "environmentDidAction <-> environmentDidAction'"
print COMMUNICATION_RANGE_REQUIREMENT # Idea: System cannot make a move if communication range requirement not satisfied
print "_endgroup\n"

#-------------------------- ENV_LIVENESS ---------------------------

print "[ENV_LIVENESS]"
print "# The environment has to sometimes perform some actions if it can"
print "# and some of them have to be success actions"
print "opsuccess"
print "! environmentDidAction"

for a in AUX_ENV_LIVENESS:
    print a


#-------------------------- SYS_LIVENESS ---------------------------

print "[SYS_LIVENESS]"
for a in AUX_SYS_LIVENESS:
    print a
