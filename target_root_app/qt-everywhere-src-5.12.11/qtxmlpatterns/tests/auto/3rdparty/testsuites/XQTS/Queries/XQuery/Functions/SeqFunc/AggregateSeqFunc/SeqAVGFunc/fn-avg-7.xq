(: Name: fn-avg-7 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-15 :)
(: Description: Test fn:sum on a sequence of xs:yearMonthDuration and xs:dayTimeDuration arguments :)

avg((xs:yearMonthDuration("P1Y"), xs:dayTimeDuration("P1D")))
