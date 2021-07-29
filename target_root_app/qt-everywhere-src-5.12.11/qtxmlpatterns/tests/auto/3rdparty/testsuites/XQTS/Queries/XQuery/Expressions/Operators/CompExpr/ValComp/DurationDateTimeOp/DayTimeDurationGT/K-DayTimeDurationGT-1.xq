(:*******************************************************:)
(: Test: K-DayTimeDurationGT-1                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of 'gt' for xs:dayTimeDuration.  :)
(:*******************************************************:)
xs:dayTimeDuration("P3DT08H34M12.144S") gt
			   xs:dayTimeDuration("P3DT08H34M12.143S")