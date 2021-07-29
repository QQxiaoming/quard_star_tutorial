(: Name: fn-max-5 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-15 :)
(: Description: Test fn:max on a sequence of xs:dayTimeDuration arguments :)

max(for $x in 1 to 10 return xs:dayTimeDuration(concat("PT",$x,"H")))
