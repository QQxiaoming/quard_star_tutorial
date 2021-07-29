(:*******************************************************:)
(: Test: K-DurationEQ-22                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of 'eq' for xs:dayTimeDuration with xs:duration on the right hand. :)
(:*******************************************************:)
xs:dayTimeDuration("P1DT2H")
	   		      eq
			      xs:duration("P1DT2H")