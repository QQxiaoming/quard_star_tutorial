(:*******************************************************:)
(: Test: K-DayTimeDurationEQ-1                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of 'eq' for xs:dayTimeDuration, returning positive. :)
(:*******************************************************:)
xs:dayTimeDuration("P3DT08H34M12.143S") eq
		   xs:dayTimeDuration("P3DT08H34M12.143S")