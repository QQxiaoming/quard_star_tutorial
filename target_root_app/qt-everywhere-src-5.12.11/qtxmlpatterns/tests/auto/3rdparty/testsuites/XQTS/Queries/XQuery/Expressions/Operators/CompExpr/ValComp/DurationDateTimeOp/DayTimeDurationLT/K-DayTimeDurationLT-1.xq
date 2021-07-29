(:*******************************************************:)
(: Test: K-DayTimeDurationLT-1                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of 'lt' for xs:dayTimeDuration.  :)
(:*******************************************************:)
xs:dayTimeDuration("P3DT08H34M12.142S") lt
			   xs:dayTimeDuration("P3DT08H34M12.143S")