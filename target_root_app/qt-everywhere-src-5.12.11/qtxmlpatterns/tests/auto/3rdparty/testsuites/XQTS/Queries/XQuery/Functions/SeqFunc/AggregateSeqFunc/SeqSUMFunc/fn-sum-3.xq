(: Name: fn-sum-3 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-15 :)
(: Description: Test fn:sum on a sequence of xs:dayTimeDuration arguments :)

sum(for $x in 1 to 10 return xs:dayTimeDuration(concat("PT",$x,"H")))
