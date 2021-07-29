(: Name: fn-avg-6 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-15 :)
(: Description: Test fn:sum on a sequence of xs:yearMonthDuration arguments :)

avg(for $x in 1 to 9 return xs:yearMonthDuration(concat("P",$x,"M")))
