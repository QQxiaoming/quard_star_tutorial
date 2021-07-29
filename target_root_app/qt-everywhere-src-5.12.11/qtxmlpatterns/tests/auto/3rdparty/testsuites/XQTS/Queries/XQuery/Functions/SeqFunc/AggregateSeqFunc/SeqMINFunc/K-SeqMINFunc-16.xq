(:*******************************************************:)
(: Test: K-SeqMINFunc-16                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: A test whose essence is: `string(min((1, xs:untypedAtomic("NaN"), xs:float(2)))) eq "NaN"`. :)
(:*******************************************************:)
string(min((1, xs:untypedAtomic("NaN"), xs:float(2)))) eq "NaN"