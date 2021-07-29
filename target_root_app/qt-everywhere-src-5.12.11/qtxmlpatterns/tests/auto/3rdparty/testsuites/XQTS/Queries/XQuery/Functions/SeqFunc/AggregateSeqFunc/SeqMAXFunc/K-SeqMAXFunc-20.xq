(:*******************************************************:)
(: Test: K-SeqMAXFunc-20                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: A test whose essence is: `max((xs:double("NaN"), 1, 1, 2, xs:float("NaN"))) instance of xs:double`. :)
(:*******************************************************:)
max((xs:double("NaN"), 1, 1, 2, xs:float("NaN"))) instance of xs:double