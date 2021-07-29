(:*******************************************************:)
(: Test: K-DurationEQ-27                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of 'eq' for xs:yearMonthDuration with xs:duration on the right hand. :)
(:*******************************************************:)
xs:yearMonthDuration("P1Y12M") eq
		   xs:duration("P1Y12M0D")