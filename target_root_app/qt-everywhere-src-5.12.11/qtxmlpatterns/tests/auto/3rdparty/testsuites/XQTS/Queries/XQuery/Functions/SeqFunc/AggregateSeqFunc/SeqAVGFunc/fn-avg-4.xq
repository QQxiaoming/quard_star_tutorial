(: Name: fn-avg-4 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-15 :)
(: Description: Test fn:avg on a sequence of xs:dayTimeDuration arguments :)

avg(for $x in 1 to 10 return xs:dayTimeDuration(concat("PT",$x,"H")))
