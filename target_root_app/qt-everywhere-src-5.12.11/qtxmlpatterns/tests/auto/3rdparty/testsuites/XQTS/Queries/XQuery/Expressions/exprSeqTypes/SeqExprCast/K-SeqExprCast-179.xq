(:*******************************************************:)
(: Test: K-SeqExprCast-179                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that when casting xs:dayTimeDuration to xs:string, that preceding zeros are handled properly. :)
(:*******************************************************:)
xs:string(xs:dayTimeDuration("P0010DT0010H0010M0010S"))
		eq "P10DT10H10M10S"