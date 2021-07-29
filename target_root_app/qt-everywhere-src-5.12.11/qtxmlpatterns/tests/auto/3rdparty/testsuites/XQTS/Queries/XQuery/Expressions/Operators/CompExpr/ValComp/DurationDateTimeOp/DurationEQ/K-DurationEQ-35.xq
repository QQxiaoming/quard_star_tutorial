(:*******************************************************:)
(: Test: K-DurationEQ-35                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The 'lt' operator is not available between xs:duration and xs:dayTimeDuration. :)
(:*******************************************************:)
xs:duration("P1999Y10M3DT08H34M12.143S") lt
			   xs:dayTimeDuration("P3DT08H34M12.143S")