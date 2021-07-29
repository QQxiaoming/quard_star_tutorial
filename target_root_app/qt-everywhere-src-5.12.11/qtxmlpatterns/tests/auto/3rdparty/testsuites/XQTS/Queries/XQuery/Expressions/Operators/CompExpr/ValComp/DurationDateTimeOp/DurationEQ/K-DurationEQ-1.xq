(:*******************************************************:)
(: Test: K-DurationEQ-1                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of 'eq' for xs:duration, returning positive. :)
(:*******************************************************:)
xs:duration("P1999Y10M3DT08H34M12.143S") eq
		   xs:duration("P1999Y10M3DT08H34M12.143S")