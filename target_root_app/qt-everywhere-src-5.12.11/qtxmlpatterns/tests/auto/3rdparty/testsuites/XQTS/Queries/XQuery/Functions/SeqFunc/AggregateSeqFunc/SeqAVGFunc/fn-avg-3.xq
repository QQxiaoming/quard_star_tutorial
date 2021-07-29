(: Name: fn-avg-3 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-15 :)
(: Description: Test fn:sum on a sequence of xs:dayTimeDuration arguments :)

avg((xs:dayTimeDuration("P1D"), xs:dayTimeDuration("PT2H"))) instance of xs:dayTimeDuration
