(:*******************************************************:)
(: Test: K-DurationEQ-28                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of 'eq' for xs:dayTimeDuration with xs:duration on the left hand. :)
(:*******************************************************:)
xs:duration("P1Y12M0D")
	   		      eq
			      xs:yearMonthDuration("P1Y12M")