(:*******************************************************:)
(: Test: K-AdjDateToTimezoneFunc-7                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Passing a too small xs:dayTimeDuration as timezone to adjust-date-to-timezone(). :)
(:*******************************************************:)
adjust-date-to-timezone(xs:date("2001-02-03"),
							xs:dayTimeDuration("-PT14H1M"))