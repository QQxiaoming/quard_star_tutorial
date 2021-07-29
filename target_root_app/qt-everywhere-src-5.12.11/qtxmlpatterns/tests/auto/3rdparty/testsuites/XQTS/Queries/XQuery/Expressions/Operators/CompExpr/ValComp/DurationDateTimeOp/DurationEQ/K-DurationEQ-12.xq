(:*******************************************************:)
(: Test: K-DurationEQ-12                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The 'le' operator is not available between xs:duration and xs:duration. :)
(:*******************************************************:)
xs:duration("P1999Y10M3DT08H34M12.143S") le
			   xs:duration("P1999Y10M3DT08H34M12.143S")