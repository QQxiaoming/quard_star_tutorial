(: Name: fn-min-7 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-15 :)
(: Description: Test fn:min on a sequence of xs:yearMonthDuration arguments :)

min(for $x in 1 to 10 return xs:yearMonthDuration(concat("P",$x,"M")))
