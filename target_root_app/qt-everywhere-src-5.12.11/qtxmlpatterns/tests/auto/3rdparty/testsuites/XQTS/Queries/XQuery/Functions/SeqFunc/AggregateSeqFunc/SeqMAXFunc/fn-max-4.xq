(: Name: fn-max-4 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-15 :)
(: Description: Test fn:max on a sequence of xs:dayTimeDuration arguments :)

max((xs:dayTimeDuration("P1D"), xs:dayTimeDuration("PT2H"))) instance of xs:dayTimeDuration
