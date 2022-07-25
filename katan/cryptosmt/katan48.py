'''
Created on May 13, 2022

@author: jesenteh
'''

from parser import stpcommands
from ciphers.cipher import AbstractCipher

class katan48(AbstractCipher):
    """
    Represents the differential behaviour of Katan48 and can be used
    to find differential characteristics for the given parameters.
    It uses an alternative representation of Katan48 in ARX form.
    """

    name = "katan48"
    
    def getFormatString(self):
        """
        Returns the print format.
        """
        return ['Xa', 'Xb', 'A', 'F', 'w']

    def createSTP(self, stp_filename, parameters):
        """
        Creates an STP file to find a characteristic for KATAN48 with
        the given parameters.
        Must use wordsize = 64
        """

        wordsize = parameters["wordsize"]
        rounds = parameters["rounds"]
        weight = parameters["sweight"]
        offset = parameters["offset"]

        with open(stp_filename, 'w') as stp_file:
            header = ("% Input File for STP\n% KATAN48 w={}"
                      "rounds={} with IR offset={}\n\n\n".format(wordsize,rounds,offset))
            stp_file.write(header)

            # Setup variables
            # xa = input (64) after first sub-round (only 48 bits used)
            # xb = input (64) after first subround (only 48 bits used)
            # f = outputs of AND operation (Only 6 bits required, use 6 bits to store)
            # a = active or inactive AND operation (Only 6 bits required, use 6 bits to store)
            xa = ["Xa{}".format(i) for i in range(rounds + 1)] #Additional one for output difference
            xb = ["Xb{}".format(i) for i in range(rounds)] #Intermediate x values, no need output difference
            f = ["F{}".format(i) for i in range(rounds)]
            a = ["A{}".format(i) for i in range(rounds)]

            # w = weight
            w = ["w{}".format(i) for i in range(rounds)]

            stpcommands.setupVariables(stp_file, xa, wordsize)
            stpcommands.setupVariables(stp_file, xb, wordsize)
            stpcommands.setupVariables(stp_file, f, wordsize)
            stpcommands.setupVariables(stp_file, a, wordsize)
            stpcommands.setupVariables(stp_file, w, wordsize)

            stpcommands.setupWeightComputation(stp_file, weight, w, wordsize)
            
            #Modify start_round to start from different positions
            for i in range(rounds):
                self.setupKatanRound(stp_file, xa[i], xb[i], f[i], a[i], xa[i+1], 
                                     w[i], wordsize, i, offset)

            # No all zero characteristic
            stpcommands.assertNonZero(stp_file, xa, wordsize)

            # Iterative characteristics only
            # Input difference = Output difference
            if parameters["iterative"]:
                stpcommands.assertVariableValue(stp_file, xa[0], xa[rounds])

            for key, value in parameters["fixedVariables"].items():
                stpcommands.assertVariableValue(stp_file, key, value)

            for char in parameters["blockedCharacteristics"]:
                stpcommands.blockCharacteristic(stp_file, char, wordsize)

            stpcommands.setupQuery(stp_file)

        return

    def setupKatanRound(self, stp_file, xa_in, xb, f, a, xa_out, w, wordsize, r, offset):
        """
        Model for differential behaviour of one round KATAN48
        """
        command = ""
        IR = [1,1,1,1,1,1,1,0,0,0,
              1,1,0,1,0,1,0,1,0,1,
              1,1,1,0,1,1,0,0,1,1,
              0,0,1,0,1,0,0,1,0,0,
              0,1,0,0,0,1,1,0,0,0,
              1,1,1,1,0,0,0,0,1,0,
              0,0,0,1,0,1,0,0,0,0,
              0,1,1,1,1,1,0,0,1,1,
              1,1,1,1,0,1,0,1,0,0,
              0,1,0,1,0,1,0,0,1,1,
              0,0,0,0,1,1,0,0,1,1,
              1,0,1,1,1,1,1,0,1,1,
              1,0,1,0,0,1,0,1,0,1,
              1,0,1,0,0,1,1,1,0,0,
              1,1,0,1,1,0,0,0,1,0,
              1,1,1,0,1,1,0,1,1,1,
              1,0,0,1,0,1,1,0,1,1,
              0,1,0,1,1,1,0,0,1,0,
              0,1,0,0,1,1,0,1,0,0,
              0,1,1,1,0,0,0,1,0,0,
              1,1,1,1,0,1,0,0,0,0,
              1,1,1,0,1,0,1,1,0,0,
              0,0,0,1,0,1,1,0,0,1,
              0,0,0,0,0,0,1,1,0,1,
              1,1,0,0,0,0,0,0,0,1,
              0,0,1,0]
              
        #***************************************
        #First iteration of nonlinear functions
        #***************************************
        
        # Check if AND is active
        # a[0] = x[6] | x[15]
        command += "ASSERT({0}[0:0] = {1}[6:6]|{2}[15:15]);\n".format(a, xa_in, xa_in)
        # a[1] = x[13]| x[21]
        command += "ASSERT({0}[1:1] = {1}[13:13]|{2}[21:21]);\n".format(a, xa_in, xa_in)
        #Locations for L1 = 7 and 15. In full 48-bit register, 7+29 = 36, 15+29 = 44
        # a[2] = x[36] | x[44]
        command += "ASSERT({0}[2:2] = {1}[36:36]|{2}[44:44]);\n".format(a, xa_in, xa_in)
        
        #Calculate weights and output of AND operations
        #If a=1, w = 1, otherwise, w = 0
        #If a = 1, f = 0/1, otherwise, f = 0
        for i in range (3):
            #command += "ASSERT({0}[{2}:{2}] = {1}[{2}:{2}]);\n".format(w,a,i)
            command += "ASSERT(BVLE({0}[{2}:{2}],{1}[{2}:{2}]));\n".format(f,a,i)
        
        #w[1]=a[2]
        command += "ASSERT({0}[1:1] = {1}[2:2]);\n".format(w,a) #AND in the L1 register
        #As long as either 1 AND operation in L2 register is active, prob is 1
        #w[0]=a[0]|a[1]
        command += "ASSERT({0}[0:0] = {1}[0:0] | {1}[1:1]);\n".format(w,a)

        #Shift and store into xb
        #Permutation layer (shift left L2 by 1 except for position 28)
        for i in range(0,28):
            command += "ASSERT({0}[{1}:{1}] = {2}[{3}:{3}]);\n".format(xb,i+1, xa_in,i)
        #Permutation layer (shift left L1 by 1 except for position 47 (L1_12))   
        for i in range(29,47):
            command += "ASSERT({0}[{1}:{1}] = {2}[{3}:{3}]);\n".format(xb,i+1, xa_in,i)
       
        #Perform XOR operation for to get bits for position L2_0 and 29 (L1_0)
        #x_out[0] = x[47]^x[41]^a[2]^(x[35]&IR[r]) 
        command += "ASSERT({0}[0:0] = BVXOR({1}[47:47],BVXOR({1}[41:41],BVXOR({2}[2:2],({1}[35:35]&0b{3})))));\n".format(xb,xa_in, f, IR[r+offset])
        #x_out[29] = x[28]^a[1]^x[19]^a[0] 
        command += "ASSERT({0}[29:29] = BVXOR({1}[28:28],BVXOR({2}[1:1],BVXOR({1}[19:19],{2}[0:0]))));\n".format(xb,xa_in, f)
        
        #***************************************
        #Second iteration of nonlinear functions
        #***************************************
        #Bits 3-5 are used for a,f,w
        
        # Check if AND is active
        # a[3] = x[6] | x[15]
        command += "ASSERT({0}[3:3] = {1}[6:6]|{2}[15:15]);\n".format(a, xb, xb)
        # a[4] = x[13]| x[21]
        command += "ASSERT({0}[4:4] = {1}[13:13]|{2}[21:21]);\n".format(a, xb, xb)
        #Locations for L1 = 7 and 15. In full 48-bit register, 7+29 = 36, 15+29 = 44
        # a[5] = x[36] | x[44]
        command += "ASSERT({0}[5:5] = {1}[36:36]|{2}[44:44]);\n".format(a, xb, xb)
        
        #Calculate weights and output of AND operations
        #If a=1, w = 1, otherwise, w = 0
        #If a = 1, f = 0/1, otherwise, f = 0
        for i in range (3,6):
            #command += "ASSERT({0}[{2}:{2}] = {1}[{2}:{2}]);\n".format(w,a,i)
            command += "ASSERT(BVLE({0}[{2}:{2}],{1}[{2}:{2}]));\n".format(f,a,i)
        
        #w[3]=a[5]
        command += "ASSERT({0}[3:3] = {1}[5:5]);\n".format(w,a) #AND in the L1 register
        #As long as either 1 AND operation in L2 register is active, prob is 1
        #w[2]=a[3]|a[4]
        command += "ASSERT({0}[2:2] = {1}[3:3] | {1}[4:4]);\n".format(w,a)

        #Shift and store into xa_out
        #Permutation layer (shift left L2 by 1 except for position 28)
        for i in range(0,28):
            command += "ASSERT({0}[{1}:{1}] = {2}[{3}:{3}]);\n".format(xa_out,i+1, xb,i)
        #Permutation layer (shift left L1 by 1 except for position 47 (L1_12))   
        for i in range(29,47):
            command += "ASSERT({0}[{1}:{1}] = {2}[{3}:{3}]);\n".format(xa_out,i+1, xb,i)
       
        #Perform XOR operation for to get bits for position L2_0 and 29 (L1_0)
        #x_out[0] = x[47]^x[41]^a[5]^(x[35]&IR[r]) 
        command += "ASSERT({0}[0:0] = BVXOR({1}[47:47],BVXOR({1}[41:41],BVXOR({2}[5:5],({1}[35:35]&0b{3})))));\n".format(xa_out,xb, f, IR[r+offset])
        #x_out[29] = x[28]^a[4]^x[19]^a[3] 
        command += "ASSERT({0}[29:29] = BVXOR({1}[28:28],BVXOR({2}[4:4],BVXOR({1}[19:19],{2}[3:3]))));\n".format(xa_out,xb, f)
        
        #Use 6 bits to store weight (for each of the ANDs in the 2 iteraions). The rest must be 0.
        command += "ASSERT(0b000000000000000000000000000000000000000000000000000000000000 = {0}[63:4]);\n".format(w) 
        command += "ASSERT(0b0000000000000000000000000000000000000000000000000000000000 = {0}[63:6]);\n".format(f)
        command += "ASSERT(0b0000000000000000000000000000000000000000000000000000000000 = {0}[63:6]);\n".format(a)
        
        #TODO: Fix the 16 MSBs to zero for all X
        command += "ASSERT(0b0000000000000000 = {0}[63:48]);\n".format(xa_in) 
        command += "ASSERT(0b0000000000000000 = {0}[63:48]);\n".format(xa_out) 
        command += "ASSERT(0b0000000000000000 = {0}[63:48]);\n".format(xb) 

        stp_file.write(command)
        return
        
