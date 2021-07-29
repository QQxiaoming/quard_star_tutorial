(:*******************************************************:)
(: Test: K-DayTimeDurationGT-5                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of 'ge' for xs:dayTimeDuration.  :)
(:*******************************************************:)
xs:dayTimeDuration("P3DT08H34M12.144S") ge
			   xs:dayTimeDuration("P3DT08H34M12.143S")