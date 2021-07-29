(:*******************************************************:)
(: Test: K-SeqAVGFunc-20                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: A test whose essence is: `string(avg((1, 2, 3, xs:float("NaN")))) eq "NaN"`. :)
(:*******************************************************:)
string(avg((1, 2, 3, xs:float("NaN")))) eq "NaN"