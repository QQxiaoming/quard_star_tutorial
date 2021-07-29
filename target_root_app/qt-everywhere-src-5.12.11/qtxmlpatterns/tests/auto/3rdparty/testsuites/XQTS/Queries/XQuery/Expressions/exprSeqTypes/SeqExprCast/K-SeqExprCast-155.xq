(:*******************************************************:)
(: Test: K-SeqExprCast-155                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that a negative xs:duration is properly serialized when cast to xs:string. :)
(:*******************************************************:)
xs:string(xs:duration("-P0010DT0010H0010M0010S"))
		eq "-P10DT10H10M10S"