(:*******************************************************:)
(: Test: K-DurationEQ-3                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of 'eq' for xs:duration.         :)
(:*******************************************************:)
not(xs:duration("P1999Y10M3DT08H34M12.043S") eq
		   xs:duration("P1999Y10M3DT08H34M12.143S"))