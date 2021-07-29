(: Name: fn-sum-1 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-15 :)
(: Description: Test fn:sum on a sequence of xs:dayTimeDuration arguments :)

sum((xs:dayTimeDuration("P1D"), xs:dayTimeDuration("PT1H"))) instance of xs:dayTimeDuration
