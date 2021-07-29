(:*******************************************************:)
(: Test: K-DateTimeEQ-7                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Test that zone offset Z is equal to Z, in xs:dateTime. :)
(:*******************************************************:)
xs:dateTime("1999-12-04T16:00:12.345Z") eq
		xs:dateTime("1999-12-04T16:00:12.345Z")