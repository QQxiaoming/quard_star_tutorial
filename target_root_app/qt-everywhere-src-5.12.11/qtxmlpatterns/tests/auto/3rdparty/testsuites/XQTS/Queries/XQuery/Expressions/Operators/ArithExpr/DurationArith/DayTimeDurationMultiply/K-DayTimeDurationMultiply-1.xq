(:*******************************************************:)
(: Test: K-DayTimeDurationMultiply-1                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of multiplying a xs:dayTimeDuration with 3. :)
(:*******************************************************:)
xs:dayTimeDuration("P3DT4H3M3.100S") * 3
	                eq xs:dayTimeDuration("P9DT12H9M9.3S")