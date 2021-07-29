(:*******************************************************:)
(: Test: K-AdjDateToTimezoneFunc-13                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Example from F&O.                            :)
(:*******************************************************:)

			adjust-date-to-timezone(xs:date("2002-03-07"), ())
			eq xs:date("2002-03-07")
		