(:*******************************************************:)
(: Test: K-SeqExprCast-180                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that a negative xs:dayTimeDuration is properly serialized when cast to xs:string. :)
(:*******************************************************:)
xs:string(xs:dayTimeDuration("-P0010DT0010H0010M0010S"))
		eq "-P10DT10H10M10S"