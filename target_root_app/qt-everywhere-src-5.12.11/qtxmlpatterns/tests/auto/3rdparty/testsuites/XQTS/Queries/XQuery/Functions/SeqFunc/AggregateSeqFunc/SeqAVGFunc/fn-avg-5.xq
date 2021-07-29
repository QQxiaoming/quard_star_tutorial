(: Name: fn-avg-5 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-15 :)
(: Description: Test fn:avg on a sequence of xs:yearMonthDuration arguments :)

avg((xs:yearMonthDuration("P1Y"), xs:yearMonthDuration("P1M"))) instance of xs:yearMonthDuration
