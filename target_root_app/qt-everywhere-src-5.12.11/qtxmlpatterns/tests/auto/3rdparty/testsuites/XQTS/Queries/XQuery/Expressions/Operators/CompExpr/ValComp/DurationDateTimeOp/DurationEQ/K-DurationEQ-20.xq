(:*******************************************************:)
(: Test: K-DurationEQ-20                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of 'eq' for xs:dayTimeDuration with xs:dayTimeDuration on the left hand. Both values are zero. :)
(:*******************************************************:)
xs:dayTimeDuration("PT0S")
	   		      eq
			      xs:yearMonthDuration("P0M")