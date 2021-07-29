(:*******************************************************:)
(: Test: K-SeqMINFunc-17                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: A test whose essence is: `string(min((xs:float("NaN"), xs:untypedAtomic("3"), xs:float(2)))) eq "NaN"`. :)
(:*******************************************************:)
string(min((xs:float("NaN"), xs:untypedAtomic("3"), xs:float(2)))) eq "NaN"