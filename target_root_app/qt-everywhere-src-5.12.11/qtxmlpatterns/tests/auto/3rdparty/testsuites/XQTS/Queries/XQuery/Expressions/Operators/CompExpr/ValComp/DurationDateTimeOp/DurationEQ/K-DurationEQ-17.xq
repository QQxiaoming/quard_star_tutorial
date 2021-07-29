(:*******************************************************:)
(: Test: K-DurationEQ-17                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of 'eq' for xs:yearMonthDuration with xs:duration on the right hand. Both values are zero. :)
(:*******************************************************:)
xs:yearMonthDuration("P0M") eq
		   xs:duration("PT0S")