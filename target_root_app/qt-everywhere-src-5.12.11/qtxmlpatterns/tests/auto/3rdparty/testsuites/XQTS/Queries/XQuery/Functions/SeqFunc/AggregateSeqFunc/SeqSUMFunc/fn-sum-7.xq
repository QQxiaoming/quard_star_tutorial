(: Name: fn-sum-7 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-15 :)
(: Description: Test fn:sum on a sequence of xs:yearMonthDuration and xs:dayTimeDuration arguments :)

sum((xs:yearMonthDuration("P1Y"), xs:dayTimeDuration("P1D")))
