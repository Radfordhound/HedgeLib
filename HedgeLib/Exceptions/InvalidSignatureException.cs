using System;

namespace HedgeLib.Exceptions
{
    public class InvalidSignatureException : Exception
    {
        // Constructors
        public InvalidSignatureException(string expectedSig, string readSig) : base(string.Format(
            "Can not read data, the given file's signature is incorrect! (Expected {0} got {1}.)",
            expectedSig, readSig)) { }
    }
}