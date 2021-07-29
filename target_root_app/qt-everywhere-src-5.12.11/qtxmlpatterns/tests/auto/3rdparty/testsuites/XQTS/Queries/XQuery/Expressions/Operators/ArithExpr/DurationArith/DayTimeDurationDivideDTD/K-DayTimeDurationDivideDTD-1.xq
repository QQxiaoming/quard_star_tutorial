(:*******************************************************:)
(: Test: K-DayTimeDurationDivideDTD-1                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple test of dividing a xs:dayTimeDuration with xs:dayTimeDuration. :)
(:*******************************************************:)
(xs:dayTimeDuration("PT8M") div xs:dayTimeDuration("PT2M")) eq 4