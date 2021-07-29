(: Name: fn-sum-6 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-15 :)
(: Description: Test fn:sum on a sequence of xs:yearMonthDuration arguments :)

sum(for $x in 1 to 10 return xs:yearMonthDuration(concat("P",$x,"M")))
