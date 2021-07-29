(:*******************************************************:)
(: Test: K-DurationEQ-26                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of 'eq' for xs:dayTimeDuration with xs:yearMonthDuration on the right hand. :)
(:*******************************************************:)
xs:dayTimeDuration("PT0S")
	   		      eq
			      xs:yearMonthDuration("P0M")