(: Name: fn-sum-4 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-15 :)
(: Description: Test fn:sum on a sequence of xs:yearMonthDuration arguments :)

sum((xs:yearMonthDuration("P1Y"), xs:yearMonthDuration("P1M"))) instance of xs:yearMonthDuration
