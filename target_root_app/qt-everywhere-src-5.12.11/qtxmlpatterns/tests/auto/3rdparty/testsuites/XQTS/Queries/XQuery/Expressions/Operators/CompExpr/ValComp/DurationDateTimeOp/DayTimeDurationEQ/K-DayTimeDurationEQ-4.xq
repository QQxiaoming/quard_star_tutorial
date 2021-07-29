(:*******************************************************:)
(: Test: K-DayTimeDurationEQ-4                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of 'ne' for xs:dayTimeDuration.  :)
(:*******************************************************:)
xs:dayTimeDuration("PT08H34M12.143S") ne
		   xs:dayTimeDuration("P3DT08H34M12.143S")