(:*******************************************************:)
(: Test: K-SeqExprCast-154                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that when casting xs:duration to xs:string, preceding zeros are handled properly. :)
(:*******************************************************:)
xs:string(xs:duration("P0010Y0010M0010DT0010H0010M0010S"))
		eq "P10Y10M10DT10H10M10S"