(:*******************************************************:)
(: Test: K-DayTimeDurationDivide-1                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of dividing a xs:dayTimeDuration with 4. :)
(:*******************************************************:)
xs:dayTimeDuration("P8DT4H4M4.400S") div 4
	                eq xs:dayTimeDuration("P2DT1H1M1.1S")