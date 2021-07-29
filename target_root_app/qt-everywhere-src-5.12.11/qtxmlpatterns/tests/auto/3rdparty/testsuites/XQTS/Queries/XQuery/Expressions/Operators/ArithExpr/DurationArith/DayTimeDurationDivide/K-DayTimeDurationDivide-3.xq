(:*******************************************************:)
(: Test: K-DayTimeDurationDivide-3                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of dividing a xs:dayTimeDuration with xs:double('INF'). :)
(:*******************************************************:)
xs:dayTimeDuration("P3D") div xs:double("INF") eq
			   xs:dayTimeDuration("PT0S")