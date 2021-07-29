(:*******************************************************:)
(: Test: K-AdjDateToTimezoneFunc-10                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Example from F&O.                            :)
(:*******************************************************:)

			adjust-date-to-timezone(xs:date("2002-03-07-07:00"),
						xs:dayTimeDuration("-PT5H0M"))
			eq xs:date("2002-03-07-05:00")
		