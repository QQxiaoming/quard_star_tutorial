(:*******************************************************:)
(: Test: K-DurationEQ-23                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of 'ne' for xs:dayTimeDuration with xs:duration on the left hand. :)
(:*******************************************************:)
xs:duration("P1M1DT2H") ne
		   xs:dayTimeDuration("P1DT2H")