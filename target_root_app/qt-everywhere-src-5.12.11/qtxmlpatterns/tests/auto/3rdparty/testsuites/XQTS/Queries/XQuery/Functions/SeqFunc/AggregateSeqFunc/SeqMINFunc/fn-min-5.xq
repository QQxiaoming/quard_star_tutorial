(: Name: fn-min-5 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-15 :)
(: Description: Test fn:min on a sequence of xs:dayTimeDuration arguments :)

min(for $x in 1 to 10 return xs:dayTimeDuration(concat("PT",$x,"H")))
