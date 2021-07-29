(: Name: fn-max-8 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-15 :)
(: Description: Test fn:max on a sequence of xs:yearMonthDuration and xs:dayTimeDuration arguments :)

max((xs:yearMonthDuration("P1Y"), xs:dayTimeDuration("P1D")))
